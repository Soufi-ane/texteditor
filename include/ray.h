#ifndef RAY_H
#define RAY_H

#define SCREEN_HEIGHT 1080
#define TOP_PADDING 34
#define SCREEN_WIDTH 960
// #define LINE1_Y 133
#define LINE1_Y 120
#define LINE_X_POSITION 50
#define LINE_HEIGHT 47
#define MODE_POSITION {810, 38}
#define DAY_POSITION {67, 38}
#define MONTH_POSITION {112, 38}
#define YEAR_POSITION {155, 38}
#define GLSL_VERSION 330
#define FILES_X_POSITION 100

void resetBlur(
	Image* bg,
	Texture2D* bgTexture,
	char* path
);

void applyBlur(
	Image* bg,
	Texture2D* bgTexture,
	int value
);

#endif
