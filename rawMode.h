// #ifndef RAWMODE_H
// #define RAWMODE_H

#include<termios.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<unistd.h> 

// #include"all.c"
// #include"kilo.h"
#ifndef STRUCT_H
#define STRUCT_H
#include"struct.h"
#endif
#ifndef CURSOR_H
#define CURSOR_H
#include"cursorUtilities.c"
#endif

void die(const char *s);
int readKey();
// #ifndef EDITORCTRL_H
// #define EDITORCTRL_H
// #include"editorControl.c"
// #endif
#ifndef EDITORUTIL_H
#define EDITORUTIL_H
#include "editorUtilities.c"
#endif



// #ifndef EDITORCTRL_H
// #define EDITORCTRL_H
// #include "editorControl.h"
// #endif


void disableRawMode();
void enableRawMode();
void initEditor();
void clear();

// typedef struct editorConfig{
// 	int cx,cy;//cx is posn in the line,without indentation
// 	int rx;//actual posn in line
// 	int rowOffset,colOffset;
// 	int screenRows;
// 	int screenColumns;
// 	char *file;
// 	int numRows;
// 	int dirty;
// 	erow *row;
// 	char status[80];
// 	time_t statusTime;
// 	struct editorSyntax *syntax;
// 	struct termios orig_termios;
// }editorConfig;

// #endif