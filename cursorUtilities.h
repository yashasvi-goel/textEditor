/*
	Author:Yashasvi Goel
	Code Refactored on 21/3/20
*/
// #ifndef CURSOR_H
// #define CURSOR_H

// #include"kilo.h"
#include<unistd.h> 
#include<stdio.h>

#ifndef STRUCT_H
#define STRUCT_H
#include"struct.h"
#endif

// #ifndef EDITORUTIL_H
// #define EDITORUTIL_H
// #include "editorUtilities.c"
// #endif

void moveCursor(int);
int cursorPosition(int*,int*);

// #endif