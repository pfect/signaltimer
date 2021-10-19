CC=gcc

EXTRA_WARNINGS=-Wall -W 

BINS=timer

timer:	timer.c log.c ini.c 
	 $(CC) $+ $(CFLAGS)  -o $@ -I.

clean:
	rm -rf $(BINS)

