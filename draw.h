#ifndef DRAW_H
#define DRAW_H

#define ABUF_INIT {NULL, 0}

#define CL_ALL "\x1b[2J"
#define CL_ALL_S 4

#define CL_LINE "\x1b[K"
#define CL_LINE_S 3

#define REPOS_C "\x1b[H"
#define REPOS_C_S 3

#define DISABLE_C "\x1b[?25l"
#define DISABLE_C_S 6

#define REABLE_C "\x1b[?25h"
#define REABLE_C_S 6

#define TABSIZE 4

struct append_buffer {
	char *buf;
	int len;
};

struct cursor {
	int x;
	int y;
	int r_x;
} cursor;

//singlw row sturct
typedef struct s_row {
	int size;
	char *chars;
	int r_size;
	char *render;
} s_row;

struct {
	int rowoff;
	int coloff;
	int rownum;
	s_row *row;
} rows;

void abAppend (struct append_buffer *ab, const char *s, int i_len);
void abFree (struct append_buffer *ab);

void drawRows (struct append_buffer *ab);
void refreshScreen (void);
void clearScreen (void);
void initDraw (void);
void moveCursor (int a);
void edScroll (void);
void updateRender (s_row *rw);
int cursorRealToRender (s_row *rw, int c_x);
void drawStatusBar (struct append_buffer *ab, char *s);

#endif