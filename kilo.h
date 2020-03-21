// #ifndef KILOs_H
// #define KILOs_H

#define _DEFAULT_SOURCE
//#define _BSD_SOURCE
#define _GNU_SOURCE

#include<stdlib.h>
#include<fcntl.h>


#include<ctype.h>
#include<string.h>

#ifndef STRUCT_H
#define STRUCT_H
#include"struct.h"
#endif

#ifndef RAWMODE_H
#define RAWMODE_H
#include "rawMode.c"
#endif

#ifndef EDITORUTIL_H
#define EDITORUTIL_H
#include "editorUtilities.c"
#endif
#define DISPLAYED_STATUS "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl+F= find"

// #endif