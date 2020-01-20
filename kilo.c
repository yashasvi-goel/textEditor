#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>

struct termios orig_termios;

void disableRawMode(){
	tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios);
}
void enableRawMode(){
	tcgetattr(STDIN_FILENO,&orig_termios);//fetch the terminal attr
	atexit(disableRawMode);
	struct termios raw=orig_termios;

	raw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);//local flags
	//to disable echo; no text appears on screen
	//ISIG diables ctl+C ctrl-z
	//IEXTEN disables ctl+v
	//ICANON disables canonical mode; input isn't altered anymore
	raw.c_iflag&=~(IXON|ICRNL);//input flags
	//IXON disables ctl+s ctl+q

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);//apply the attrs(file,delay,which one);
}
int main(){
	enableRawMode();
	char c;
	while(read(STDIN_FILENO,&c,1)==1&&c!='q'){
		if(iscntrl(c)){
			printf("%d\n",c);
		}
		else{
			printf("%d '%c'",c,c);
		}
	}
	return 0;
}
