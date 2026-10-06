#!/usr/bin/env python3
"""Behavior tests using an isolated fixture; runs on NetBSD with Python 3."""
import os
from pathlib import Path
import socket
import subprocess
import tempfile
import unittest
BIN = str(Path(__file__).resolve().parent / 'myls')
class ListingTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(); self.root=Path(self.tmp.name)
        (self.root/'a').write_text('a'); (self.root/'b').write_text('bb')
        (self.root/'.hidden').touch(); (self.root/'sub').mkdir()
        (self.root/'sub'/'child').touch(); (self.root/'link').symlink_to('sub')
        (self.root/'broken').symlink_to('missing'); (self.root/'run').touch()
        (self.root/'run').chmod(0o4755); os.mkfifo(self.root/'pipe')
        (self.root/'bad\nname').touch()
        self.sock=None
        try:
            self.sock=socket.socket(socket.AF_UNIX); self.sock.bind(str(self.root/'sock'))
        except PermissionError:
            if self.sock: self.sock.close()
            self.sock=None
    def tearDown(self):
        if self.sock: self.sock.close()
        self.tmp.cleanup()
    def runls(self,*args,ok=True):
        p=subprocess.run([BIN,*args],cwd=self.root,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        self.assertEqual(p.returncode==0,ok,p.stderr.decode())
        return p.stdout.decode()
    def test_sort(self):
        self.assertEqual(self.runls('b','a'),'a\nb\n')
        self.assertEqual(self.runls('-r','b','a'),'b\na\n')
        self.assertEqual(self.runls('-S','a','b'),'b\na\n')
        self.assertEqual(self.runls('-f','b','a'),'b\na\n')
    def test_hidden(self):
        self.assertNotIn('.hidden',self.runls('-q') if os.geteuid()!=0 else '')
        self.assertIn('.hidden',self.runls('-Aq'))
        lines=self.runls('-aq').splitlines(); self.assertIn('.',lines); self.assertIn('..',lines)
        self.assertNotIn('.',self.runls('-Aq').splitlines())
    def test_types(self):
        out=self.runls('-Fq')
        for name in ['sub/','run*','link@','broken@','pipe|','bad?name']:
            self.assertIn(name,out)
        if self.sock: self.assertIn('sock=',out)
        self.assertIn(' -> missing',self.runls('-l','broken'))
        self.assertTrue(self.runls('-l','run').startswith('-rwsr-xr-x'))
    def test_precedence(self):
        self.assertEqual(self.runls('-Rd','sub'),'sub\n')
        self.assertIn('child',self.runls('-dR','sub'))
        self.assertIn('bad?name',self.runls('-wq'))
        self.assertIn('bad\nname',self.runls('-qw'))
        self.assertEqual(self.runls('-ln','a').split()[2],str(os.getuid()))
        self.assertIn('2B',self.runls('-kh','-l','b'))
        self.assertNotIn('2B',self.runls('-hk','-l','b'))
    def test_recursive_and_links(self):
        out=self.runls('-Ra'); self.assertIn('./sub:',out)
        self.assertNotIn('./link:',out); self.assertIn('child',out)
        self.assertEqual(self.runls('link'),'child\n')
        self.assertEqual(self.runls('-d','link'),'link\n')
    def test_errors_and_totals(self):
        self.assertIn('a\n',self.runls('missing','a',ok=False))
        self.runls('-z',ok=False)
        self.assertTrue(self.runls('-l').startswith('total '))
        self.assertFalse(self.runls('-s').startswith('total '))
    def test_times_and_sparse(self):
        os.utime(self.root/'a',(100,100)); os.utime(self.root/'b',(200,200))
        self.assertEqual(self.runls('-t','a','b'),'b\na\n')
        self.assertEqual(self.runls('-tr','a','b'),'a\nb\n')
        with open(self.root/'sparse','wb') as f: f.truncate(1024*1024)
        self.assertEqual(int(self.runls('-s','sparse').split()[0]),(self.root/'sparse').stat().st_blocks)
        for flag in 'AacdFfhiklnqRrSstuw': self.runls('-'+flag,'a')
if __name__=='__main__': unittest.main()
