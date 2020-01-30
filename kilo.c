#define _DEFAULT_SOURCE
//#define _BSD_SOURCE
#define _GNU_SOURCE

#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<termios.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<stdarg.h>
#include<ctype.h>
#include<string.h>

#define ctrl(k) ((k) & 0x1f)
#define str_INIT {NULL,0}
#define version "0.0.1"
#define _TAB_STOP 8
#define KILO_QUIT_TIMES 3

enum editorKey {
	BACKSPACE =127,
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
  int rsize;
  char *chars;
  char *render;
} erow;
typedef struct editorConfig{
	int cx,cy;//cx is posn in the line,without indentation
	int rx;//actual posn in line
	int rowOffset,colOffset;
	int screenRows;
	int screenColumns;
	char *file;
	int numRows;
	int dirty;
	erow *row;
	char status[80];
	time_t statusTime;
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
void search();
char *editorPrompt(char* prompt,void (*callback)(char*,int));
void editorRowAppendString(erow*,char*,size_t);
void editorDelChar();
void editorSetStatusMessage(const char *fmt, ...); 
void moveCursor(int);
void editorInsertChar(int);
int cursorPosition(int*,int*);
void saveToFile();
void die(const char *s)
{
	clearScreen(0);
	perror(s);
	exit(1);
}
void editorUpdateRow(erow *row){
	int tabs=0;
	int j;
	for(j=0;j<row->size;j++)
		if(row->chars[j]=='\t')
			tabs++;
	free(row->render);
	row->render=malloc(row->size+tabs*(_TAB_STOP-1) +1);

	int idx=0;
	for(j=0;j<row->size;j++){
		if(row->chars[j]=='\t'){
			row->render[idx++]=' ';
			while(idx% _TAB_STOP !=0)
				row->render[idx++]=' ';
		}
		else{
			row->render[idx++]=row->chars[j];
		}
	}
	row->render[idx]='\0';
	row->rsize=idx;
}
int editorRowCxToRx(erow *row, int cx) {//TODO
  int rx = 0;
  int j;
  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t')
      rx += (_TAB_STOP - 1) - (rx % _TAB_STOP);
    rx++;
  }
  return rx;
}
int editorRowRxToCx(erow *row, int rx) {
  int cur_rx = 0;
  int cx;
  for (cx = 0; cx < row->size; cx++) {
    if (row->chars[cx] == '\t')
      cur_rx += (_TAB_STOP - 1) - (cur_rx % _TAB_STOP);
    cur_rx++;
    if (cur_rx > rx) return cx;
  }
  return cx;
}
void editorInsertRow(int at,char * line,size_t linelen){
	if(at<0|| at>E.numRows)
		return;
//void editorAppendRow(char *line,size_t linelen){

	E.row=realloc(E.row,sizeof(erow)*(E.numRows+1));
	memmove(&E.row[at+1],&E.row[at],sizeof(erow)*(E.numRows-at));

//	int at=E.numRows;
	E.row[at].size=linelen;
	E.row[at].chars=malloc(linelen+1);
	memcpy(E.row[at].chars, line ,linelen);
	E.row[at].chars[linelen]='\0';

	E.row[at].rsize=0;
	E.row[at].render=NULL;
	editorUpdateRow(&E.row[at]);

	E.numRows++;
	E.dirty++;
}
void editorOpen(char *filename)
{
	free(E.file);
	E.file=strdup(filename);

	FILE *fp=fopen(filename,"r");
	if(!fp)
		die("fopen");
	char *line=NULL;
	size_t linecap=0;
	ssize_t linelen;
	while((linelen = getline(&line, &linecap, fp))!=-1){
		while (linelen > 0 && (line[linelen - 1] == '\n' ||line[linelen - 1] == '\r'))
			linelen--;

		editorInsertRow(E.numRows,line,linelen);
//		editorAppendRow(line,linelen);
	}
	free(line);
	fclose(fp);
	E.dirty=0;
}
void editorInsertNewline(){
	if (E.cx == 0){
		editorInsertRow(E.cy, "", 0);
	}
	else{
		erow *row = &E.row[E.cy];
		editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
		row = &E.row[E.cy];
		row->size = E.cx;
		row->chars[row->size] = '\0';
		editorUpdateRow(row);
	}
	E.cy++;
	E.cx = 0;
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
	E.rx=0;
	E.rowOffset=0;
	E.colOffset=0;
	E.numRows=0;
	E.row=NULL;
	E.file=NULL;
	E.status[0]='\0';
	E.statusTime=0;
	E.dirty=0;

	if(getWindowSize(&E.screenRows,&E.screenColumns)==-1)
		die("Window");
	E.screenRows-=2;
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
	while((nread=read(STDIN_FILENO,&c,1))!=1){
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
				if (read(STDIN_FILENO, &seq[2], 1) != 1)
					return '\x1b';
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
	static int quit_times=KILO_QUIT_TIMES;
	int curr=readKey();
	switch(curr){//add all the mode controls below
		case '\r':
			editorInsertNewline();
//			E.cy++;
//			E.colOffset=E.cx=0;
			break;
		case ctrl('q'):
			if (E.dirty && quit_times > 0) {
				editorSetStatusMessage("WARNING!!! File has unsaved changes. "
						"Press Ctrl-Q %d more times to quit.", quit_times);
				quit_times--;
				return;
			}
			clearScreen(0);
			exit(0);
			break;
		case ctrl('f'):
			search();
			break;
		case HOME_KEY:
			E.cx=0;
			break;
		case END_KEY:
			if(E.cy<E.numRows)
				E.cx=E.row[E.cy].size;
			break;
		case PAGE_UP:
		case PAGE_DOWN:
			{
				if (curr == PAGE_UP) {
					E.cy = E.rowOffset;
				}
				else if(curr == PAGE_DOWN){
					E.cy = E.rowOffset + E.screenRows - 1;
					if (E.cy > E.numRows)
						E.cy = E.numRows;
				}
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
		case BACKSPACE:
		case ctrl('h'):
		case DEL_KEY:
			if(curr== DEL_KEY)
				moveCursor(ARROW_RIGHT);
			editorDelChar();
			break;

		case ctrl('l'):
		case '\x1b':
			break;
		case ctrl('s'):
			saveToFile();
			break;
		default:
			editorInsertChar(curr);
			break;
	}
	quit_times=KILO_QUIT_TIMES;
}
void editorFreeRow(erow* row){
	free(row->render);
	free(row->chars);
}
void delRow(int at){
	if(at<0 || at>= E.numRows)
		return;
	editorFreeRow(&E.row[at]);
	memmove(&E.row[at],&E.row[at+1],sizeof(erow)*(E.numRows-at -1));
	E.numRows--;
	E.dirty++;
}
void editorRowDelChar(erow *row, int at) {
  if (at < 0 || at >= row->size)
	  return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  editorUpdateRow(row);
  E.dirty++;
}
void editorDelChar() {
	if(E.cy == E.numRows) return;
	if(E.cx==0 && E.cy==0) return;
	erow *row = &E.row[E.cy];
	if (E.cx > 0) {
		editorRowDelChar(row, E.cx - 1);
		E.cx--;
	}
	else{
		E.cx=E.row[E.cy-1].size;
		editorRowAppendString(&E.row[E.cy-1],row->chars,row->size);
		delRow(E.cy);
		E.cy--;
	}
}
void editorRowAppendString(erow* row,char*s,size_t len){//append 's' to row
	row->chars=realloc(row->chars,row->size +len +1);
	memcpy(&row->chars[row->size],s,len);
	row->size +=len;
	row->chars[row->size]='\0';
	editorUpdateRow(row);
	E.dirty++;
}
void editorSetStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);//initializes ap
  vsnprintf(E.status, sizeof(E.status), fmt, ap);//prints var arguments
  va_end(ap);//analog of free() for va_list
  E.statusTime = time(NULL);
}
void drawStatusBar(strBuffer* ab)
{
	bufAppend(ab,"\x1b[7m",4);
	char status[80], rstatus[80];
	int len=0;
//	char *file;
//	if(E.file==NULL)
//		file=strdup(&E.file);
		len=snprintf(status,sizeof(status),"%.20s -%d lines %s",E.file ? E.file : "NONE",E.numRows,E.dirty? "(modified)":"");
//	else{
////		*E.file='\0';
//		len=snprintf(status,sizeof(status),"%.20s -%d lines",E.file,E.numRows);//TODO BUG HERE
//	}
	 int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cy + 1, E.numRows);
	if(len> E.screenColumns)
		len=E.screenColumns;
	bufAppend(ab,status,len);
	while(len<E.screenColumns){
		if(E.screenColumns-len==rlen){
			bufAppend(ab,rstatus,rlen);
			break;
		}
		else{
		bufAppend(ab," ",1);
		len++;
		}
	}
	bufAppend(ab,"\x1b[m",3);
	bufAppend(ab, "\r\n",2);
}
void drawMessageBar(strBuffer *ab) {
  bufAppend(ab, "\x1b[K", 3);
  int msglen = strlen(E.status);
  if (msglen > E.screenColumns)
	  msglen = E.screenColumns;
  if (msglen && time(NULL) - E.statusTime < 5)
    bufAppend(ab, E.status, msglen);
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
			int len = E.row[filerow].rsize - E.colOffset;
			if(len<0)
				len=0;
			if (len > E.screenColumns) len = E.screenColumns;
			bufAppend(ab, &E.row[filerow].render[E.colOffset], len);
		}

		bufAppend(ab,"\x1b[K",3);
		//		write(STDOUT_FILENO,"~",1);
//		if(y<E.screenRows-1)
			bufAppend(ab,"\r\n",2);
		//			write(STDOUT_FILENO,"\r\n",3);
	}
}
void editorScroll(){
	E.rx=0;
	if(E.cy<E.numRows){
		E.rx=editorRowCxToRx(&E.row[E.cy],E.cx);
	}
	if(E.cy<E.rowOffset){
		E.rowOffset=E.cy;
	}
	if(E.cy>=E.rowOffset+E.screenRows){
		E.rowOffset=E.cy- E.screenRows+1;
	}
	if(E.rx<E.colOffset){
		E.colOffset=E.rx;
	}
	if(E.rx>=E.colOffset+E.screenColumns){
		E.colOffset=E.rx - E.screenColumns+1;
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
	if(options==1)//default
	{
		drawTildes(&ab);
		drawStatusBar(&ab);
		drawMessageBar(&ab);
	}

	//	bufAppend(&ab,"\x1b[H",3);
	char buf[32];
	//	E.cx=12;
	snprintf(buf,sizeof(buf),"\x1b[%d;%dH",E.cy -E.rowOffset+1,E.rx-E.colOffset+1);
	bufAppend(&ab,buf,strlen(buf));
	bufAppend(&ab,"\x1b[?25h",6);
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
void moveCursor(int key){
	erow *row=(E.cy >= E.numRows) ? NULL : &E.row[E.cy];
	switch (key) {
		case ARROW_LEFT:
			if(E.cx!=0)
				E.cx--;
			else if(E.cy>0){
				E.cy--;
				E.cx=E.row[E.cy].size;
			}
			break;
		case ARROW_RIGHT:
			if(row&& E.cx<row->size)
				E.cx++;
			else if(row && E.cx==row->size){
				E.cy++;
				E.cx=0;
			}
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

	row = (E.cy >= E.numRows) ? NULL : &E.row[E.cy];
	int rowlen = row ? row->size : 0;
	if (E.cx > rowlen) {
//		E.cy++;
		E.cx = rowlen;
	}
}
void editorRowInsertChar(erow* row,int at,int c){//insert a single char 'c' at 'at'th posn in row
	if(at<0||at> row->size)
		at=row->size;
	row->chars=realloc(row->chars,row->size+2);//for the new char and the null byte
	memmove(&row->chars[at+1],&row->chars[at],row->size - at+ 1);
	row->size++;
	row->chars[at]=c;
	editorUpdateRow(row);
	E.dirty++;
}
void editorInsertChar(int c){
	if(E.cy==E.numRows){
		editorInsertRow(E.numRows,"",0);
	}
	editorRowInsertChar(&E.row[E.cy],E.cx,c);
	E.cx++;
}
char *editorRowsToString(int *buflen){

	int total=0,j;
	for(j=0;j<E.numRows;j++)
		total+=E.row[j].size+1;

	*buflen=total;

	char *buf=malloc(total*sizeof(char));
	char *str=buf;

	for(j=0;j<E.numRows;j++)
	{
		memcpy(str,E.row[j].chars,E.row[j].size);
		str+=E.row[j].size;
		*str='\n';
		str++;
	}
	return buf;
}
void saveToFile(){//TODO probably with the automatic cursor movement
	if(E.file==NULL){
		E.file=editorPrompt("Save as: %s(ESC to cancel)",NULL);
		if(E.file==NULL){
			editorSetStatusMessage("Save Aborted");
			return;
		}
	}
	int len;
	char *text=editorRowsToString(&len);
	int fd=open(E.file, O_RDWR|O_CREAT, 0644);
	//O_RDWR: Read and write
	//O_CREAT: creat if it doesn't exist //0644 defines permissions for the new file
	if(fd!=-1){
		if(ftruncate(fd,len)!=-1){
			if(write(fd,text,len)==len){
				close(fd);
				free(text);
				E.dirty=0;
				editorSetStatusMessage("%d bytes written to disk",len);
				return;
			}
		}
		close(fd);
	}

	free(text);
//	editorSetStatusMessage("Can't save! I/O error: %s",strerror(errno));
}
char* editorPrompt(char* prompt, void(*callback)(char*,int)){
	size_t bufsize=128;
	char *buf =malloc(bufsize);
	size_t buflen=0;
	buf[0]='\0';

	while(1){
		editorSetStatusMessage(prompt,buf);
		clearScreen(1);

		int c =readKey();
		if(c==DEL_KEY|| c==ctrl('h')||c== BACKSPACE){
			if(buflen!=0)
				buf[--buflen]='\0';
		}
		else if(c=='\x1b'){
			editorSetStatusMessage("");
			if(callback)
				callback(buf,c);
			free(buf);
			return NULL;
		}
		else if(c=='\r'){
			if(buflen!=0){
				editorSetStatusMessage("");
				if(callback)
					callback(buf,c);
				return buf;
			}
		}
		else if(!iscntrl(c)&& c<128){//<128 ensures it is not a control char
			if(buflen==bufsize-1){
				bufsize*=2;
				buf=realloc(buf,bufsize);
			}
			buf[buflen++]=c;
			buf[buflen]= '\0';
		}

		if(callback)
			callback(buf,c);
	}
}
void searchCallback(char* query,int key){
	static int last_match=-1;
	static int direction=1;
	if(key=='\r'||key=='\x1b'){
		last_match=-1;
		direction=1;
		return;
	}
	else if(key==ARROW_RIGHT || key==ARROW_DOWN){
		direction=1;
	}
	else if(key==ARROW_LEFT || key==ARROW_UP){
		direction=-1;
	}
	else{
		last_match=-1;
		direction=1;
	}

	if(last_match==-1)
		direction=1;
	int current=last_match;
	int i;
	for(i=0;i<E.numRows;i++){
		current+=direction;
		if(current==-1)
			current=E.numRows-1;
		else if(current==E.numRows)
			current=0;
		erow* row=&E.row[current];
		char *match=strstr(row->render,query);
		if(match){
			last_match=current;
			E.cy=current;
			E.cx=editorRowRxToCx(row,match - row->render);
			E.rowOffset=E.numRows;
			break;
		}
	}
}
void search(){
	int a,s,d,f;
	a=E.cx;
	s=E.cy;
	d=E.colOffset;
	f=E.rowOffset;
	char *query=editorPrompt("Search: %s (ESC to cancel)",searchCallback);
	if(query)
		free(query);
	else{
		E.cx=a;
		E.cy=s;
		E.colOffset=d;
		E.rowOffset=f;
	}
}
int main(int argc,char *argv[]){
	enableRawMode();
	initEditor();
	if(argc>=2){
		editorOpen(argv[1]);
	}

	editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl+F= find");

	while(1){
		clearScreen(1);
		processKeypress();
	}
	disableRawMode();
	return 0;
}
