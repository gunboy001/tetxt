#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "draw.h"
#include "term.h"

void abAppend (struct append_buffer *ab, const char *s, int i_len)
{
	//extend the block of memory that contained the old buffer
	char *new = realloc(ab->buf, ab->len + i_len);

	if (new == NULL) return;
	memcpy(&new[ab->len], s, i_len);
	ab->buf = new;
	ab->len += i_len;
}

void abFree (struct append_buffer *ab)
{
	free(ab->buf);
}

void drawRows (struct append_buffer *ab)
{	
	int y, filerow;
	for (y = 0; y < global_state.scr_rows; y++) {
		filerow = y + rows.rowoff;
		if (filerow >= rows.rownum) abAppend(ab, "~", 1);
		else {
			int len = rows.row[filerow].r_size - rows.coloff;
			if (len > global_state.scr_cols) len = global_state.scr_cols;
			abAppend(ab, &rows.row[filerow].render[rows.coloff], len);
		}
		
		// clear the cursor line
		abAppend(ab, CL_LINE, CL_LINE_S);
		
		abAppend(ab, "\r\n", 2);
	}
}

void refreshScreen (void)
{
	edScroll();
	
	//init the buffer
	struct append_buffer ab = ABUF_INIT;

	//hide the cursor when repainting
	abAppend(&ab, DISABLE_C, DISABLE_C_S);

	// reposition the cursor up
	abAppend(&ab, REPOS_C, REPOS_C_S);

	drawRows(&ab);

	char msg[50];
	snprintf(msg, 50, "%s | %d x %d", global_state.filename, global_state.scr_cols, global_state.scr_rows);
	drawStatusBar(&ab, msg);

	char buf[32];
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", 
		(cursor.y - rows.rowoff) + 1, (cursor.r_x - rows.coloff) + 1);
	abAppend(&ab, buf, strlen(buf));
	
	//redraw cursor
	abAppend(&ab, REABLE_C, REABLE_C_S);
	
	write(STDOUT_FILENO, ab.buf, ab.len);
	abFree(&ab);
}

void clearScreen (void)
{
	write(STDOUT_FILENO, CL_ALL, CL_ALL_S);
	write(STDOUT_FILENO, REPOS_C, REPOS_C_S);
}

void initDraw (void)
{
	cursor.x = 0;
	cursor.y = 0;
	cursor.r_x = 0;
	rows.rownum = 0;
	rows.rowoff = 0;
	rows.coloff = 0;
	rows.row = NULL;
}

void moveCursor (int a)
{
	s_row *rw = (cursor.y >= rows.rownum) ? NULL : &rows.row[cursor.y];
	
	switch (a) {
		case ARR_DOWN:
			if (cursor.y < rows.rownum) cursor.y++;
			break;
		case ARR_UP:
			if (cursor.y) cursor.y--;
			break;
		case ARR_DX:
			if (rw && cursor.x < rw->size) cursor.x++;
			else if (cursor.y < rows.rownum) {
				cursor.y++;
				cursor.x = 0;
			}
			break;
		case ARR_SX:
			if (cursor.x) cursor.x--;
			else if (cursor.y) {
				cursor.y--;
				cursor.x = rows.row[cursor.y].size;
			}
			break;
	}

	rw = (cursor.y >= rows.rownum) ? NULL : &rows.row[cursor.y];
	if (!rw) cursor.x = 0;
	else if (cursor.x > rw->size) cursor.x = rw->size;
}

void edScroll (void)
{
	cursor.r_x = 0;
	if (cursor.y < global_state.scr_cols)
		cursor.r_x = cursorRealToRender(&rows.row[cursor.y], cursor.x);
	
	if (cursor.y < rows.rowoff)
		rows.rowoff = cursor.y;
	if (cursor.y >= rows.rowoff + global_state.scr_rows)
		rows.rowoff = cursor.y - global_state.scr_rows + 1;

	if (cursor.r_x < rows.coloff)
		rows.coloff = cursor.r_x;
	if (cursor.r_x >= rows.coloff + global_state.scr_cols)
		rows.coloff = cursor.r_x - global_state.scr_cols + 1;
}

void updateRender (s_row *rw)
{
	int tabs = 0, i;
	for (i = 0; i < rw->size; i++) {
		if (rw->chars[i] == '\t') tabs++;
	}

	free(rw->render);

	int mem = rw->size + tabs * (TABSIZE - 1) + 1;
	//we already count 1 byte for the tab char so TABSIZE-1
	rw->render = malloc(mem);
	
	if (rw->render == NULL) die ("malloc in updateRender");

	int off = 0;
	for (i = 0; i < rw->size; i++) {
		if (rw->chars[i] == '\t') {
			for (int j = 0; j < TABSIZE; j++)
				rw->render[off++] = ' ';
		} else {
			rw->render[off++] = rw->chars[i];
		}
	}
	rw->render[off] = '\0';
	rw->r_size = off;
}

int cursorRealToRender (s_row *rw, int c_x)
{
	int r_x = 0;
	for (int i = 0; i < c_x; i++) {
		if (rw->chars[i] == '\t') r_x += (TABSIZE - 1) - (r_x % TABSIZE);
		r_x++;
	}
	return r_x;
}

void drawStatusBar (struct append_buffer *ab, char *s)
{
	abAppend(ab, "\x1b[7m", 4);
	
	int msglen = strlen(s);
	if (msglen <= global_state.scr_cols) abAppend(ab, s, msglen);
	else msglen = 0;
	
	for (int i = 0; i < global_state.scr_cols - msglen; i++)
		abAppend(ab, " ", 1);
	abAppend(ab, "\x1b[m", 3);
}