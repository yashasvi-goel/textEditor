#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>

#define ctrl(k) ((k) & 0x1f)

struct termios orig_termios;

void die(const char *s)
{
	perror(s);
	exit(1);
}
void disableRawMode()
{
	if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios)==-1)
		die("tcsetattr");
}
void enableRawMode()
{
	if(tcgetattr(STDIN_FILENO,&orig_termios)==-1)//fetch the terminal attr
		die("tcgetattr");
	atexit(disableRawMode);
	struct termios raw=orig_termios;

	raw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);//local flags
	//to disable echo; no text appears on screen
	//ISIG diables ctl+C ctrl-z
	//IEXTEN disables ctl+v
	//ICANON disables canonical mode; input isn't altered anymore
	raw.c_iflag&=~(IXON|ICRNL|INPCK|ISTRIP|BRKINT);//input flags
	//IXON disables ctl+s ctl+q
	raw.c_oflag&= ~(OPOST);//OPOST disables all post-processing including carriage operator
	//Following are to edit config of read()
	raw.c_cc[VMIN]=0;//min chars after which read returns
	raw.c_cc[VTIME]=10;//max time read() waits before returning


	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1)//apply the attrs(file,delay,which one)
		die("tcsetattr");
}
int main()
{
	enableRawMode();
	//char c;
	while(1){
		char c='\0';
		if(read(STDIN_FILENO,&c,1)==-1)
			die("read");
		if(c!=ctrl('q')){
			if(iscntrl(c)){
				printf("%d\r\n",c);
			}
			else{
				printf("%d '%c'\r\n",c,c);
			}
		}
		else
			break;
	}
	return 0;
}
