#ifndef _H_INPUT
#define _H_INPUT

#pragma warning(disable:4786) // bug of VC6

#pragma once
#include <windows.h>
#include <algorithm>
#include <vector>
#include "console.h"
#include <cassert>

#define FONT_SIZE_X 8
#define FONT_SIZE_Y 12
// LTRB
static const SMALL_RECT _input_window={0,0,80-1,40-1};
static const SMALL_RECT _input_default_box={10-2,10,70,30-2};
static const SMALL_RECT _input_menu_box={0,40-1-16,16,40-1};
static const SMALL_RECT _input_none={-1,-1,-1,-1};
static const std::vector<SMALL_RECT> _input_none_vbox;
static const COORD _input_default_option={(_input_default_box.Right+_input_default_box.Left)/2-8,_input_default_box.Bottom-2};
static const COORD _input_default_option1={_input_default_box.Left+15,_input_default_box.Bottom-2};
static const COORD _input_default_option2={_input_default_box.Right-15-8,_input_default_box.Bottom-2};
static const WORD _input_default_option_color=BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED;
static const WORD _input_default_color=FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED;
#define _INPUT_DEFAULT_INPUT_NAME "Edit"
#define _INPUT_DEFAULT_INPUT_CNAME "Clear"
static const SMALL_RECT _input_default_input_box={_input_default_box.Left+5\
,_input_default_box.Top+5,_input_default_box.Right-5,_input_default_box.Bottom-5};
static const SMALL_RECT _input_default_input_box_center={_input_default_input_box.Left\
,_input_default_input_box.Top+3,_input_default_input_box.Right,_input_default_input_box.Bottom-3};
#define MAPVK_VK_TO_CHAR 2
#define _INPUT_DRAWOPTION_CLEAR INT_MIN




class _input
{
public:
	_input();
	
	struct _option_jmp
	{
		int ljmp,rjmp,ujmp,djmp;
		void reset(int l=-1,int r=-1,int u=-1,int d=-1) {ljmp=l;rjmp=r;ujmp=u;djmp=d;}
		_option_jmp() {reset();}
	};
	struct _option
	{
		LPCSTR name;
		COORD pos;
		_option_jmp jmp;
	};
	struct _input_box
	{
		char *rstr;
		bool show;
		SMALL_RECT pos;
		LPCSTR defvalue;
		bool defedit;
		int controlby[2];
		//bool multi;
		// ...
	};


	/*
		Draw options inside the box
		std::vector<_option>	-> vector of options
		(int)					-> selected option(negative for the inverse) (DEFAULT:0)
			_INPUT_DRAWOPTION_CLEAR for clear option attributes
		(bool)					-> show extra characters in the both side? (DEFAULT:true)
		(bool)					-> highlight the selected texts (only available when showing) (DEFAULT:true)
	*/
	void DrawOption(std::vector<_option>,int=0,bool=true,bool=true);

	
	/*
		Draw options (combined with ShowTexts())
		(LPCSTR)				-> string of texts
		std::vector<_option>	-> vector of options 
		std::vector<_input_box>	-> vector of input box
		(int)					-> default selected option(negative for the inverse) (DEFAULT:0)
		(SMALL_RECT)			-> position of box (DEFAULT:_input_deafult_box)
		std::vector<SMALL_RECT>	-> additional box to draw (DEFAULT:_input_none_vbox)
		(short)					-> (0)/double(1) the line of the box (binary:00,01,10,11) (DEFAULT:0)
		(bool)					-> animated (DEFAULT:false)
		(bool)					-> clear original rectangle first (DEFAULT:false)
		(bool)					-> remain the rectangle(require the above two to be false and true respectively) (DEFAULT:false)
		(bool)					-> ignore all above options, (un)initialize the box manually (DEFAULT:false)
		return one option selected
	*/
	int ShowOptions(LPCSTR,std::vector<_option>,std::vector<_input_box>&,int=0,SMALL_RECT=_input_default_box,const std::vector<SMALL_RECT>& =_input_none_vbox,short=0,bool=false,bool=false,bool=false,bool=false);


	/*
		Print texts in the box
		(LPCSTR)		-> string to show
		(SMALL_RECT)	-> position of box (DEFAULT:_input_default_box)
	*/
	void ShowTexts(LPCSTR,SMALL_RECT=_input_default_box);


	/*
		Start showing waiting texts
	*/
	void StartShowWaitingTexts();

	/*
		Stop showing waiting texts
	*/
	void StopShowWaitingTexts();


	/*
		Fill the rectangle with space bar
		(SMALL_RECT)	-> rectangle to fill
		(char*)			-> string to fill with
	*/
	void ClearRect(SMALL_RECT,char[]=" ");


	/*
		Draw a box on the console 
		(SMALL_RECT)	-> rectangle box (DEFAULT:_input_default_box)
		(short)			-> double the line (true/false), else fill with space bar (DEFAULT:false)
	*/
	void DrawBox(SMALL_RECT=_input_default_box,short=false);


	/*
		Draw a box on the console animatedly
		(SMALL_RECT)	-> rectangle box (DEFAULT:_input_default_box)
		(bool)			-> double the line (DEFAULT:false)
		(int)			-> delay speed (DEFAULT:3)
		(int)			-> step (DEFAULT:1)
		(bool)			-> false/true zoom in/out (DEFAULT:false)
	*/
	void DrawBoxAnimatedly(SMALL_RECT=_input_default_box,bool=false,int=3,int=1,bool=false);



	/*
		Still blocking waiting for inputing?
		(bool)	-> Suspend input? true/false
		(bool)	-> Stop input? (Only valid when above is set true) (DEFAULT:false)
	*/
	void SetWaitInputFlag(bool,bool=false);
	
	
	/*
		Still blocking waiting for inputing?
		return true/false
	*/
	bool GetWaitInputFlag();
	

	/*
		Set default output handle
		(HANDLE)	-> handle
	*/
	void SetOutputHandle(HANDLE);

	/*
		Get current output handle
		return handle
	*/
	HANDLE GetOutputHandle();

	/*
		Output a string from the specified position
		(LPCSTR)	-> string
		(COORD)		-> beginning position
	*/
	void OutputStrAt(LPCSTR,COORD);


	/*
		Buffer the output into the different screen
	*/
	void StartOutputBuffer();


	/*
		Flush the output buffer created before
			requires StartOutputBuffer() executed before calling it
	*/
	void FlushOutputBuffer();


	/*
		Clear the current output buffer, the same as clrscr()
	*/
	void ClearScreen();



//private:
public:
	_ConsoleHelper chinput;

	HANDLE hWaitingT;
	bool bWaitingT;
	HANDLE hOutput;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	bool bInputFlag,bInputEnd;

	HANDLE hOBuffer,hOutputBak;


	/*
		Initialize options, especially for ljmp,rjmp,ujmp,djmp
		If they're all 0, initialize them by default.
		std::vector<_option> -> opt
	*/
	void InitializeOptions(std::vector<_option>&);


	/*
		Change cursor position
		(COORD)	-> position
	*/
	void ChangePos(COORD);


	/*
		Show/hide cursor
		(bool)	-> show/hide
	*/
	void SetCursorVis(bool);


	/*
		Output a string
		(LPCSTR)	-> string
		(DWORD)		-> output attributes (DEFAULT:0)
	*/
	void OutputStr(LPCSTR,WORD=0);

	
	/*
		Show one-line-text in the position
		(LPCSTR)	-> string to show
		(int)		-> how many lines it's skipped
		(SMALL_RECT)-> position of box (DEFAULT:_input_default_box)
		return offset of how many lines it will skip
			Is able to output empty strings
	*/
	int ShowOneLineText(LPCSTR,int,SMALL_RECT=_input_default_box);


	/*
		A thread showing \|/-
	*/
	static DWORD WINAPI ShowWaitingTexts(LPVOID);




};



static void ltrim(char*s)
{
    char*p;
    p=s;
    while(*p==' '||*p=='\t')
		*p++;
    strcpy(s,p);
}

static void rtrim(char*s)
{
    int i;
    i=strlen(s)-1;
    while((s[i]==' '||s[i]=='\t')&&i>=0)
		i--;
    s[i+1]='\0';
}

static void trim(char*s)
{
    ltrim(s);
    rtrim(s);
}





#endif
