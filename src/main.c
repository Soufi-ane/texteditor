#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "note.h"
#include "ray.h"
#include "files.h"


enum Mode {
	NORMAL,
	INSERT,
	NEW,
	OPEN,
	SEARCH,
	NONE,
};
enum Mode currentMode = NONE;

void getLocalDate(char* year,char* month,char* day) {
	time_t t = time(NULL);
	struct tm* locaTime = localtime(&t);
	strftime(year,5,"%Y",locaTime);
	strftime(month,3,"%m",locaTime);
	strftime(day,3,"%d",locaTime);
}

int currentFileIndex = 0;
char dir[100];
NoteLine* fileNames[MAX_FILES_NUM];
char year[5],month[3],day[3];
int isInsertingTitle = 1;
int linesNum = 1;
int filesCount = 0;
int longPressDelay = 30;

int main(){
	Color fgColor = GetColor(0xD9D9D9FF);
	const char* HOME_DIR = getenv("HOME");
	if(HOME_DIR == NULL) printf("ERRR!\n"); 
	sprintf(dir,"%s/.local/notes",HOME_DIR);
	getDirContent(fileNames,&filesCount,dir);
	getLocalDate(year,month,day);
	NoteLine* line = createNoteLine(LINE_LENGTH);
	NoteLine* currentLine = line;
	NoteLine* note[LINES_COUNT - 1];
	NoteLine* title = createNoteLine(TITLE_LENGTH);
	note[0] = line;


	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"Notes");
	SetExitKey(KEY_NULL);
	int fileSize = 0;
	Font fontSDF = {0};
	loadFontSDF(
		// "assets/fonts/JetBrainsMonoNF.ttf",
		"/usr/local/share/fonts/JetBrainsMonoNF.ttf",
		&fileSize,
		128,
		&fontSDF
	);

	// Image bg = LoadImage("assets/imgs/notebook.png");
	// Texture2D bgTexture = LoadTextureFromImage(bg);
	Image menuImg = LoadImage("/usr/local/share/images/menuImg.png");
	if(menuImg.data == NULL) menuImg = LoadImage("assets/imgs/menuImg.png");
	// applyBlur(&bg,&bgTexture,2);
	// ImageBlurGaussian(&bg,2);
	// ImageColorBrightness(&bg,-70);
	Texture2D menuTexture = LoadTextureFromImage(menuImg);
	UnloadImage(menuImg);
	SetTargetFPS(60);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	while(!WindowShouldClose()){
		if(currentMode != INSERT) {SetExitKey(KEY_Q);}
		else if(currentMode == INSERT) {SetExitKey(KEY_NULL);}
		int c ;
		if(IsKeyDown(KEY_BACKSPACE)){
			if(longPressDelay < 1){
				if(isInsertingTitle && title->length) popChar(title);
				else {
					if(currentLine->length < 1 && linesNum > 1) {
						currentLine = note[--linesNum - 1];
						currentLine->length--;
					}
					else if (currentLine->length > 0) popChar(currentLine);
				} 
				longPressDelay = 1;
			} else longPressDelay--;
		}
		if(IsKeyReleased(KEY_BACKSPACE)) longPressDelay = 30;
		if(IsKeyPressed(KEY_CAPS_LOCK)) currentMode = NORMAL;

		while((c = GetCharPressed()) >= 8){
			if(currentMode == NONE ){
				if(c == 'n' || c == 'N'){
					// currentMode = NEW;
					// resetBlur(&bg,&bgTexture,"assets/imgs/notebook.png");
					currentMode = INSERT;
					// TODO : WTF
				} 
				else if(c == 'o' || c == 'O'){
					currentMode = OPEN;
					// bg = LoadImage("assets/imgs/notebook.png");
					// bgTexture = LoadTextureFromImage(bg);

				} 
			}  
			else if (currentMode == OPEN){
				if(c == 'j') currentFileIndex = 
					currentFileIndex < filesCount - 1 ? currentFileIndex + 1 
					: 0;
				else if(c == 'k' )currentFileIndex = 
					currentFileIndex > 0  ? currentFileIndex - 1
					: filesCount - 1;
			}
			else if(currentMode != INSERT){
				if( currentMode == NORMAL && 
					(c == 'i' || c == 'I' || c == 'a' || c == 'A')
				  ){
					currentMode = INSERT;
					SetExitKey(KEY_NULL);
				} 
				else if(c == 't'){
					popChar(title);
					isInsertingTitle = 1;
					currentMode = INSERT;
				}
				else if(c == 'e') {
					currentMode = OPEN;
					// applyBlur(&bg,&bgTexture,2);
				}
			}
			else if(linesNum < LINES_COUNT  && currentMode == INSERT){
				if(currentLine->length > 58 /* 65 */ ){ 
					// printf("new line");
					NoteLine* newLine;
					// addChar(currentLine, '\0');
					newLine = createNoteLine(LINE_LENGTH);
					if(linesNum == LINES_COUNT - 1){
						printf("some stuff\n");
						// note[linesNum] = newLine;
					}
					else {
						addChar(currentLine, '\n');
						note[linesNum++] = newLine;
						currentLine = newLine;
					} 
				}
				else if(c >= 32 ){
					if(isInsertingTitle){
						if(title->length < TITLE_LENGTH - 1) addChar(title,c);
					} 
					else{
						printf("Added char\n");
						addChar(currentLine,c);
					} 
				} 
			}
		}
		if(IsKeyPressed(KEY_ESCAPE)){
			printf("escape\n");
			currentMode = NORMAL;
		}
		if(IsKeyPressed(KEY_BACKSPACE)){
			// printf("backcspace\n");
			if(isInsertingTitle) popChar(title);
			else {
				if(currentLine->length < 1 && linesNum > 1) {
					currentLine = note[--linesNum - 1];
					currentLine->length--;
				}
				else if (currentLine->length > 0) popChar(currentLine);
			} 
		}
		if(IsKeyPressed(KEY_ENTER)){
			if(currentMode == OPEN){
				char path[100];
				sprintf(path,"%s/.local/notes/%s",HOME_DIR,
						fileNames[currentFileIndex]->chars);
				readNote(note,title,year,month,day,&linesNum,path);
				currentLine = note[linesNum -1];
				// remove \n if it's the last thing.
				if(currentLine->chars[currentLine->length - 1] == '\n'){
					currentLine->length--;
				}
				isInsertingTitle = 0;
				currentMode = INSERT;
				// resetBlur(&bg,&bgTexture,"assets/imgs/notebook.png");
			} 
			else {
				if(isInsertingTitle){
					addChar(title,'\n');
					isInsertingTitle = 0;
				} 
				else {
					if(linesNum < LINES_COUNT - 1){
						addChar(currentLine,'\n');
						NoteLine* newLine = createNoteLine(LINE_LENGTH);
						note[linesNum++] = newLine;
						currentLine = newLine;
					}
				}
			}
		}
		Vector2 titleSize = {0.0f,0.0f};
		titleSize = MeasureTextEx(fontSDF,title->chars,40,0);
		Vector2 titlePosition = {SCREEN_WIDTH/2.0f ,TOP_PADDING};
		titlePosition.x = GetScreenWidth()/2 - titleSize.x/2;
		ClearBackground(WHITE);
		DrawLineEx(
				(Vector2){100,70},
				(Vector2){110,40},
				3.0f,fgColor
		);
		DrawLineEx(
				(Vector2){145,70},
				(Vector2){155,40},
				3.0f,fgColor
		);
		for(int l=0; l<LINES_COUNT;l++){
			int yPos = LINE1_Y + l * LINE_HEIGHT;
			DrawLineEx(
					(Vector2){30,yPos},
					(Vector2){930,yPos},
					3.0f,fgColor
			);
		}
		// background
		// DrawTextureEx(
			// bgTexture,(Vector2) {0,0},0.0f,1.0f,WHITE
		// );
		if(currentMode == NONE || currentMode == OPEN){
			/* DrawTextureEx(
				menuTexture,(Vector2) {0,300},
				0.0f,1.0f,(Color){255,255,255,240}
			); */
			DrawRectangle(0,270,960,400,(Color) {0,0,0,230});
			if(currentMode == NONE){
				DrawTextEx(
					fontSDF,"Press Something!",
					(Vector2){
						350,
						LINE1_Y + LINE_HEIGHT * 4
					},
					32,0,WHITE
				);
				DrawTextEx(
					fontSDF,"n : New note",
					(Vector2){
						LINE_X_POSITION * 3,
						LINE1_Y + LINE_HEIGHT * 5
					},
					32,0,WHITE
				);
				DrawTextEx(
					fontSDF,"o : Open a note",
					(Vector2){
						LINE_X_POSITION * 3,
						LINE1_Y + LINE_HEIGHT * 6
					},
					32,0,WHITE
				);
				DrawTextEx(
					fontSDF,"s : Search for a note",
					(Vector2){
						LINE_X_POSITION * 3,
						LINE1_Y + LINE_HEIGHT * 7
					},
					32,0,WHITE
				);
			}
			else {
				for(int i=0;i<filesCount;i++){
					Vector2 nameSize = MeasureTextEx(fontSDF,fileNames[i]->chars,32,0);
					titleSize = MeasureTextEx(fontSDF,title->chars,40,0);
					if(i == currentFileIndex) {
						DrawRectangle(
							0,GetScreenHeight()/2 - 200 + (i * 40) - 5,960,
							nameSize.y + 10, LIGHTGRAY
						);
					}
					DrawTextEx(
						fontSDF,fileNames[i]->chars,
						(Vector2){
							FILES_X_POSITION,
							GetScreenHeight()/2 - 200 + (i * 40)
						},
						32,0,i == currentFileIndex ? BLACK : WHITE
					);
				}
			}
		}
		if(currentMode != NONE && currentMode != OPEN){
			DrawTextEx(
				fontSDF,day,
				(Vector2) DAY_POSITION,32,0,BLACK
			);
			DrawTextEx(
				fontSDF,month,
				(Vector2) MONTH_POSITION,32,0,BLACK
			);
			DrawTextEx(
				fontSDF,year,
				(Vector2) YEAR_POSITION,32,0,BLACK
			);
			DrawTextEx(
				fontSDF,
				TextFormat("%d:%d",linesNum,currentLine->length),
				(Vector2) {850,1025},32,0,BLACK
			);
			DrawTextEx(
				fontSDF,
				TextFormat("%d",title->length),
				(Vector2) {450,1025},32,0,BLACK
			);
		}
		DrawTextEx(
			fontSDF,
			currentMode == NORMAL ? "NORMAL" : currentMode == INSERT ?
			"INSERT" : currentMode == NONE ? "NONE" : currentMode ==  NEW ? 
			"NEW" : currentMode == OPEN ? "OPEN" : "SEARCH",
			(Vector2) MODE_POSITION,
			32,0,BLACK
		);
		DrawTextEx(fontSDF,title->chars,titlePosition,40,0,BLACK);
		for(int i=0; i<linesNum;i++){
			DrawTextEx(
				fontSDF,
				note[i]->chars,
				(Vector2){
					LINE_X_POSITION,
					LINE1_Y + 22  + (LINE_HEIGHT) * i
				},
				32,0,BLACK
			);
		}
		EndDrawing();
	}
	// UnloadImage(bg);
	// UnloadTexture(bgTexture);
	UnloadTexture(menuTexture);
	CloseWindow();
	char savePath[100];
	sprintf(savePath,"%s/.local/notes/%s",HOME_DIR,"myNote");
	writeFile(savePath,day,month,year,title->chars,note,linesNum);

	return 0; 
}
