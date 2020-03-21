// #ifndef ROWUTIL_H
// #define ROWUTIL_H

#include<stdlib.h>
#include<string.h>
#ifndef STRUCT_H
#define STRUCT_H
#include"struct.h"
#endif
// #include"editorUtilities.c"
#ifndef EDITORUTIL_H
#define EDITORUTIL_H
#include "editorUtilities.c"
#endif
#define _TAB_STOP 8

void editorRowAppendString(erow*,char*,size_t);
void editorUpdateSyntax(erow* );
void editorInsertRow(int ,char * ,size_t );
void editorFreeRow(erow* );
void delRow(int );
void editorRowInsertChar(erow* row,int at,int c);
char *editorRowsToString(int *buflen);
void editorRowDelChar(erow *row, int at);
void editorUpdateRow(erow *row);

// #endif