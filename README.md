# Midterm Project — Implement ls(1)

- Student: Nguyễn Văn Vũ
- Student ID: 24IT313
- Repository name: `NguyenVanVu_24IT313_midterm`
- Repository URL: **TODO: replace with your actual GitHub repository URL before submitting.**
- Target: NetBSD, C11; also builds on Linux.
- Specification: the supplied three-page NetBSD 10.1 ls(1) subset, dated October 27, 2024. No external ls command is invoked.

## Build and run

```sh
make
./myls
./myls -la
./myls -lh /etc
./myls -n /etc/passwd
./myls -R .
./myls -d /etc
./myls -iFs .
./myls -tr .
BLOCKSIZE=1024 ./myls -s .
TZ=UTC ./myls -l .
make clean
```

Use `./myls` explicitly: the system `ls` is not replaced. Only a C compiler and make are needed. Makefile uses syntax compatible with BSD make and GNU make.

## Implemented options

| Option | Behavior |
|---|---|
| -A | Include hidden names except . and ..; enabled for effective UID 0 |
| -a | Include all names, including . and .. |
| -c | Select status-change time for printing and time sorting |
| -d | List directory operands themselves; disable recursion and operand link dereference |
| -F | Append /, *, @, %, = or \| according to type |
| -f | Preserve collection order; disable sorting |
| -h | Human-readable allocated sizes and long-format sizes, base 1024 |
| -i | Print inode number |
| -k | Use 1024-byte units for block counts |
| -l | Long format with owner/group names, falling back to numeric IDs |
| -n | Long format with numeric owner/group IDs |
| -q | Replace nonprintable or invalid characters with ? |
| -R | Recursively list actual subdirectories; disable -d |
| -r | Reverse the selected sorting order |
| -S | Sort by decreasing logical size; name breaks ties |
| -s | Print allocated block count, rounded up to selected units |
| -t | Sort by decreasing selected timestamp, then name |
| -u | Select access time for printing and time sorting |
| -w | Output raw filename bytes |

Last option wins for q/w, l/n, c/u, R/d, h/k. -k does not change the logical byte size field in -l. -f disables sorting regardless of other sorting flags. Default output is one entry per line. Non-directory operands precede directory operands; the groups are sorted separately.

Long format displays type/permissions (including setuid, setgid and sticky), link count, owner, group, byte size or device major/minor, month/day/hour/minute, name and symbolic-link target. `total` precedes directory contents with -l/-n, or with -s when stdout is a terminal. Block accounting uses st_blocks, not st_size.

Default -q applies only when stdout is a terminal. Printable multibyte characters are preserved using the current LC_CTYPE locale. TZ is handled by localtime_r. BLOCKSIZE accepts a positive byte count with optional K/M/G and optional B suffix (e.g. 1024, 1K, 2MB); invalid values fall back to 512. -h/-k override BLOCKSIZE. Exit status is 0 for success, 1 for errors. Errors go to stderr; other valid operands continue to be processed.

## Modules

| Files | Responsibility |
|---|---|
| main.c | Operand grouping and application lifecycle |
| ls.h | Shared types and module integration |
| options.c / options.h | Argument parsing and environment defaults |
| entries.c / entries.h | Metadata collection, ownership and sorting |
| display.c / display.h | Permissions, names, sizes, dates, device numbers and links |
| traversal.c / traversal.h | Directory enumeration and recursive traversal |
| util.c / util.h | Allocation, path construction and diagnostics |
| test.py | Isolated behavioral regression tests |

Directory entries are inspected with lstat; recursive traversal never follows directory symlinks. Ancestor device/inode checks detect directory cycles. Paths and link targets are dynamically allocated rather than limited to a fixed PATH_MAX buffer. Allocation errors terminate cleanly. A disappearing entry reports an error. Like ordinary pathname-based tools, this program does not provide an atomic snapshot of a concurrently changing filesystem.

## Decisions where the supplied manual is incomplete

- The last -S or -t chooses the sorting key. Ties use bytewise lexicographic strcmp ordering; nanoseconds participate in time comparisons.
- Command-line symlinks to directories are followed only for ordinary listings without -d, -l/-n or -F; dangling links remain printable. Directory-entry links are never followed.
- -f does not implicitly enable -a, since the supplied manual only specifies unsorted output.
- Human sizes use binary units with suffixes B/K/M/G/T/P/E; one decimal below 10 units and no decimals above that. Precise humanize_number formatting is not specified in the supplied text.
- Dates always use the specified month/day/hour/minute format, including old files; no year substitution is added.
- NetBSD whiteout/archive types are supported conditionally when the system exposes S_ISWHT/S_ISARCH1/S_ISARCH2. Ordinary file, directory, link, device, socket and FIFO types are supported directly. Special filesystem types require a suitable NetBSD fixture for validation.
- The R/d paragraph saying these options determine file time is interpreted as a typo: they control directory traversal.

## Validation and evidence

Local validation was performed on Linux: warning-free build with -Wall -Wextra -Wpedantic and 7 passing behavioral test groups. These exercise all 19 flags, sorting, precedence, hidden entries, recursion, links, special permissions, sparse-file allocation, totals, and errors. UNIX socket creation is automatically omitted when forbidden by the execution environment; it remains tested where permitted.

The project compiled successfully on NetBSD without warnings.
Manual tests passed for all 19 options, recursive traversal,
symbolic links, option precedence, BLOCKSIZE, nonprintable
filenames, and error handling.
Before submitting, run these on your NetBSD VM:

```sh
make clean
make
./myls -la .
./myls -n /dev/null
./myls -F /tmp
./myls -Ra .
./myls /path/that/does/not/exist README.md
echo $?
```

With Python 3 installed, optionally run:

```sh
python3 test.py
```

Also capture terminal screenshots of: successful make, default listing, -la, -n, -F, -R, -t/-S/-r, -h/-k/-s, and a missing-path error with nonzero status. Run permission-denied tests as a normal user, not root. Use system ls only to compare behaviors specified in the supplied manual; system ls may include other defaults.

## GitHub submission

Create an empty GitHub repository named `NguyenVanVu_24IT313_midterm`. Replace YOUR_GITHUB_USERNAME below with your actual username and update the repository URL at the top of this report.

```sh
git init
git add .
git commit -m "Implement modular ls subset and project report"
git branch -M main
git remote add origin https://github.com/YOUR_GITHUB_USERNAME/NguyenVanVu_24IT313_midterm.git
git push -u origin main
```

If transferring a ZIP to NetBSD, enable hidden files when copying so .gitignore is retained. Check `git status --short` before commits. Executable myls, object files and core dumps are ignored. Submit the report with the actual repository URL through e-learning.
