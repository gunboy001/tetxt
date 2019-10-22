#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
//STD functions
#include <stdlib.h>
// STD input output
#include <stdio.h>
// iscntrl
#include <ctype.h>
#include <sys/types.h>

#include "term.h"
#include "draw.h"
#include <string.h>
#include <fcntl.h>

//Strip bits 5 and 6 from the key 
//This emulates what the control key does
//To the key you press
#define CTRL_KEY(k) ((k) & 0x1f)

void processKey (void);
void openFile (char *filename);
void appendRow (char *s, size_t len);
void rowInsertChar (s_row *rw, int at, int c);
void insertChar (int c);
char *rowsToString(int *buflen);
void editorSave (void);

int main (int argc, char *argv[])
{
	enableRawMode();
	initEditor();
	initDraw();

	if (argc >= 2) openFile(argv[1]);
	else die("invalid file");

	updateState(argv[1]);

	while (1) {
		refreshScreen();
		processKey();	
	}
	return 0;
}

void processKey (void)
{
	int c = readKey();

	switch (c) {
		case CTRL_KEY('q'):
			clearScreen();
			exit(0);
			break;
		case CTRL_KEY('s'):
			editorSave();
			exit(0);
			break;
		case ARR_UP:
		case ARR_DOWN:
		case ARR_DX:
		case ARR_SX:
			moveCursor(c);
			break;
		default:
			insertChar(c);
			break;
	}
}

void openFile (char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp) die("fopen");
	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen;
	linelen = getline(&line, &linecap, fp);
	while ((linelen = getline(&line, &linecap, fp)) != -1) {
		while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
			linelen--;
		appendRow(line, linelen);
	}
	free(line);
	fclose(fp);
}

void appendRow (char *s, size_t len)
{
	rows.row = realloc(rows.row, sizeof(s_row) * (rows.rownum + 1));
	int at = rows.rownum;
	rows.row[at].size = len;
	rows.row[at].chars = malloc(len + 1);
	memcpy(rows.row[at].chars, s, len);
	rows.row[at].chars[len] = '\0';

	rows.row[at].r_size = 0;
	rows.row[at].render = NULL;
	
	updateRender(&rows.row[at]);

	rows.rownum++;
}

void rowInsertChar (s_row *rw, int at, int c)
{
	if (at < 0 || at > rw->size) at = rw->size;
	rw->chars = realloc(rw->chars, rw->size + 2);
	memmove(&rw->chars[at +1], &rw->chars[at], rw->size - at + 1);
	rw->size++;
	rw->chars[at] = c;
	updateRender(rw);
}

void insertChar (int c)
{
	if (cursor.y == rows.rownum) appendRow("", 0);
	rowInsertChar(&rows.row[cursor.y], cursor.x, c);
	cursor.x++;
}

char *rowsToString(int *buflen) {
	int totlen = 0;
	int j;
	
	for (j = 0; j < rows.rownum; j++)
		totlen += rows.row[j].size + 1;
		
	*buflen = totlen;
	char *buf = malloc(totlen);
	char *p = buf;
	
	for (j = 0; j < rows.rownum; j++) {
		memcpy(p, rows.row[j].chars, rows.row[j].size);
		p += rows.row[j].size;
		*p = '\n';
		p++;
	}
	return buf;
}

void editorSave() {
	if (global_state.filename == NULL) return;
	
	int len;
	char *buf = rowsToString(&len);
	int fd = open(global_state.filename, O_RDWR | O_CREAT, 0644);
	
	ftruncate(fd, len);
	write(fd, buf, len);
	close(fd);
	free(buf);
}