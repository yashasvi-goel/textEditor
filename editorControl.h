/*
	Author:Yashasvi Goel
	Code Refactored on 21/3/20
*/
#ifndef STRUCT_H
#define STRUCT_H
#include"struct.h"
#endif

int editorRowRxToCx(erow *, int );
int editorRowCxToRx(erow *, int );
void editorScroll();
void bufFree(strBuffer *var);
void bufAppend(strBuffer* ab,const char *s,int len);

#ifndef ROWUTIL_H
#define ROWUTIL_H
#include"rowUtilities.c"
#endif

void editorSetStatusMessage(const char *fmt, ...);