
#include<string.h>
#include<stdlib.h>
#ifndef STRUCT_H
#define STRUCT_H
#include"struct.h"
#endif
#ifndef EDITORUTIL_H
#define EDITORUTIL_H
#include "editorUtilities.c"
#endif
#ifndef ROWUTIL_H
#define ROWUTIL_H
#include"rowUtilities.c"
#endif



#define DISPLAYED_PROMPT "Search: %s (Use ESC/Arrows/Enter)"


void search();
void searchCallback(char* query,int key);

// #endif