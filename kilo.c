#include<unistd.h>
#include<sys/ioctl.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<string.h>

#define ctrl(k) ((k) & 0x1f)
#define str_INIT {NULL,0}
#define version "0.0.1"

typedef struct editorConfig{
	int screenRows;
	int screenColumns;
	struct termios orig_termios;
}editorConfig;
typedef struct strBuffer{
	char *buffer;
	int len;
}strBuffer;
void bufFree(strBuffer *var){
	free(var->buffer);
}
void bufAppend(strBuffer* ab,const char *s,int len){
	char *new=realloc(ab->buffer,ab->len+len);
	if(new==NULL)
		return;
	memcpy(&new[ab->len],s,len);
	ab->buffer=new;
	ab->len+=len;
}
struct editorConfig E;
void clearScreen();
char readKey();
int cursorPosition(int*,int*);
void die(const char *s)
{
	clearScreen(0);
	perror(s);
	exit(1);
}
int getWindowSize(int *rows, int *cols)
{
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
	  if(write(STDOUT_FILENO,"\x1b[500C\x1b[600B",12)!= 12)
		  return -1;
	  return cursorPosition(rows,cols);
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
void drawTildes(strBuffer* ab)//draws tildes
{
	int y;
	for(y=0;y<E.screenRows;y++)
	{
		if(y==E.screenRows / 3){
			char welcome[80];
			int welcomelen = snprintf(welcome, sizeof(welcome),"Kilo editor -- version %s", version);
			if(welcomelen > E.screenColumns)
				welcomelen = E.screenColumns;
			int padding =(E.screenColumns-welcomelen)/2;
			if(padding){
				bufAppend(ab,"~",1);
				padding--;
			}
			while(padding--)
				bufAppend(ab," ",1);
			bufAppend(ab, welcome, welcomelen);
		}
		else{
			bufAppend(ab,"~",1);
		}
		bufAppend(ab,"\x1b[K",3);
//		write(STDOUT_FILENO,"~",1);
		if(y<E.screenRows-1)
			bufAppend(ab,"\r\n",2);
//			write(STDOUT_FILENO,"\r\n",3);
	}
}
void clearScreen(int options)
{
	strBuffer ab=str_INIT;
	bufAppend(&ab,"\x1b[?25l",6);
//	bufAppend(&ab,"\x1b[2J",4);
	bufAppend(&ab,"\x1b[H",3);
	if(options==1)
		drawTildes(&ab);
	bufAppend(&ab,"\x1b[H",3);
	bufAppend(&ab,"\x1b[?25l",6);

	write(STDOUT_FILENO,ab.buffer,ab.len);
	bufFree(&ab);
}
int cursorPosition(int* rows,int* cols)
{
	char buf[32];
	unsigned int i=0;
	if(write(STDOUT_FILENO,"\x1b[6n",4)!=4)
		return -1;
	while(i<sizeof(buf) -1){
		if(read(STDIN_FILENO,&buf[i],1)==1) break;
		if(buf[i]=='R') break;
		i++;
	}
	buf[i]='\0';
	if(buf[0]!='\x1b' || buf[1]!='[') return -1;
	if(sscanf(&buf[2], "%d;%d",rows,cols)!=2) return -1;
	return 0;
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
