/*
 * find_keyboard_event.c
 *
 *  Created on: 23 abr. 2019
 *      Author: luis
 */

#include "find_keyboard_event.h"

#define INPUT_DIR "/dev/input/"

/**
 * Checks in received file is a character device
 */
static int is_char_device(const struct dirent *file) {

    struct stat filestat;
    char filename[512];
    int err;

    snprintf(filename, sizeof(filename), "%s%s", INPUT_DIR, file->d_name);

    err = stat(filename, &filestat);
    if (err) {
        return 0;
    }

    return S_ISCHR(filestat.st_mode);
}

char *get_keyboard_event_file(void) {

	char *keyboard_file = NULL;
	int num_files, i;
	struct dirent **event_files;
	char filename[512];

	num_files = scandir(INPUT_DIR, &event_files, &is_char_device, &alphasort);

	if (num_files < 0) {

		return NULL;

	} else {

		for (i = 0; i < num_files; i++) {

			int32_t event_bitmap = 0;
			int fd;
			int32_t kbd_bitmap = KEY_A | KEY_B | KEY_C | KEY_Z;

			snprintf(filename, sizeof(filename), "%s%s", INPUT_DIR, event_files[i]->d_name);
			fd = open(filename, O_RDONLY);

			if (fd == -1) {
				perror("open");
				continue;
			}

            ioctl(fd, EVIOCGBIT(0, sizeof(event_bitmap)), &event_bitmap);
            if ((EV_KEY & event_bitmap) == EV_KEY) {
            // The device acts like a keyboard

                ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(event_bitmap)), &event_bitmap);
                if ((kbd_bitmap & event_bitmap) == kbd_bitmap) {

                    // The device supports A, B, C, Z keys, so it probably is a keyboard
                    keyboard_file = strdup(filename);
                    close(fd);
                    break;
                }
            }

            close(fd);

        }
    }

    // Cleanup scandir
    for (i = 0; i < num_files; ++i) {
        free(event_files[i]);
    }

    free(event_files);

    return keyboard_file;
}
