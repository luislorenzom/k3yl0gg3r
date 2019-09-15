CC=gcc
CFLAGS=-c -Wall -g -o0
KEYLOG_TARGETS=main.o keylogger.o find_keyboard_event.o

keylogger: 	$(KEYLOG_TARGETS)
		$(CC) $^ -o $@

#debug:	$(KEYLOG_TARGETS)
#	$(CC) -g -o0 $^ -o $@

clean: 
	rm -f keylogger *.o
