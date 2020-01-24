//#define _DEFAULT_SOURCE
//#define _BSD_SOURCE
//#define _GNU_SOURCE

#include<unistd.h>
#include<sys/ioctl.h>
#include <sys/types.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<string.h>

#define ctrl(k) ((k) & 0x1f)
#define str_INIT {NULL,0}
#define version "0.0.1"

enum editorKey {
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN
};
typedef struct erow {
  int size;
  char *chars;
} erow;
typedef struct editorConfig{
	int cx,cy;
	int rowOffset;
	int screenRows;
	int screenColumns;
	int numRows;
	erow *row;
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
int readKey();
void moveCursor(int);
int cursorPosition(int*,int*);
void die(const char *s)
{
	clearScreen(0);
	perror(s);
	exit(1);
}
void editorAppendRow(char *line,size_t linelen){

	E.row=realloc(E.row,sizeof(erow)*(E.numRows+1));

	int at=E.numRows;
	E.row[at].size=linelen;
	E.row[at].chars=malloc(linelen+1);
	memcpy(E.row[at].chars, line ,linelen);
	E.row[at].chars[linelen]='\0';
	E.numRows++;
}
void editorOpen(char *filename)
{
	FILE *fp=fopen(filename,"r");
	if(!fp)
		die("fopen");
	char *line=NULL;
	size_t *linecap=0;
	ssize_t linelen;
	while((linelen = getline(&line, &linecap, fp))!=-1){
		while (linelen > 0 && (line[linelen - 1] == '\n' ||line[linelen - 1] == '\r'))
			linelen--;
		editorAppendRow(line,linelen);
	}
	free(line);
	fclose(fp);
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
	E.cx=0;
	E.cy=0;
	E.rowOffset=0;
	E.numRows=0;
	E.row=NULL;
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
	raw.c_cc[VTIME]=1;//max time read() waits before returning


	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1)//apply the attrs(file,delay,which one)
		die("tcsetattr");
}
int readKey()//reads input character-by-character
{
	int nread=0;
	char c='\0';
	while((nread=read(STDIN_FILENO,&c,1))==-1){
		if(nread==-1)
			die("read");
	}
	if(c == '\x1b'){
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
		if (seq[0] == '[')
		{
			if (seq[1] >= '0' && seq[1] <= '9')
			{
				if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
				if (seq[2] == '~')
				{
					switch (seq[1]){
						case '1': return HOME_KEY;
						case '3': return DEL_KEY;
						case '4': return END_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						case '7': return HOME_KEY;
						case '8': return END_KEY;
					}
				}
			}
			else{
				switch (seq[1]){
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
				}
			}
		}
		else if (seq[0] == 'O'){
			switch (seq[1]){
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
			}
		}
		return '\x1b';
	}
	else{
		return c;
	}
}
void processKeypress()//manages all the editor modes and special characters
{
	int curr=readKey();
	switch(curr){//add all the mode controls below
		case ctrl('q'):
			clearScreen(0);
			exit(0);
			break;
		case HOME_KEY:
			E.cx=0;
			break;
		case END_KEY:
			E.cx=E.screenColumns-1;
			break;
		case PAGE_UP:
		case PAGE_DOWN:
			{
				int times = E.screenRows;
				while (times--)
					moveCursor(curr == PAGE_UP ? ARROW_UP : ARROW_DOWN);
			}
			break;
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			moveCursor(curr);
			break;
	}
}
void drawTildes(strBuffer* ab)//draws tildes
{
	int y;
	for(y=0;y<E.screenRows;y++)
	{
		int filerow=y+E.rowOffset;
		if(filerow>=E.numRows){
			if(E.numRows==0 && y==E.screenRows / 3){
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
		}
		else{
			int len = E.row[filerow].size;
			if (len > E.screenColumns) len = E.screenColumns;
			bufAppend(ab, E.row[filerow].chars, len);
		}

		bufAppend(ab,"\x1b[K",3);
		//		write(STDOUT_FILENO,"~",1);
		if(y<E.screenRows-1)
			bufAppend(ab,"\r\n",2);
		//			write(STDOUT_FILENO,"\r\n",3);
	}
}
void editorScroll(){
	if(E.cy<E.rowOffset){
		E.rowOffset=E.cy;
	}
	if(E.cy>=E.rowOffset+E.screenRows){
		E.rowOffset=E.cy- E.screenRows+1;
	}
}
void clearScreen(int options)
{
	editorScroll();

	strBuffer ab=str_INIT;
	bufAppend(&ab,"\x1b[?25l",6);
	if(options==0)
		bufAppend(&ab,"\x1b[2J",4);
	bufAppend(&ab,"\x1b[H",3);
	if(options==1)
		drawTildes(&ab);

	//	bufAppend(&ab,"\x1b[H",3);
	char buf[32];
	//	E.cx=12;
	snprintf(buf,sizeof(buf),"\x1b[%d;%dH",E.cy+1,E.cx+1);
	bufAppend(&ab,buf,strlen(buf));

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
void moveCursor(int key) {
	switch (key) {
		case ARROW_LEFT:
			if(E.cx!=0)
				E.cx--;
			break;
		case ARROW_RIGHT:
			if(E.cx!=E.screenColumns-1)
				E.cx++;
			break;
		case ARROW_UP:
			if(E.cy!=0)
				E.cy--;
			break;
		case ARROW_DOWN:
			if(E.cy<E.numRows)
				E.cy++;
			break;
	}
}
int main(int argc,char *argv[]){
	enableRawMode();
	initEditor();
	if(argc>=2){
		editorOpen(argv[1]);
	}
	while(1){
		clearScreen(1);
		processKeypress();
	}
	disableRawMode();
	return 0;
}
