#ifndef NOTE_H
#define NOTE_H

#define LINES_COUNT 20
#define LINE_LENGTH 100
#define TITLE_LENGTH 20

typedef struct line_ {
	char* chars;
	int length;
	int size;
} Line;

Line* createLine(int n);
void deleteLine(Line** line);

void emptyLine(
	Line* line,
	int n
);

void addChar(
	Line* line,
	char c
);

void addLine(
	Line* line,
	char* text
);

void popChar(Line* line);

#endif
