/*
 * keylogger.h
 *
 *  Created on: 22 abr. 2019
 *      Author: luis
 */

#ifndef KEYLOGGER_H_
#define KEYLOGGER_H_

#define NUM_EVENTS 128

/**
 *
 * Captures keystrokes by reading from the keyboard resource and writing to
 * the writeout file. Continues reading until SIGINT is recieved. A newline is
 * appended to the end of the file.
 *
 */
void keylogger(int keyboard, int writeout);


#endif /* KEYLOGGER_H_ */
