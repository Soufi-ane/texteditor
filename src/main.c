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
Line* fileNames[MAX_FILES_NUM];
Line* resultNames[MAX_FILES_NUM];
int isSearching = 0;
int numResults = 0;
char year[5],month[3],day[3];
int isInsertingTitle = 1;
int linesNum = 1;
int filesCount = 0;
int longPressDelay = 100;
int isChoosingDir = 0;

int main(){
	Line* searchQuery = createLine(50);
	Color fgColor = GetColor(0xD9D9D9FF);
	const char* HOME_DIR = getenv("HOME");
	if(HOME_DIR == NULL) printf("ERRR!\n"); 
	sprintf(dir,"%s/.local/notes",HOME_DIR);
	getDirContent(fileNames,&filesCount,dir);
	getLocalDate(year,month,day);
	Line* firstLine = createLine(LINE_LENGTH);
	Line* currentLine = firstLine;
	Line* note[LINES_COUNT - 1];
	Line* title = createLine(TITLE_LENGTH);
	note[0] = firstLine;


	InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"Note");
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

	SetTargetFPS(240);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	while(!WindowShouldClose()){
		if(currentMode != INSERT) {SetExitKey(KEY_Q);}
		else if(currentMode == INSERT) {SetExitKey(KEY_NULL);}
		int c ;
		if(IsKeyDown(KEY_BACKSPACE)){
			if(longPressDelay < 1){
				if(isSearching){
					popChar(searchQuery);
					find(
						fileNames,filesCount,searchQuery,
						resultNames,&numResults
					);
				} 
				if(isInsertingTitle) popChar(title);
				else {
					if(currentLine->length < 1 && linesNum > 1) {
						// deleteLine(&currentLine);
						currentLine = note[--linesNum - 1];
						currentLine->length--;
					}
					else if (currentLine->length > 0) popChar(currentLine);
				} 
				longPressDelay = 3;
			} else longPressDelay--;
		}
		if(IsKeyReleased(KEY_BACKSPACE)) longPressDelay = 100;
		if(IsKeyPressed(KEY_CAPS_LOCK) ){
			if(!isSearching) currentMode = NORMAL;
			else{
				isSearching = 0;
				emptyLine(searchQuery,50);
			} 
		} 
		while((c = GetCharPressed()) >= 8){
			if(currentMode == NONE ){
				if(c == 'n'){
					currentMode = INSERT;
				} 
				else if(c == 'o'){
					currentMode = OPEN;
				} 
				else if(c == 'd'){
					currentMode = INSERT;
					isChoosingDir = 1;
				} 
			}  
			else if (currentMode == OPEN){
				if(isSearching){
					if(c >= 32 && searchQuery->length < 50){
						addChar(searchQuery,c);
						find(
							fileNames,
							filesCount,
							searchQuery,
							resultNames,
							&numResults
						);
					} 
				}
				else {
					if (c == '/'){
						isSearching = 1;
						find(
							fileNames,
							filesCount,
							searchQuery,
							resultNames,
							&numResults
						);
					} 
					else if(c == 'j') currentFileIndex = 
						currentFileIndex < filesCount - 1 ? currentFileIndex + 1 
						: 0;
					else if(c == 'k' )currentFileIndex = 
						currentFileIndex > 0  ? currentFileIndex - 1
						: filesCount - 1;
				}
			}
			//todo
			/* else if(currentMode == NORMAL){
				if(c == 'k') {
					currentLine = note[linesNum - 2];
				}
			} */
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
				}
			}
			else if(linesNum < LINES_COUNT  && currentMode == INSERT){
				if(currentLine->length > 60 /* 65 */ ){ 
					// printf("new line");
					Line* newLine;
					// addChar(currentLine, '\0');
					newLine = createLine(LINE_LENGTH);
					if(linesNum == LINES_COUNT - 1){
						printf("some stuff\n");
						// note[linesNum] = newLine;
					}
					else {
						addChar(currentLine, '\n');
						note[linesNum++] = newLine;
						currentLine = newLine;
						addChar(currentLine,c);
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
			if(isSearching){
				popChar(searchQuery);
				find(
					fileNames,
					filesCount,
					searchQuery,
					resultNames,
					&numResults
				);
			} 
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
			} 
			else {
				if(isInsertingTitle){
					addChar(title,'\n');
					isInsertingTitle = 0;
				} 
				else {
					if(linesNum < LINES_COUNT - 1){
						addChar(currentLine,'\n');
						Line* newLine = createLine(LINE_LENGTH);
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
		if(currentMode == NONE || currentMode == OPEN){
			DrawRectangle(0,270,960,400,(Color) {0,0,0,230});
			if(currentMode == NONE ){
				if(!isChoosingDir){
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
						fontSDF,"d : Select directory",
						(Vector2){
							LINE_X_POSITION * 3,
							LINE1_Y + LINE_HEIGHT * 7
						},
						32,0,WHITE
					);
				} else {
					int i;
					for(i=0;i<100;i++){
						if(dir[i]== '\0') break;
						DrawTextCodepoint(
							fontSDF,dir[i],
							(Vector2){
								FILES_X_POSITION + i * 14,
								GetScreenHeight()/2 - 120
							},
							32,WHITE
						);
					}
					DrawRectangle(
						FILES_X_POSITION + i * 14,
						GetScreenHeight()/2 - 120,
						14,30,WHITE
					);
				}
			}
			else {
				/* DrawRectangleRoundedLines(
					(Rectangle){70,280,820,45},
					0.8f,
					200,
					WHITE
				); */
				if(isSearching){
					if(searchQuery->length){
						int i ;
						for(i=0;i<searchQuery->length;i++){
							DrawTextCodepoint(
								fontSDF,searchQuery->chars[i],
								(Vector2){
									FILES_X_POSITION + i * 14,
									GetScreenHeight()/2 - 255
								},
								32,WHITE
							);
						}
						/* DrawTextCodepoint(
							fontSDF,' ',
							(Vector2){
								FILES_X_POSITION + i * 14,
								GetScreenHeight()/2 - 255
							},
							32,WHITE
						); */
						DrawRectangle(
							FILES_X_POSITION + i * 14,
							GetScreenHeight()/2 - 255,
							14,30,WHITE
						);
					}
					else {
						DrawTextEx(
						fontSDF,"Search...",
						(Vector2){
							FILES_X_POSITION,
							GetScreenHeight()/2 - 253
						},
						28,0,(Color){255,255,255,180}
					);
					}
				} else {
					DrawTextEx(
						fontSDF,"Press / to search",
						(Vector2){
							FILES_X_POSITION,
							GetScreenHeight()/2 - 253
						},
						28,0,(Color){255,255,255,180}
					);
				}
				for(int i=0;i<(isSearching ? numResults : filesCount);i++){
					Vector2 nameSize = MeasureTextEx(fontSDF,fileNames[i]->chars,32,0);
					titleSize = MeasureTextEx(fontSDF,title->chars,40,0);
					if(i == currentFileIndex) {
						DrawRectangle(
							0,GetScreenHeight()/2 - 200 + (i * 40) - 5,960,
							nameSize.y + 10, LIGHTGRAY
						);
					}
					DrawTextEx(
						fontSDF,
						isSearching ? resultNames[i]->chars : fileNames[i]->chars,
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
		for(int i=0;i<title->length;i++) {
			if(title->chars[i] != '\n'){
				DrawTextCodepoint(
					fontSDF,
					title->chars[i]
					,(Vector2){
						titlePosition.x + 20 * i,
						titlePosition.y
						}
					,40,BLACK
				);
			}
		}
		int i;
		for(i=0; i<linesNum;i++){
			for(int j=0;j<note[i]->length;j++){
				if(note[i]->chars[j] != '\n'){
					DrawTextCodepoint(
						fontSDF,
						note[i]->chars[j],
						(Vector2){
							LINE_X_POSITION + j * 14,
							LINE1_Y + 22  + (LINE_HEIGHT) * i
						},
						32,BLACK
					);
				}
			}
		}
		DrawRectangle(
			isInsertingTitle ? titlePosition.x + 20 * title->length :
			LINE_X_POSITION + currentLine->length * 14 ,
			isInsertingTitle ? titlePosition.y  :
			LINE1_Y +22 + LINE_HEIGHT* (i-1) ,
			isInsertingTitle ? 19 :14,isInsertingTitle ? 40 :30,BLACK
		);
		DrawFPS(10,10);
		EndDrawing();
	}
	CloseWindow();
	char savePath[100];
	sprintf(savePath,"%s/.local/notes/%s",HOME_DIR,"myNote");
	writeFile(savePath,day,month,year,title->chars,note,linesNum);
	int i;
	deleteLine(&title);
	deleteLine(&firstLine);
	for(i=1;i<linesNum;i++){
		deleteLine(&note[i]);
	}
	for(i=0;i<filesCount;i++){
		deleteLine(&fileNames[i]);
	}
	for(i=0;i<numResults;i++){
		deleteLine(&resultNames[i]);
	}
	UnloadFontData(fontSDF.glyphs,fontSDF.glyphCount);
	deleteLine(&searchQuery);
	// UnloadTexture(fontSDF.texture);
	return 0; 
}
