#ifndef TERM_H
#define TERM_H

#include <termios.h>

#define ESCAPE_SEQ '\x1b'

enum keys {
	ARR_UP = 1000,
	ARR_DOWN,
	ARR_SX,
	ARR_DX
};
//save the global state of the editor into a struct
struct state {
	int scr_rows;
	int scr_cols;
	struct termios orig_termios;
	char *filename;
} global_state;

void disableRawMode (void);
void enableRawMode (void);
void die (const char *s);
int readKey (void);
int getWindowSize(int *rows, int *cols);
void initEditor (void);
void updateState (char *arg);

#endif