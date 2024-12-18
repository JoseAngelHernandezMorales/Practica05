SHELL=/bin/bash
# https://gcc.gnu.org/gcc-10/porting_to.html
# > Legacy C code where all tentative definitions should be placed
# > into a common block can be compiled with '-fcommon'.
CFLAGS = -g -fcommon
CC     = gcc

ALL_D    = daemon.c log.c util.c lockfile.c socket.c confdata.c daemon-child-func.c

daemon: $(ALL_D)
	$(CC) -o daemon $(CFLAGS) $(ALL_D)

.PHONY: clean
clean:
	-rm -v -f *.o daemon \#*\# *~ logfile

debug-strace:	daemon
	@echo ""
	@echo "* To debug this daemon run the following in the shell with strace"
	@echo "* Then, open the client and send requests"
	@echo "* At the end, terminate the daemon with ^C"
	@echo "* The log will be saved as logfile"
	@echo ""
	@echo "strace ./daemon -F -d 9 2>&1 | tee logfile"
