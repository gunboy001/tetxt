//tcgetattr
#include <unistd.h>
//Terminal attributes
#include <termios.h>
//STD functions
#include <stdlib.h>
// STD input output
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "term.h"
#include <string.h>

void enableRawMode (void) 
{

	//Get current terminal attributes into raw
	if (tcgetattr(STDIN_FILENO, &global_state.orig_termios) == -1) die("tcgetaddr");

	//Once we anable raw mode disable it on exit
	//this in part of stdlib
	atexit(disableRawMode);

	struct termios raw = global_state.orig_termios;

	//Disable:
	//	software control flags (Ctrl-S Ctrl-Q)
	//Enable:
	//	translate carriage return to newline
	//in input flags
	raw.c_iflag &= ~(IXON | ICRNL);

	//Disable output processing 
	//The terminal automatically translates
	//newline \n into carriage return followed
	//by a new line \r\n so we disable that
	raw.c_oflag &= ~(OPOST);

	//Disable bits that determine:
	//	terminal echoing
	//	Canonical mode (give input line by line)
	//	Interrupt signals (Ctrl-C Ctrl-Z) (ISIG IEXTEN)
	//in the local flags
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	//Misc flags
	raw.c_cflag |= (CS8);
	raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP);

	//Set maximum timeouts for read() return in 100 ms
	raw.c_cc[VTIME] = 1;

	//Set minimum read bytes to make read() return
	raw.c_cc[VMIN] = 0; 

	//Set new attributes
	//TCSAFLUSH specifies that the cahnges should 
	//Be applied after all other ios are set
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetaddr");
}

void disableRawMode (void)
{
	//Restore the original settings
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &global_state.orig_termios) == -1) die("tcsetaddr");
}

void die (const char *s)
{
	//Give error message and exit
	perror(s);
	exit(1);
}

int readKey (void)
{
	int nread;
	char c;

	// while no bytes are input check for error
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
		if (nread == -1 && errno != EAGAIN) die("read");

	if (c == ESCAPE_SEQ) {
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1) != 1) return ESCAPE_SEQ;
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return ESCAPE_SEQ;
			if (seq[0] == '[') {
				switch (seq[1]) {
					case 'A': return ARR_UP;
					case 'B': return ARR_DOWN;
					case 'C': return ARR_DX;
					case 'D': return ARR_SX;
				}
			}
		return ESCAPE_SEQ;
		} else {
		return c;
	}
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void initEditor (void)
{
	if (getWindowSize(&global_state.scr_rows, &global_state.scr_cols) == -1) 
		die("Screen too smol");
	global_state.scr_rows -= 1;
	global_state.filename = NULL;
}

void updateState (char *arg)
{
	free(global_state.filename);
	global_state.filename = malloc(strlen(arg) + 1);
	memcpy(global_state.filename, arg, strlen(arg) + 1);
}