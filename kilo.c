#include<unistd.h>
#include<sys/ioctl.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>

#define ctrl(k) ((k) & 0x1f)

struct editorConfig{
	int screenRows;
	int screenColumns;
	struct termios orig_termios;
};

struct editorConfig E;
void clearScreen();
char readKey();
void die(const char *s)
{
	clearScreen(0);
	perror(s);
	exit(1);
}
int getWindowSize(int *rows, int *cols){
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
	  if(write(STDOUT_FILENO,"\x1b[500C\x1b[600B",12)!= 12)
		  return -1;
	  readKey();
    return -1;
  }
  else{
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}
void initEditor(){
	if(getWindowSize(&E.screenRows,&E.screenColumns)==-1)
		die("Window");
}
void disableRawMode()
{
	if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&E.orig_termios)==-1)
		die("tcsetattr");
}
void enableRawMode()
{
	if(tcgetattr(STDIN_FILENO,&E.orig_termios)==-1)//fetch the terminal attr
		die("tcgetattr");
	atexit(disableRawMode);
	struct termios raw=E.orig_termios;

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
char readKey()//reads input character-by-character
{
	int nread=0;
	char c='\0';
	while((nread=read(STDIN_FILENO,&c,1))==-1){
		if(nread==-1)
			die("read");
	}
	return c;
}
void processKeypress()//manages all the editor modes and special characters
{
	char curr=readKey();
	switch(curr){//add all the mode controls below
		case ctrl('q'):
			clearScreen(0);
			exit(0);
			break;
	}
}
void drawTildes()//draws sides
{
	int y;
	for(y=0;y<E.screenRows;y++)
		write(STDOUT_FILENO,"~\r\n",3);
}
void clearScreen(int options){
	write(STDOUT_FILENO,"\x1b[2J",4);
	write(STDOUT_FILENO,"\x1b[H",3);
	if(options==1)
		drawTildes();
	write(STDOUT_FILENO, "\x1b[H",3);
}
int cursorPosition(int* rows,int* cols)
{
	if(write(STDOUT_FILENO,"\x1b[6n",4)!=4)
		return -1;
	printf("\r\n");
	char c;
	while(read(STDIN_FILENO,&c,1)==1){
		if(ctrl(c))
			printf("%d\r\n",c);
		else
			printf("%d ('%c')\r\n", c, c);
	}
	readKey();
	return -1;
}
int main(){
	enableRawMode();
	initEditor();
	while(1){
		clearScreen(1);
		processKeypress();
	}
	return 0;
}
