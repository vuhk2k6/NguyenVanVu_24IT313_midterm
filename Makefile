CC = cc
CFLAGS = -O2 -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_NETBSD_SOURCE
OBJ = main.o options.o util.o entries.o display.o traversal.o
all: myls
myls: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)
$(OBJ): ls.h util.h options.h entries.h display.h traversal.h
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f myls $(OBJ)
.PHONY: all clean
