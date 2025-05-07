#ifndef NOTE_H
#define NOTE_H

#define LINES_COUNT 20
#define LINE_LENGTH 100
#define TITLE_LENGTH 20

typedef struct noteline_ {
	char* chars;
	int length;
	int size;
} NoteLine;

NoteLine* createNoteLine(int n);

void addChar(
	NoteLine* line,
	char c
);

void addLine(
	NoteLine* line,
	char* text
);

void popChar(NoteLine* line);

#endif
