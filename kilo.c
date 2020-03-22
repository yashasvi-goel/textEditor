/*
	Author:Yashasvi Goel
	Code Refactored on 21/3/20
*/
#include "kilo.h"


int main(int argc,char *argv[]){
	enableRawMode();
	initEditor();
	if(argc>=2){
		editorOpen(argv[1]);
	}
	editorSetStatusMessage(DISPLAYED_STATUS);

	while(1){
		clearScreen(1);
		processKeypress();
	}
	disableRawMode();
	return 0;
}
