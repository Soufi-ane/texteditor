#include "files.h"
#include "note.h"
#include "ray.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void getLocalDate(Date* d) {
  time_t t = time(NULL);
  struct tm *locaTime = localtime(&t);
  strftime(d->year, 5, "%Y", locaTime);
  strftime(d->month, 3, "%m", locaTime);
  strftime(d->day, 3, "%d", locaTime);
}

Editor editor = {
  .mode = NORMAL,
  .isChoosingDir = false,
  .isOpeningFile = false,
  .isTakingNote = true,
  .isSearching = false,
  .isNamingFile = false,
  .isDebugging = false,
  .isMenuOpen = false,
  .isInsertingTitle = false,
  .displayedFilesStart = 0,
  .filesCount = 0,
  .numResults = 0,
  .currentFileIndex = 0,
  .cursor = {
    .index = 0,
    .col = 61,
    .last_col = 0,
    .row = -1,
    .width = 15,
    .height = 35,
  },
};

char dir[100];

int main() {
  Note newNote = {
    .linesNum = 1,
    .length = 0,
  };
  editor.note = &newNote;
  editor.currentFileName = malloc(sizeof(char) * FILE_NAME_LENGTH);
  editor.message = malloc(sizeof(char) * LINE_LENGTH);
  editor.searchQuery = malloc(sizeof(char) * 50);
  Color fgColor = GetColor(0xD9D9D9FF);
  editor.HOME_DIR  = getenv("HOME");
  if (editor.HOME_DIR == NULL)
    printf("ERRR!\n");
  sprintf(dir, "%s/.local/notes", editor.HOME_DIR);
  getDirContent(&editor,editor.fileNames, &editor.filesCount, dir);
  getLocalDate(&editor.note->date);
  // char* firstLine = malloc(sizeof(char) * LINE_LENGTH);
  // editor.currentLine = firstLine;
  editor.note->title =  malloc(sizeof(char) * TITLE_LENGTH);
  editor.note->title[0] = '\0';
  editor.note->body = malloc(sizeof(char) * 100);
  editor.note->size = 100;
  editor.note->body[0] = '\0';

  refreshDiplayedFiles(&editor);

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Note");
  SetExitKey(KEY_NULL);
  int fileSize = 0;
  Font fontSDF = {0};
  loadFontSDF(
      // "assets/fonts/JetBrainsMonoNF.ttf",
      "/usr/local/share/fonts/JetBrainsMonoNF.ttf",
      &fileSize,
      128,
      &fontSDF);

  SetTargetFPS(240);
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  printf("files count : %d\n",editor.filesCount);
  while (!WindowShouldClose()) {
    if (editor.mode != INSERT) {
      SetExitKey(KEY_Q);
    } else if (editor.mode == INSERT) {
      SetExitKey(KEY_NULL);
    }
    handleKeys(&editor,editor.fileNames);

    // printf("buffer size : %zu, length : %d\n",editor.note->size,editor.note->length);


    // drawing.
    Vector2 titleSize = {0.0f, 0.0f};
    titleSize = MeasureTextEx(fontSDF, editor.note->title, 32, 0);
    Vector2 titlePosition = {SCREEN_WIDTH / 2.0f, TOP_PADDING};
    titlePosition.x = GetScreenWidth() / 2.0 - titleSize.x / 2;
    ClearBackground(WHITE);
    DrawLineEx((Vector2){100, 70}, (Vector2){110, 40}, 3.0f, fgColor);
    DrawLineEx((Vector2){145, 70}, (Vector2){155, 40}, 3.0f, fgColor);
    for (int l = 0; l < LINES_COUNT - 1; l++) {
      int yPos = LINE1_Y + l * LINE_HEIGHT;
      DrawLineEx((Vector2){30, yPos}, (Vector2){930, yPos}, 3.0f, fgColor);
    }
    DrawTextEx(fontSDF,
      editor.mode == NORMAL ? "NORMAL" : "INSERT",
      (Vector2)MODE_POSITION,
      32, 0, BLACK);

    //render title
    for (int i = 0; i < strlen(editor.note->title); i++) {
      if (editor.note->title[i] != '\n') {
        DrawTextCodepoint(fontSDF,
          editor.note->title[i],
          (Vector2){titlePosition.x + 14 * i, titlePosition.y},
          32, BLACK);
      }
    }
    /* DrawRectangle(
      editor.isInsertingTitle ? titlePosition.x + 20 * editor.title->length
        : LINE_X_POSITION + editor.currentLine->length * 14,
      editor.isInsertingTitle ? titlePosition.y
        : LINE1_Y + 22 + LINE_HEIGHT * (i - 1),
      editor.isInsertingTitle ? 19 : 14,
      editor.isInsertingTitle ? 40 : 30,
      BLACK); */

    if(editor.isNamingFile) {
      char placeholder[] = "File name";
      Vector2 nameSize = {0.0f, 0.0f};
      nameSize = MeasureTextEx(
        fontSDF, 
        strlen(editor.currentFileName) > 0 ? editor.currentFileName :
        placeholder, 28, 0);
      Vector2 namePosition = {SCREEN_WIDTH / 2.0f, 400};
      namePosition.x = GetScreenWidth() / 2.0 - nameSize.x / 2;
      DrawRectangle(0, 365, GetScreenWidth(), 100,(Color){0, 0, 0, 230});     
      DrawTextEx(fontSDF,
        strlen(editor.currentFileName) > 0 ? editor.currentFileName :
        placeholder, namePosition , 28, 0, (Color){255, 255, 255, 180});
    }

    // old cursor
    // if(editor.isTakingNote){
      /* DrawRectangle(
        // magic numbers
        editor.cursor.col * (editor.cursor.row < 0 ? 7 : 14) 
        + 50 + (editor.cursor.row < 0 ? 1 : 0) ,
        editor.cursor.row < 0 ? TOP_PADDING : 
        (editor.cursor.row - 1) * LINE_HEIGHT + LINE1_Y + 22,
        editor.cursor.width, editor.cursor.height,
        editor.isNamingFile ? WHITE :BLACK
      ); */
    // }
    //
    // empty note cursor
    if(!editor.note->length){
      DrawRectangle(
        LINE_X_POSITION,
        LINE1_Y - TEXT_HEIGHT,
        editor.cursor.width, editor.cursor.height,
        BLACK
      );
    }

    //render body
    int y_offset = 0;
    int i,x_offset;
    for (i = 0,x_offset=0; i < editor.note->length; i++,x_offset++) {
      // line wrap
      if(x_offset >= 62) {
        y_offset++;
        x_offset = 0;
      }
      //cursor
      if(editor.cursor.index == i) {
        DrawRectangle(
          LINE_X_POSITION + 14 * x_offset ,
          LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset),
          editor.cursor.width, editor.cursor.height,
          BLACK
        );
      }
      if(editor.mode == INSERT && editor.cursor.index == i+1) {
        DrawRectangle(
          LINE_X_POSITION + 14 * (x_offset + 1) ,
          LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset),
          editor.cursor.width, editor.cursor.height,
          BLACK
        );
      }
      if (editor.note->body[i] != '\n') {
        DrawTextCodepoint(fontSDF,
          editor.note->body[i],
          (Vector2){LINE_X_POSITION + 14 * x_offset,
          LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset)},
          32,
          editor.cursor.index == i ? RED : BLACK);
      } else {
        y_offset++;
        x_offset = -1;
      }
    }

    if (editor.mode == NORMAL) {
      // if (!editor.isChoosingDir && !editor.isOpeningFile) {
      if(editor.isMenuOpen || editor.isOpeningFile){
        DrawRectangle(0, 270, 960, 400, (Color){0, 0, 0, 230});
      }
      if (editor.isMenuOpen) {
        DrawTextEx(fontSDF,
          "Press Something!",
          (Vector2){350, LINE1_Y + LINE_HEIGHT * 4},
          32, 0, WHITE);
        DrawTextEx(fontSDF,
          "n : New note",
          (Vector2){LINE_X_POSITION * 3, LINE1_Y + LINE_HEIGHT * 5},
          32, 0, WHITE);
        DrawTextEx(fontSDF,
          "o : Open a note",
          (Vector2){LINE_X_POSITION * 3, LINE1_Y + LINE_HEIGHT * 6},
          32, 0, WHITE);
        DrawTextEx(fontSDF,
          "d : Select directory",
          (Vector2){LINE_X_POSITION * 3, LINE1_Y + LINE_HEIGHT * 7},
          32, 0, WHITE);
      } else if (editor.isOpeningFile) {
      }
      /* else {
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
      } */
      // }
      // else {
      if (editor.isOpeningFile) {
        if (editor.isSearching) {
          if (strlen(editor.searchQuery)) {
            int i;
            for (i = 0; i < strlen(editor.searchQuery); i++) {
              DrawTextCodepoint(fontSDF,
                editor.searchQuery[i],
                (Vector2){FILES_X_POSITION + i * 14,
                   GetScreenHeight() / 2.0 - 255},
                32, WHITE);
            }
            /* DrawTextCodepoint(
                    fontSDF,' ',
                    (Vector2){
                            FILES_X_POSITION + i * 14,
                            GetScreenHeight()/2 - 255
                    },
                    32,WHITE
            ); */
            DrawRectangle(FILES_X_POSITION + i * 14,
              GetScreenHeight() / 2 - 255,
              14, 30, WHITE);
          } else {
            DrawTextEx(fontSDF,
              "Search...",
              (Vector2){FILES_X_POSITION, GetScreenHeight() / 2 - 253},
              28, 0, (Color){255, 255, 255, 180});
          }
        } else {
          DrawTextEx(fontSDF,
            "Press / to search",
            (Vector2){FILES_X_POSITION, GetScreenHeight() / 2 - 253},
            28, 0, (Color){255, 255, 255, 180});
        }
        for (
            int i = 0;
            i < (editor.isSearching ? editor.numResults : 
              (editor.filesCount < MAX_DISPLAYED_FILES ? editor.filesCount:
               MAX_DISPLAYED_FILES ));
            i++) 
        {
          Vector2 nameSize = MeasureTextEx(fontSDF, editor.displayedNames[i], 32, 0);
          // titleSize = MeasureTextEx(fontSDF, editor.title->chars, 40, 0);
          if (i == editor.currentFileIndex) {
            DrawRectangle(0,
              GetScreenHeight() / 2 - 200 + (i * 40) - 5,
              960, nameSize.y + 10, LIGHTGRAY);
          }

          DrawTextEx(fontSDF,
           editor.isSearching ? editor.resultNames[i] : 
           editor.displayedNames[i],
           (Vector2){FILES_X_POSITION,
             GetScreenHeight() / 2 - 200 + (i * 40)},
           32, 0,
           i == editor.currentFileIndex ? BLACK : WHITE);
        }
      }
    } 
      DrawTextEx(fontSDF, editor.note->date.day, (Vector2)DAY_POSITION, 32, 0, BLACK);
      DrawTextEx(fontSDF, editor.note->date.month, (Vector2)MONTH_POSITION, 32, 0, BLACK);
      DrawTextEx(fontSDF, editor.note->date.year, (Vector2)YEAR_POSITION, 32, 0, BLACK);

      /* DrawTextEx(fontSDF,
        TextFormat("%02d:%02d", editor.note->linesNum, editor.currentLine->length),
        (Vector2){850, 1025},
        32, 0, BLACK); */

      DrawTextEx(fontSDF,
        "1/1",
        (Vector2){450, 1025},
        32, 0, BLACK);

    // debugging 
    if(editor.isDebugging) {
      DrawRectangle(0,SCREEN_HEIGHT - 600, 400,600, BLACK);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]","isInsertingTitle",editor.isInsertingTitle),
        (Vector2){20, SCREEN_HEIGHT - 570},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]", "isTakingNote",editor.isTakingNote),
        (Vector2){20, SCREEN_HEIGHT - 540},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]", "isMenuOpen",editor.isMenuOpen),
        (Vector2){20, SCREEN_HEIGHT - 510},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]", "isOpeningFile",editor.isOpeningFile),
        (Vector2){20, SCREEN_HEIGHT - 480},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]", "isSearching",editor.isSearching),
        (Vector2){20, SCREEN_HEIGHT - 450},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]", "isNamingFile",editor.isNamingFile),
        (Vector2){20, SCREEN_HEIGHT - 420},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]", "isChoosingDir",editor.isChoosingDir),
        (Vector2){20, SCREEN_HEIGHT - 390},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%s (%d,%d)", "cursor",
          editor.cursor.col,editor.cursor.row),
        (Vector2){20, SCREEN_HEIGHT - 360},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-17s [%d]", "index",editor.cursor.index),
        (Vector2){20, SCREEN_HEIGHT - 330},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-16s [%03d]", "line length",00),
        (Vector2){20, SCREEN_HEIGHT - 300},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%-16s [%03d]", "lines count",editor.note->linesNum),
        (Vector2){20, SCREEN_HEIGHT - 270},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%s: [%s]", "file name",editor.currentFileName),
        (Vector2){20, SCREEN_HEIGHT - 240},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%s: [%s]", "message",editor.message),
        (Vector2){20, SCREEN_HEIGHT - 210},
        32, 0, GREEN);
      DrawTextEx(fontSDF,
        TextFormat("%s: [%s] [%d]", "title",editor.note->title,
          strlen(editor.note->title)),
        (Vector2){20, SCREEN_HEIGHT - 180},
        32, 0, GREEN);
      /* DrawTextEx(fontSDF,
        TextFormat("%s: [%d]", "total length",editor.note->length,
          editor.note->title->length),
        (Vector2){20, SCREEN_HEIGHT - 150}, 
        32, 0, GREEN);*/
      /* DrawTextEx(fontSDF,
        TextFormat("%-17s: [%d]", "position",editor.cursor.index,
          editor.note->title->length),
        (Vector2){20, SCREEN_HEIGHT - 120},
        32, 0, GREEN); */
    }
    DrawTextEx(fontSDF,
      TextFormat("%s",editor.message),
      (Vector2){20, SCREEN_HEIGHT - 60},
      32, 0, BLACK);

    EndDrawing();
  }
  CloseWindow();
  int i;
  // deleteLine(&editor.note->title);
  // deleteLine(&firstLine);
  writeFile(&editor);
  //todo
  /* for (i = 1; i < editor.note.linesNum; i++) {
    deleteLine(&editor.note.body[i]);
  }
  for (i = 0; i < editor.filesCount; i++) {
    deleteLine(&editor.fileNames[i]);
  }
  for (i = 0; i < editor.numResults; i++) {
    deleteLine(&editor.resultNames[i]);
  } */
  UnloadFontData(fontSDF.glyphs, fontSDF.glyphCount);
  // deleteLine(&editor.searchQuery);
  // UnloadTexture(fontSDF.texture);
  return 0;
}
