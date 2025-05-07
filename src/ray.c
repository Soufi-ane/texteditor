#include <raylib.h>


void resetBlur(
		Image* bg,
		Texture2D* bgTexture,
		char* path
){
	*bg = LoadImage(path);
	*bgTexture = LoadTextureFromImage(*bg);
}

void applyBlur(
		Image* bg,
		Texture2D* bgTexture,
		int value
){
	ImageBlurGaussian(bg,value);
	*bgTexture = LoadTextureFromImage(*bg);
}
