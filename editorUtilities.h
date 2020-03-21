// #ifndef EDITORUTIL_H
// #define EDITORUTIL_H

#ifndef STRUCT_H
#define STRUCT_H
#include"struct.h"
#endif
#ifndef RAWMODE_H
#define RAWMODE_H
#include "rawMode.c"
#endif
#ifndef EDITORCTRL_H
#define EDITORCTRL_H
#include"editorControl.c"
#endif
#ifndef ROWUTIL_H
#define ROWUTIL_H
#include"rowUtilities.c"
#endif
#ifndef CURSOR_H
#define CURSOR_H
#include"cursorUtilities.c"
#endif



void clearScreen(int options);

// void editorScroll();
void drawTildes(strBuffer* ab);
void drawStatusBar(strBuffer* ab);
void drawMessageBar(strBuffer *ab);
int is_separator(int c);
void editorUpdateSyntax(erow* row);
// int cursorPosition(int*,int*);
void moveCursor(int);
int getWindowSize(int *rows, int *cols);


void editorOpen(char *filename);

void editorSelectSyntaxHighlight();
int editorSyntaxToColor(int hl);
char* editorPrompt(char* prompt, void(*callback)(char*,int));
#ifndef SEARCH_H
#define SEARCH_H
#include"search.c"
#endif
#ifndef SAVE_H
#define SAVE_H
#include"save.c"
#endif
void processKeypress();
void editorDelChar();
void editorInsertNewline();
void editorDelChar();
void editorInsertChar(int c);
// 


// #endif