#pragma once
#include "../EasySocket.h"
#include "debug.h"
#include "input.h"
#include "card.h"
#include "../g_card.h"
#include <cassert>

void progend(){getch();exit(0);}
// This will clear the active screen buffer, not the selected one
// clrscr() is now a inner function of _input
/*#ifndef clrscr
void clrscr(){system("color 07 & cls");} // easily...(
#endif*/

void GameControl();

_socket_packet LoopRecv(_socket&,_input&,bool);

bool CommunicateW(_socket_packet,_socket&,_input&,_socket_packet&,bool=true);

DWORD WINAPI OnlineSendMessage(LPVOID);

char roomtitle[_BUFFER_ONELENGTH];

DWORD WINAPI WaitForReadyMessage(LPVOID);

_socket *_p_c_hook_cs;
_input *_p_c_hook_gi;
bool _p_c_hook_bBlock;

DWORD WINAPI RefreshRoomCenter(LPVOID);
int _r_refresh_default;

BOOL WINAPI ConsoleHandler(DWORD);

void RenderFramework(_socket_packet,_input&,_card&);

_socket_packet _rf_lastrender;
int _rf_text_count;
char _rf_text[1048576];
void AddRoomText(_socket_packet);
void RenderRoomText(_input&);

struct _stoc
{
	COORD pos;
	int sec;
	_input*gi;
};
DWORD WINAPI ShowTimeoutCount(LPVOID);
DWORD WINAPI WaitForCDZI(LPVOID);

HANDLE _h_main_thread;

DWORD WINAPI CheckMenuPressed(LPVOID);

_socket_packet_cardinfo _p_wfouti_cards;
struct _stoo
{
	COORD pos;
	_socket_packet_cardinfo cards,prevout;
	_card*cd;
};
DWORD WINAPI WaitForOUTI(LPVOID);

int main(int argc,char**argv)
{
	/* Set console style */
	SetConsoleTitle("CCDDZ --- Initializing");
	HWND hConsole=GetConsoleWindow();
	system("chcp 437"); // easily...(
	DEBUG::normal();
	printf("Setting console code page ... ");
	int ret=SetConsoleOutputCP(437);
	if(ret==0 || !IsValidCodePage(437))
	{
		DEBUG::red("Failure\n");
		DEBUG::red("SetConsoleOutputCP(%d) failed with %d\n",ret,GetLastError());
		DEBUG::red("IsValidCodePage(437)= %d\n",IsValidCodePage(437));
		progend();
	}
	else DEBUG::green("Success\n");
	printf("Checking console style ... ");
	int style=GetWindowLong(hConsole,GWL_STYLE);
	if(!(style&WS_THICKFRAME))
	{
		DEBUG::red("Failure\n");
		DEBUG::red("There's no WS_THICKFRAME in its style.\n");
		DEBUG::red("Is this console default???");
		progend();
	}
	else DEBUG::green("Success\n");
	printf("Setting console type ... ");
	ret=SetWindowLong(hConsole,GWL_STYLE,style&~WS_THICKFRAME);
	if(ret==0)
	{
		DEBUG::red("Failure\n");
		DEBUG::red("SetWindowLong(%d) failed with %d\n",ret,GetLastError());
		progend();
	}
	else DEBUG::green("Success\n");
	printf("Setting console editing mode ... ");
	#define ENABLE_EXTENDED_FLAGS 0x0080
	#define ENABLE_INSERT_MODE 0x0020
	#define ENABLE_QUICK_EDIT_MODE 0x0040
	DWORD dwpremode=0;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),&dwpremode);
	dwpremode&=~ENABLE_INSERT_MODE;dwpremode&=~ENABLE_QUICK_EDIT_MODE;
	ret=SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),dwpremode|ENABLE_EXTENDED_FLAGS);
	if(ret==0)
	{
		DEBUG::red("Failure\n");
		DEBUG::red("SetConsoleMode(%d) failed with %d\n",ret,GetLastError());
		progend();
	}
	else DEBUG::green("Success\n");
	
	/* Set console font size */
	HANDLE hOutput=GetStdHandle(STD_OUTPUT_HANDLE);
	typedef BOOL (WINAPI *PROCSETCONSOLEFONT)(HANDLE, DWORD);
	//typedef BOOL (WINAPI *PROCGETCONSOLEFONTINFO)(HANDLE,BOOL,DWORD,CONSOLE_FONT_INFO*);
	typedef COORD (WINAPI *PROCGETCONSOLEFONTSIZE)(HANDLE,DWORD);
	typedef DWORD (WINAPI *PROCGETNUMBEROFCONSOLEFONTS)();
	HMODULE hKernel32=GetModuleHandle("kernel32.dll");
	PROCSETCONSOLEFONT SetConsoleFont=(PROCSETCONSOLEFONT)GetProcAddress(hKernel32,"SetConsoleFont");
	//PROCGETCONSOLEFONTINFO GetConsoleFontInfo=(PROCGETCONSOLEFONTINFO)GetProcAddress(hKernel32,"GetConsoleFontInfo");
	PROCGETCONSOLEFONTSIZE GetConsoleFontSize=(PROCGETCONSOLEFONTSIZE)GetProcAddress(hKernel32,"GetConsoleFontSize");
	PROCGETNUMBEROFCONSOLEFONTS GetNumberOfConsoleFonts=(PROCGETNUMBEROFCONSOLEFONTS)GetProcAddress(hKernel32,"GetNumberOfConsoleFonts");
	printf("Getting total number of available fonts ... ");
	long nFonts=0;
	if(!GetNumberOfConsoleFonts || (nFonts=GetNumberOfConsoleFonts())<=0)
	{
		DEBUG::red("Failure\n");
		if(nFonts<=0)
		{
			DEBUG::red("(%d)GetNumberOfConsoleFonts(%d) failed with %d\n",GetNumberOfConsoleFonts,nFonts,GetLastError());
			//progend();
		}
		else
			DEBUG::red("We couldn't find GetNumberOfConsoleFonts() in your system !!!\n");
		printf("We couldn't execute GetNumberOfConsoleFonts() correctly in your system...\n");
		printf("You may try ");DEBUG::green("enabling");printf(" the ");DEBUG::highlight("legacy console");printf(" in the console property window.\n");
		printf("'''  Right-click on the application title bar and choose the Properties menu option.\n");
		printf("'''  Choose the first tab, Options.\n");
		printf("'''  Then check the box at the bottom of the page describing Use legacy console.\n");
		printf("'''  Press the OK button to apply.\n");
		printf("'''  Restart this program and you will see it pass all checks.\n");
		printf("Press any key to continue, knowing the program may render abnormally, or close manually.\n");
		getch();
	}
	else DEBUG::green("%d\n",nFonts);
	//CONSOLE_FONT_INFO*cFonts=new CONSOLE_FONT_INFO[nFonts];
	printf("Getting detailed info of each font ... ");
	/*ret=GetConsoleFontInfo(hOutput,FALSE,nFonts,&cFonts[0]);
	if(ret==0)
	{
		DEBUG::red("Failure\n");
		DEBUG::red("GetConsoleFontInfo(%d) failed with %d\n",ret,GetLastError());
		progend();
	}
	else DEBUG::green("Success\n");*/
	DEBUG::red("Unknown\n");
	printf("Searching fonts ... ");
	bool found=false;
	for(int i=0;i<nFonts;i++)
		{
			COORD dsize=GetConsoleFontSize(hOutput,i);
			/* We're going to use font size 8x12 */
			if(dsize.X==FONT_SIZE_X && dsize.Y==FONT_SIZE_Y)
			{
				found=true;
				DEBUG::green("%dx%d (index:%d)\n",dsize.X,dsize.Y,i);
				
				printf("Setting up this font ... ");
				ret=SetConsoleFont(hOutput,i);
				if(ret==0)
				{
					DEBUG::red("Failure\n");
					DEBUG::red("SetConsoleFont(%d) failed with %d\n",ret,GetLastError());
					progend();
				}
				else DEBUG::green("Success\n");
				break;
			}
		}
	if(!found)
	{
		DEBUG::red("Failure\n");
		DEBUG::red("Can't find a suitable font size !\n");
		printf("Press any key to continue, knowing the program may render abnormally, or close manually.\n");
		getch();
		//progend();
	}


	/* Set console size */
	DEBUG::normal();printf("Checking console size ... ");
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	ret=GetConsoleScreenBufferInfo(hOutput,&csbi);
	if(ret==0)
	{
		DEBUG::red("Failure\n");
		DEBUG::red("GetConsoleScreenBufferInfo(%d) failed with %d\n",ret,GetLastError());
		progend();
	}
	else DEBUG::green("Success\n");
	printf("Setting console size ... ");
	ret=SetConsoleWindowInfo(hOutput,TRUE,&_input_window);
	if(ret==0)
	{
		DEBUG::red("Failure\n");
		DEBUG::red("SetConsoleWindowInfo(%d) failed with %d\n",ret,GetLastError());
		progend();
	}
	else DEBUG::green("Success\n");
	printf("Setting console buffer size ... ");
	COORD ccsize={_input_window.Right+1,_input_window.Bottom+1};
	ret=SetConsoleScreenBufferSize(hOutput,ccsize);
	if(ret==0)
	{
		DEBUG::red("Failure\n");
		DEBUG::red("SetConsoleScreenBufferSize(%d) failed with %d\n",ret,GetLastError());
		progend();
	}
	else DEBUG::green("Success\n");

	/* Initialization finished */
	printf("\nPreparing to enter CCDDZ ... ");
	//Sleep(1000);
	DEBUG::green("OK\n");

	printf("\n[DEBUG] Press any key to continue ... ");
#ifdef _DEBUG
	getch();
#endif

	/* Game start!!! */
	system("color 07 & cls");
	/* This is a fake handle ! */
	//_h_main_thread=GetCurrentThread();
	DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&_h_main_thread,
		0,FALSE,DUPLICATE_SAME_ACCESS);
	GameControl();

	/* Program ends */
	//getch();
	return 0;
}

using _input::_option;
using _input::_input_box;

_input_box ib,ni;
std::vector<_input_box>vib,vni;

std::vector<_option>msgopt,licopt,exopt,slopt,usropt,bakopt,romopt,ynopt,pstopt,nullopt,cdzopt,dslopt,rrsopt;

HANDLE hOnlineSend;
char*username;
int userdzs,pindex;
COORD cardlupos,cardrupos,cardmepos;
COORD cardluopos,cardruopos,cardmopos,handcardpos;
COORD countlupos,countrupos,countmepos;

bool ValidateLicense(_input&gi,bool sld=true)
{
	std::vector<_option>lco=licopt;
	if(!sld)
	{
		lco.pop_back();
		lco[0].name="Acknowledged";
		lco[0].pos.X+=10;
	}
	SMALL_RECT rect={_input_window.Left+4,_input_window.Top+9,_input_window.Right-4,_input_window.Bottom-9};
	int ret=gi.ShowOptions("TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION\n\n"
		"Copyright CCDDZ mnihyc\n\n"
		"Licensed under the Apache License, Version 2.0 (the \"License\");\n"
		"you may not use this file except in compliance with the License.\n"
		"You may obtain a copy of the License at\n"
		"\nhttp://www.apache.org/licenses/LICENSE-2.0\n\n"
		"Unless required by applicable law or agreed to in writing, software\n"
		"distributed under the License is distributed on an \"AS IS\" BASIS,\n"
		"WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,\n"
		"either express or implied.\n"
		"See the License for the specific language governing permissions and\n"
		"limitations under the License."
		,lco,vib,sld,rect,_input_none_vbox,0,true,false);
	return (ret==0);
}

void ShowUsage(_input&gi)
{
	std::vector<_option>nmsgopt=msgopt;
	nmsgopt[0].pos.Y+=10;
	SMALL_RECT rect={_input_window.Left+2,_input_window.Top+2,_input_window.Right-2,_input_window.Bottom-2};
	int ret=gi.ShowOptions("\nUsage of CCDDZ\n\n\n"
		"Basic Controls\n"
		"UP/DOWN/LEFT/RIGHT ---> Change options\n"
		"ENTER/SPACEBAR ---> Select an option\n\n"
		"Input Controls\n"
		"All the visible ASCII chars are acceptable\n"
		"Backspace/Insert mode is supported\n"
		"ESCAPE ---> End the input\n"
		"ENTER ---> End the input and jump to the options below.\n"
		"UP/DOWN/LEFT/RIGHT ---> Change the position of the cursor\n"
		"Empty chars are replaced with spacebar\n\n"
		"Play Controls\n"
		"UP ---> Select an card\n"
		"DOWN ---> Deselect an card\n"
		"SPACEBAR ---> Deselect all the cards\n"
		"ENTER ---> To select whether PLAY (the selected card(s))\n or SKIP (this round)\n"
		"ENTER then DOWN ---> Back to the card selection mode\n"
		"TAB (Your Text) then ENTER ---> Send internal room text\n\n"
		"Other Controls\n"
		"CTRL+C ---> Exit the program safely\n"
		"ALT+M ---> Show the menu\n"
		"You can't close the client when playing in a room\n\n"
		"......"
		,nmsgopt,vib,0,rect,_input_none_vbox,0,true,false);
}


void GameControl()
{
	char sbuf[1024]={0};
	username=NULL;
	_input gi;
	/* Special need */
	_p_c_hook_gi=&gi;

	/* Show usage() */
	ib.show=false;
	vib.push_back(ib);
	vni.push_back(ni);
	msgopt.clear();licopt.clear();exopt.clear();slopt.clear();usropt.clear();bakopt.clear();ynopt.clear();pstopt.clear();nullopt.clear();cdzopt.clear();
	_option nopt;
	nopt.name="OK && Continue";
	nopt.pos=_input_default_option;
	nopt.pos.X++;
	msgopt.push_back(nopt);
	nopt.pos=_input_default_option1;
	nopt.pos.X-=8;
	slopt.push_back(nopt);
	nopt.name=_INPUT_DEFAULT_INPUT_NAME;
	nopt.pos=_input_default_option2;
	nopt.pos.X-=7;
	slopt.push_back(nopt);
	nopt.name=_INPUT_DEFAULT_INPUT_CNAME;
	nopt.pos=_input_default_option2;
	nopt.pos.X+=12;
	slopt.push_back(nopt);
	nopt.name="Continue to exit";
	nopt.pos=_input_default_option;
	exopt.push_back(nopt);
	nopt.name="Return to previous page";
	nopt.pos.X-=4;
	bakopt.push_back(nopt);
	nopt.name="I agree";
	nopt.pos=_input_default_option1;
	licopt.push_back(nopt);
	nopt.name="Yes, continue";
	ynopt.push_back(nopt);
	nopt.name="I don't agree";
	nopt.pos=_input_default_option2;
	licopt.push_back(nopt);
	nopt.name="No, return";
	ynopt.push_back(nopt);
	nopt.name="Sign up";
	nopt.pos=_input_default_option1;
	usropt.push_back(nopt);
	nopt.name="Log in";
	nopt.pos=_input_default_option2;
	usropt.push_back(nopt);
	SMALL_RECT nbox=_input_default_box;
	nbox.Top-=2;nbox.Bottom+=2;nbox.Left--;
	licopt[0].pos.Y+=2;licopt[1].pos.Y+=2;
	nopt.name="Not call";
	nopt.pos.X=_input_window.Right/2+2;
	nopt.pos.Y=_input_window.Bottom-11;
	nopt.pos.X-=14;
	cdzopt.push_back(nopt);
	nopt.name="Call DZ";
	nopt.pos.X+=14;
	cdzopt.push_back(nopt);
	nopt.name="Continue";
	nopt.pos.X-=7;
	rrsopt.push_back(nopt);
	if(!ValidateLicense(gi))
	{
		gi.ShowOptions("\n\n\n\n\n\nCan't continue because you disagree our license...\n\n\n"
			"The program is going to exit...\n",exopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		return;
	}

	/* Install console hook */
	_p_c_hook_bBlock=false;
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE);

	/* Create a thread to establish the checking of menu button pressed */
	HANDLE hCheckMenuP=CreateThread(NULL,0,CheckMenuPressed,&gi,0,NULL);

	nbox=_input_default_box;
	//nbox.Top-=2;nbox.Bottom+=2;
	/*gi.ShowOptions(
		"Game control\n\n"
		"Dialog\n"
		"Select an item: {ENTER}/{SPACE}\n"
		"Change an item: {LEFT}/{RIGHT}\n\n"
		"Play\n"
		"Change a card: {LEFT}/{RIGHT}\n"
		"Select a card: {UP}\n"
		"Ensure your card: {ENTER}/{SPACE}\n"
		"Unselect this card if you've selected it: {DOWN}\n"
		"Unselect all the cards you've selected if you didn't select this\n"
		"Open the menu / Finish the input : {ESC}\n\n"
		,msgopt,ib,0,nbox,true,true);*/
	nbox=_input_default_box;
	//nbox.Top-=2;nbox.Bottom+=2;
	/*gi.ShowOptions(
		"Detailed information\n\n"
		"Input\n"
		"We only accept characters through MapVirtualKey() from      ASCII 32 to 126\n"
		"Backspace is NOT supported, use spacebar to replace/undo    your input\n"
		"Use {ESC} to finish your input\n"
		"You can edit your input again by selecting 'Edit'\n"
		"You can clear your input by selecting 'Clear'\n"
		"There're no \\r or \\n characters in the input!\n"
		"The final result will be trimmed by trim()\n"
		"Use {ENTER} to end the input and then select an item directly\n"
		,msgopt,ib,0,nbox,true,true);*/
	/*gi.ShowOptions(
		"\nIMPORANT NOTICE !!!\n\n\n\n"
		"If there's nothing on the console, please RESTART the        program !!!\n\n"
		"That's an unknown BUG !!!\n\n\nYeah, but this'll NOT happen if you press MORE SLOWLY...\n\n"
		,msgopt,ib,0,_input_default_box,true,true);*/
	ShowUsage(gi);

	/* Input server address */
	_socket cs;
	/* Special need */
	_p_c_hook_cs=&cs;
	int sret=cs.global_init();
	if(sret!=0)
	{
		sprintf(sbuf,"\n\n\n\n\n\nWSAStartup(%d) failed with error code %d\n\n\n"
			"The program is going to exit...\n",WSAGetLastError(),sret);
		gi.ShowOptions(sbuf,exopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		return;
	}
	hOnlineSend=NULL;
inputserveraddress:
	if(hOnlineSend!=NULL)
	{
		TerminateThread(hOnlineSend,0);
		hOnlineSend=NULL;
	}

	std::vector<SMALL_RECT>adbox;adbox.clear();
	dslopt.clear();
	dslopt.push_back(slopt[1]);dslopt.push_back(slopt[2]);
	dslopt.push_back(slopt[1]);dslopt.push_back(slopt[2]);
	dslopt.push_back(slopt[0]);
	dslopt[0].pos.Y-=8;dslopt[1].pos.Y-=8;
	dslopt[0].pos.X=_input_default_option1.X+2;dslopt[1].pos.X=_input_default_option2.X+2;
	dslopt[2].pos.Y+=2;dslopt[3].pos.Y+=2;
	dslopt[2].pos.X=_input_default_option1.X+2;dslopt[3].pos.X=_input_default_option2.X+2;
	dslopt[4].pos.X=_input_default_option.X+1;dslopt[4].pos.Y+=6;
	dslopt[0].jmp.reset(-1,1,-1,2);
	dslopt[1].jmp.reset(0,-1,-1,3);
	dslopt[2].jmp.reset(-1,3,0,4);
	dslopt[3].jmp.reset(2,-1,1,4);
	dslopt[4].jmp.reset(-1,-1,2,-1);

	SMALL_RECT tmprect=_input_default_box;
	tmprect.Top-=6;tmprect.Bottom+=6;
	SMALL_RECT tmprect0={tmprect.Left+1,tmprect.Top+6,tmprect.Right-1,tmprect.Top+6};
	adbox.push_back(tmprect0);
	SMALL_RECT tmprect1={tmprect.Left+1,dslopt[0].pos.Y+2,tmprect.Right-1,dslopt[0].pos.Y+2};
	adbox.push_back(tmprect1);
	SMALL_RECT tmprect2={tmprect.Left+1,dslopt[2].pos.Y+2,tmprect.Right-1,dslopt[2].pos.Y+2};
	adbox.push_back(tmprect2);

	_input_box ni0,ni1;
	ni0.rstr=NULL;
	ni0.show=true;ni0.defedit=false;
	ni0.defvalue="127.0.0.1";
	ni0.pos=_input_default_input_box_center;
	ni0.pos.Top-=4;ni0.pos.Bottom-=4;
	ni0.controlby[0]=0;ni0.controlby[1]=1;
	vni[0]=ni0;

	ni1.rstr=NULL;
	ni1.show=true;ni1.defedit=false;
	ni1.defvalue="16137";
	ni1.pos=_input_default_input_box_center;
	ni1.pos.Top+=6;ni1.pos.Bottom+=6;
	ni1.pos.Left+=22;ni1.pos.Right-=23;
	ni1.controlby[0]=2;ni1.controlby[1]=3;
	vni.push_back(ni1);

	gi.ShowOptions("\n\nInput the server address and port:\n\n"
		"Leave default if you don't know exactly about that.\n\n\n\n"
		"Server address:\n\n\n\n\n\n\n\n\n\nServer port:",dslopt,vni,0,tmprect,adbox,0,true,true);

	ni0=vni[0];
	ni1=vni[1];vni.pop_back();

	int port=atoi(ni1.rstr);
	if(port<=0 || port>65535)
	{
		gi.ShowOptions("\n\n\n\n\n\n\n"
			"The port range isn't correct...",bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto inputserveraddress;
	}

	gi.ShowTexts("Resolving .......",_input_default_input_box_center);
	gi.StartShowWaitingTexts();
	in_addr** ips=cs.getipbyhost(ni0.rstr);
	gi.StopShowWaitingTexts();
	if(!ips)
	{
		gi.ShowOptions("\n\n\n\n\n\n\n"
			"Couldn't resolve hostname to IP address...",bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto inputserveraddress;
	}

	std::vector<_option>tmpopt;tmpopt.clear();
	for(int i=0;ips[i];i++)
	{
		nopt.jmp.reset();
		char*buf=new char[32];
		sprintf(buf,"%s:%d",inet_ntoa(*ips[i]),port);
		nopt.name=buf;
		tmpopt.push_back(nopt);
	}
	nopt.name="Return to previous page";
	tmpopt.push_back(nopt);
	tmprect=_input_default_box;
	tmprect.Top-=8;tmprect.Bottom+=8;
	for(int i=0;i<tmpopt.size();i++)
	{
		tmpopt[i].pos.X=(tmprect.Left+tmprect.Right-strlen(tmpopt[i].name))/2;
		tmpopt[i].pos.Y=tmprect.Top+5+(tmprect.Bottom-tmprect.Top-5)/tmpopt.size()*(i+1);
	}
	tmpopt[tmpopt.size()-1].pos.Y=tmprect.Bottom-2;
	tmpopt[tmpopt.size()-1].pos.X=_input_default_option.X-4;
	sret=gi.ShowOptions("\n\nSelect an server address to connect:"
		,tmpopt,vib,0,tmprect,_input_none_vbox,0,true,true);
	if(sret==tmpopt.size()-1)
		goto inputserveraddress;

	/* Get the version of server */
	_socket_packet msg;
	msg.type=_PACKET_MESSAGE_SERVER_VER;
	msg.a=_PACKET_THIS_VERSION;
	SMALL_RECT rc=_input_default_input_box_center;
	rc.Left-=10;rc.Right+=10;
	rc.Top-=2;
	gi.ShowTexts("\n\nCalling socket() and connect() .......",rc);
	gi.StartShowWaitingTexts();
	cs.BindPort(0,true);
	if(!cs.ConnectPort(ips[sret]->s_addr,port))
	{
		gi.StopShowWaitingTexts();
		sprintf(sbuf,"\n\n\n\n\nsocket() failed with error code %d\n\n\nor connect() timed out",WSAGetLastError());
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto inputserveraddress;
	}
	gi.StopShowWaitingTexts();
	gi.ClearScreen();
	_socket_packet sp;
	if(!CommunicateW(msg,cs,gi,sp))
		goto inputserveraddress;
	if(sp.type != _PACKET_MESSAGE_SERVER_VER)
	{
		sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
			"packet.type=0x%x but 0x%x expected\n\n",sp.type,_PACKET_MESSAGE_SERVER_VER);
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto inputserveraddress;
	}
	if(sp.a != _PACKET_THIS_VERSION)
	{
		sprintf(sbuf,"\n\n\n\n\n\n\nThe version doesn't match....\n\n\n"
			"The server has a version of 0x%x,\nbut the client has a version of 0x%x",sp.a,_PACKET_THIS_VERSION);
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto inputserveraddress;
	}
	/*sprintf(sbuf,"\n\n\nThe version of the server: 0x%x\n\n\n"
		"The server says:\n\n\n%s",sp.a,sp.data.b);
	gi.ShowOptions(sbuf,msgopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);*/
	
	/* Sign up or log in */
	SetConsoleTitle("CCDDZ --- Logging in");
useroption:
	sret=gi.ShowOptions("\n\n\n\nSign up a new account\n\n\n"
		"or log in your account",usropt,vib,1,_input_default_box,_input_none_vbox,0,true,true);

	tmprect=_input_default_box;
	tmprect.Top-=8;tmprect.Bottom+=8;

	tmpopt=dslopt;
	nopt.jmp.reset(4,-1,2,-1);tmpopt[tmpopt.size()-1].jmp.rjmp=5;
	nopt.name="Back to previous page";
	nopt.pos=tmpopt[tmpopt.size()-1].pos;
	nopt.pos.X+=8;tmpopt[tmpopt.size()-1].pos.X-=14;
	nopt.pos.Y=tmpopt[tmpopt.size()-1].pos.Y=tmprect.Bottom-2;
	tmpopt.push_back(nopt);
	tmpopt[0].pos.Y-=7,tmpopt[1].pos.Y-=7;
	tmpopt[2].pos.Y+=1;tmpopt[3].pos.Y+=1;

	adbox.clear();
	SMALL_RECT tmprect3={tmprect.Left+1,tmpopt[0].pos.Y+3,tmprect.Right-1,tmpopt[0].pos.Y+3};
	adbox.push_back(tmprect3);
	SMALL_RECT tmprect4={tmprect.Left+1,tmpopt[2].pos.Y+3,tmprect.Right-1,tmpopt[2].pos.Y+3};
	adbox.push_back(tmprect4);

	ni0.rstr=NULL;
	ni0.show=true;ni0.defedit=true;
	ni0.defvalue=NULL;
	ni0.pos=_input_default_input_box_center;
	ni0.pos.Left+=15;ni0.pos.Right-=15;
	ni0.pos.Top-=12;ni0.pos.Bottom-=12;
	ni0.controlby[0]=0;ni0.controlby[1]=1;
	vni[0]=ni0;

	ni1.rstr=NULL;
	ni1.show=true;ni1.defedit=false;
	ni1.defvalue=NULL;
	ni1.pos=_input_default_input_box;
	ni1.pos.Top+=3;ni1.pos.Bottom+=3;
	ni1.controlby[0]=2;ni1.controlby[1]=3;
	vni.push_back(ni1);
	int ssret=gi.ShowOptions("\n\nInput your username:\n\n\n\n\n\n\n\n\n\n\n\nInput your password:"
		,tmpopt,vni,0,tmprect,adbox,0,true,true);
	ni0=vni[0];
	ni1=vni[1];vni.pop_back();
	
	if(ssret==tmpopt.size()-1)
		goto useroption;

	char*tusername=ni0.rstr;
	if(strlen(tusername)==0)
	{
		delete[]tusername;
		sprintf(sbuf,"\n\n\n\n\n\n\nThe username can't be empty...");
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto useroption;
	}
	char*password=ni1.rstr;
	memset(&sp,0,sizeof(sp));
	if(sret==1)
		sp.type=_PACKET_USER_LOGIN;
	else
		sp.type=_PACKET_USER_REGISTER;
	strcpy(sp.data.userinfo.name,tusername);
	strcpy(sp.data.userinfo.pass,password);
	if(!CommunicateW(sp,cs,gi,sp))
		goto inputserveraddress;
	if(sp.type != _PACKET_USER_LOGIN && sp.type != _PACKET_USER_REGISTER)
	{
		sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
			"packet.type=0x%x but 0x%x or 0x%x expected\n\n",sp.type,_PACKET_USER_LOGIN,_PACKET_USER_REGISTER);
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto inputserveraddress;
	}
	if(sp.a==_PACKET_MESSAGE_SUCCESS)
	{
		userdzs=sp.aa;
		sprintf(sbuf,"\n\n\n\n\nThe command has completed successfully.\n\n\n%s",sp.data.b);
		gi.ShowOptions(sbuf,msgopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		if(sret==0)
			goto useroption;
	}
	else
	{
		sprintf(sbuf,"\n\n\n\n\nThe command executed unsuccessfully.\n\n\n%s",sp.data.b);
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto useroption;
	}
	username=tusername;

	/* Game rooms */
	hOnlineSend=CreateThread(NULL,0,OnlineSendMessage,(LPVOID*)&cs,0,NULL);
selectgameroom:
	gi.ClearScreen();
	SetConsoleTitle("CCDDZ --- Game center");
	_r_refresh_default=0;
	bool refresh_background=false;
refreshcenter:
	_socket_packet rs;
	rs.type=_PACKET_ALLROOM_INFO;
	if(!CommunicateW(rs,cs,gi,sp,!refresh_background))
		goto inputserveraddress;
	gi.ClearScreen();
	if(sp.type != _PACKET_ALLROOM_INFO)
	{
		sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
			"packet.type=0x%x but 0x%x expected\n\n",sp.type,_PACKET_ALLROOM_INFO);
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto selectgameroom;
	}
	/* Memory leak but I don't care about that..... */
	romopt.clear();
	nopt.name="Refresh room list";
	nopt.pos.X=_input_window.Left+2;
	nopt.pos.Y=_input_window.Top+2;
	nopt.jmp.reset();
	romopt.push_back(nopt);
	_socket_packet_allroominfo ari=sp.data.allroominfo;
	for(int i=0;i<sp.data.allroominfo.num;i++)
	{
		char *tb=new char[_BUFFER_ONELENGTH+1];
		nopt.name=tb;
		memset(tb,0,sizeof(tb));		
		strcpy(tb,sp.data.allroominfo.name[i]);
		int yp=(_input_window.Bottom-_input_window.Top-25)/3;
		nopt.pos.Y=_input_window.Top+4+yp*i;
		nopt.pos.X=(_input_window.Right-_input_window.Left-strlen(tb))/2+_input_window.Left+1;
		romopt.push_back(nopt);
		nopt.pos.Y+=2;
		nopt.pos.X=_input_window.Right-12-2;
		char *tt=new char[16];
		nopt.name=tt;
		memset(tt,0,sizeof(tt));
		strcpy(tt,"Players: ");
		itoa(sp.data.allroominfo.playing[i],&tt[strlen(tt)],10);
		strcat(tt,"/");
		itoa(sp.data.allroominfo.maxplayer[i],&tt[strlen(tt)],10);
		romopt.push_back(nopt);
	}
	nopt.name="Create a room";
	nopt.pos.X=_input_window.Right-2-strlen(nopt.name);
	nopt.pos.Y=_input_window.Bottom-2;
	romopt.push_back(nopt);
	for(int i=1;i<romopt.size()-1;i++)
	{
		romopt[i].jmp.ljmp=(i && i%2==0 ? i-1 : -1);
		romopt[i].jmp.rjmp=(i+1!=romopt.size() && i%2!=0 ? i+1 : -1);
		romopt[i].jmp.ujmp=(i-2>=0 ? i-2 : (i==1 ? 0 : -1));
		romopt[i].jmp.djmp=(i+2<romopt.size() ? i+2 : (i==romopt.size()-2 ? romopt.size()-1 : -1));
	}
	romopt[0].jmp.reset(-1,1,-1,1);
	if(romopt.size()-1 != 1)
		romopt[1].jmp.ljmp=0;
	romopt[romopt.size()-1].jmp.reset((romopt.size()==2 ? 0 : -1),-1,(romopt.size()>=3 ? romopt.size()-3 : romopt.size()-2),-1);

	if(_r_refresh_default>=romopt.size())
		_r_refresh_default=0;
	/* Use multi-thread in order to support refreshing the list periodically */
	HANDLE hRefreshRC=CreateThread(NULL,0,RefreshRoomCenter,(LPVOID)&gi,0,NULL);
	//sret=gi.ShowOptions("Join/Create a room\n",romopt,ib,0,_input_window,false,true);
	DWORD dRet;
	while(GetExitCodeThread(hRefreshRC,&dRet) && dRet==STILL_ACTIVE)
	{
		DWORD lasttime=GetTickCount();
		while(GetExitCodeThread(hRefreshRC,&dRet) && dRet==STILL_ACTIVE && GetTickCount()-lasttime<=5000)
			Sleep(100);
		if(dRet==STILL_ACTIVE)
		{
			gi.SetWaitInputFlag(true,true);
			WaitForSingleObject(hRefreshRC,INFINITE);
			gi.SetWaitInputFlag(false);
			GetExitCodeThread(hRefreshRC,&dRet);
			CloseHandle(hRefreshRC);
			_r_refresh_default=dRet;
			if(dRet==romopt.size()-1)
				dRet=-1;
			refresh_background=true;
			goto refreshcenter;
		}
	}
	CloseHandle(hRefreshRC);
	sret=dRet;

	/* Refresh the list */
	if(sret==0)
		goto selectgameroom;
	memset(&sp,0,sizeof(sp));
	int roomindex=-1,roompos=-1;
	bool needjoin=true;

	gi.ClearScreen();
	/* Create a room */
	if(sret==romopt.size()-1)
	{
		tmprect=_input_default_box;
		tmprect.Bottom++;
		adbox.clear();

		tmpopt.clear();
		tmpopt.push_back(dslopt[0]);tmpopt.push_back(dslopt[1]);
		tmpopt[0].pos.Y+=1;tmpopt[1].pos.Y+=1;
		tmpopt[0].jmp.reset(-1,1,-1,3);tmpopt[1].jmp.reset(0,-1,-1,3);
		SMALL_RECT tmprect5={tmprect.Left+1,tmpopt[0].pos.Y+2,tmprect.Right-1,tmpopt[0].pos.Y+2};
		adbox.push_back(tmprect5);
		tmpopt.push_back(slopt[0]);tmpopt.push_back(slopt[1]);tmpopt.push_back(slopt[2]);
		tmpopt[2].name="1 Player";
		tmpopt[2].pos.Y-=3;tmpopt[2].jmp.reset(-1,3,0,5);
		tmpopt[3].name="3 Players";
		tmpopt[3].pos.Y-=3;tmpopt[3].pos.X-=4;tmpopt[3].jmp.reset(2,4,0,5);
		tmpopt[4].name="2 Players";
		tmpopt[4].pos.Y-=3;tmpopt[4].pos.X-=2;tmpopt[4].jmp.reset(3,-1,0,5);
		SMALL_RECT tmprect6={tmprect.Left+1,tmpopt[3].pos.Y+2,tmprect.Right-1,tmpopt[3].pos.Y+2};
		adbox.push_back(tmprect6);
		tmpopt.push_back(bakopt[0]);
		tmpopt[5].pos.Y=tmprect.Bottom-2;
		tmpopt[5].jmp.reset(-1,-1,3,-1);

		ni.rstr=NULL;
		ni.show=true;ni.defedit=true;
		ni.defvalue=NULL;
		ni.pos=_input_default_input_box_center;
		ni.pos.Bottom++;
		ni.pos.Top-=4;ni.pos.Bottom-=4;
		ni.pos.Left+=10;ni.pos.Right-=10;
		ni.controlby[0]=0;ni.controlby[1]=1;
		vni[0]=ni;
		ssret=gi.ShowOptions("\nInput the name of the room:",tmpopt,vni,0,tmprect,adbox,0,true,true);
		ni=vni[0];
		if(ssret==tmpopt.size()-1)
			goto selectgameroom;

		char*roomname=ni.rstr;ni.rstr=NULL;
		if(strlen(roomname)==0)
		{
			delete[]roomname;
			sprintf(sbuf,"\n\n\n\n\n\n\nThe name of the room can't be empty...");
			gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			goto selectgameroom;
		}
		sp.type=_PACKET_ROOM_CREATE;
		sp.a=(ssret==3 ? 3 : (ssret==2 ? 1 : 2));
		strcpy(sp.data.roominfo.players[0],username);
		strcpy(sp.data.roominfo.name,roomname);
		if(!CommunicateW(sp,cs,gi,sp))
			goto inputserveraddress;
		if(sp.type != _PACKET_ROOM_CREATE)
		{
			sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
				"packet.type=0x%x but 0x%x expected\n\n",sp.type,_PACKET_ROOM_CREATE);
			gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			goto selectgameroom;
		}
		if(sp.a==_PACKET_MESSAGE_SUCCESS)
		{
			sprintf(sbuf,"\n\n\n\n\nThe command has completed successfully.\n\n\n%s",sp.data.b);
			gi.ShowOptions(sbuf,msgopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			roomindex=sp.aa;
			needjoin=false;
		}
		else
		{
			sprintf(sbuf,"\n\n\n\n\nThe command executed unsuccessfully.\n\n\n%s",sp.data.b);
			gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			goto selectgameroom;
		}
	}
	roompos=(sret-1)/2;
	if(roomindex==-1)
		roomindex=ari.index[(sret-1)/2];
	memset(&sp,0,sizeof(sp));

	/* View detailed room info */
	if(sret%2==0)
	{
		sp.type=_PACKET_ROOM_INFO;
		sp.a=roomindex;
		if(!CommunicateW(sp,cs,gi,sp))
			goto inputserveraddress;
		if(sp.type != _PACKET_ROOM_INFO)
		{
			sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
				"packet.type=0x%x but 0x%x expected\n\n",sp.type,_PACKET_ROOM_INFO);
			gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			goto selectgameroom;
		}
		sprintf(sbuf,"\n\n\n\nRoom %d name:\n%s\n\n\n\nPlayers %d/%d:\n"
			/*"%s %d\n%s %d\n%s %d\n"*/,roomindex,sp.data.roominfo.name,sp.data.roominfo.playing,sp.data.roominfo.maxplayer);
		for(int i=0;i<3;i++)
		{
			if(strlen(sp.data.roominfo.players[i])!=0)
			{
				char ssbuf[_BUFFER_ONELENGTH]={0};
				sprintf(ssbuf,"%s %d",sp.data.roominfo.players[i],sp.data.roominfo.dzs[i]);
				strcat(sbuf,ssbuf);
			}
			strcat(sbuf,"\n");
		}
		SMALL_RECT sbr=_input_default_box;
		sbr.Left=_input_window.Left+5;sbr.Right=_input_window.Right-5;
		gi.ShowOptions(sbuf,bakopt,vib,0,sbr,_input_none_vbox,0,true,true);
		goto selectgameroom;
	}
	
	/* Join room [roomindex] */
	if(needjoin)
	{
		if(ari.playing[roompos]>=ari.maxplayer[roompos])
		{
			sprintf(sbuf,"\n\n\n\n\n\n\nThis room is currently full!");
			gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			goto selectgameroom;
		}
		sp.type=_PACKET_ROOM_JOIN;
		sp.a=roomindex;
		strcpy(sp.data.b,username);
		if(!CommunicateW(sp,cs,gi,sp))
			goto inputserveraddress;
		if(sp.type != _PACKET_ROOM_JOIN)
		{
			sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
				"packet.type=0x%x but 0x%x expected\n\n",sp.type,_PACKET_ROOM_JOIN);
			gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			goto selectgameroom;
		}
		if(sp.a==_PACKET_MESSAGE_FAILURE)
		{
			sprintf(sbuf,"\n\n\n\n\nThe command executed unsuccessfully.\n\n\n%s",sp.data.b);
			gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
			goto selectgameroom;
		}
	}
	memset(&sp,0,sizeof(sp));
	sp.type=_PACKET_ROOM_INFO;
	sp.a=roomindex;
	if(!CommunicateW(sp,cs,gi,sp))
		goto inputserveraddress;
	if(sp.type != _PACKET_ROOM_INFO)
	{
		sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
			"packet.type=0x%x but 0x%x expected\n\n",sp.type,_PACKET_ROOM_INFO);
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,true);
		goto selectgameroom;
	}
	_socket_packet bp=sp;
	for(pindex=0;pindex<3;pindex++)
		if(strcmp(bp.data.roominfo.players[pindex],username)==0)
			break;
	assert(pindex<3);


	/* Clear room text when leaving */
	memset(_rf_text,0,sizeof(_rf_text));

	/* Enter a room */
roomstart:
	gi.ClearScreen();
	SetConsoleTitle("CCDDZ --- Playing");
	HANDLE hInput;
	_card cd(&gi);
	bool imr=false;
	cd.RenderPlayerFramework(bp.data.roominfo,pindex,true);
	RenderRoomText(gi);
	sprintf(roomtitle,"Room %d",roomindex);

	pstopt.clear();
	nopt.name="Ready to play now";
	nopt.pos.Y=_input_window.Bottom-12;
	nopt.pos.X=_input_window.Left+6;
	pstopt.push_back(nopt);
	nopt.name="Exit the room";
	nopt.pos.Y=_input_window.Bottom-12;
	nopt.pos.X=_input_window.Right-6-strlen(nopt.name);
	pstopt.push_back(nopt);
	hInput=CreateThread(NULL,0,WaitForReadyMessage,(LPVOID)&gi,0,NULL);
	/* Don't stop receiving messages */
	if(cs.bStopRecv)
		cs.StartRecvMsg();
	while(1)
	{
		Sleep(100);
		memset(&sp,0,sizeof(sp));
		DWORD dex;
		if(hInput && !GetExitCodeThread(hInput,&dex))
			assert(0);
		if(hInput && dex!=STILL_ACTIVE)
		{
			if(dex==0) // Ready/start
			{
				imr=(!imr);
				if(imr)
					sp.type=_PACKET_ROOM_PLAYER_READY;
				else
					sp.type=_PACKET_ROOM_PLAYER_DREADY;
				int k;
				for(k=0;k<3;k++)
					if(strcmp(bp.data.roominfo.players[k],username)==0)
						break;
				assert(k<3);
				sp.a=roomindex;sp.aa=k;
				if(!CommunicateW(sp,cs,gi,sp,false))
					goto inputserveraddress;
				if(sp.type != _PACKET_ROOM_INFO)
				{
					sprintf(sbuf,"\n\n\n\n\n\n\nReceived unknown packet from the server...\n\n\n"
						"packet.type=0x%x but 0x%x expected\n\n",sp.type,_PACKET_ROOM_INFO);
					gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true);
					goto selectgameroom;
				}
				bp=sp;
			}
			else if(dex==1)
			{
				sp.type=_PACKET_ROOM_LEAVE;
				int k;
				for(k=0;k<3;k++)
					if(strcmp(bp.data.roominfo.players[k],username)==0)
						break;
				assert(k<3);
				sp.a=roomindex;sp.aa=k;
				strcpy(sp.data.b,username);
				if(bp.data.roominfo.playing==1)
				{
					sprintf(sbuf,"\n\n\n\n\nYou're the only player in this room, \n"
						"once you exit, the room will be deleted automatically.\n\n"
						"Are you sure to continue?");
					sret=gi.ShowOptions(sbuf,ynopt,vib,1,_input_default_box,_input_none_vbox,0,true);
					if(sret==0)
					{
						cs.SendMsg(sp);
						goto selectgameroom;
					}
				}
				else
				{
					cs.SendMsg(sp);
					goto selectgameroom;
				}
			}
			cd.RenderPlayerFramework(bp.data.roominfo,pindex,true);
			if(imr)
				pstopt[0].name="Still unready to play";
			else
				pstopt[0].name="Ready to play now";
			/* Room should have began, so don't establish this */
			if(bp.data.roominfo.ready[0] && bp.data.roominfo.ready[1] && bp.data.roominfo.ready[2])
				hInput=NULL;
			else
				hInput=CreateThread(NULL,0,WaitForReadyMessage,(LPVOID)&gi,0,NULL);
		}
		sp=cs.RecvMsg();
		if(sp.type==0)
			continue;
		if(sp.type==_PACKET_ROOM_INFO)
		{
			bp=sp;
			for(pindex=0;pindex<3;pindex++)
				if(strcmp(bp.data.roominfo.players[pindex],username)==0)
					break;
			assert(pindex<3);
			cd.RenderPlayerFramework(bp.data.roominfo,pindex,true);
		}
		if(sp.type==_PACKET_ROOM_TEXT)
		{
			AddRoomText(sp);
			RenderRoomText(gi);
		}
		if(sp.type==_PACKET_ROOM_START)
		{
			bp=sp;
			gi.SetWaitInputFlag(true,true);
			WaitForSingleObject(hInput,INFINITE);
			gi.SetWaitInputFlag(false,false);
			hInput=NULL;
			//TerminateThread(hInput,0);
			break;
		}
	}
	/* Don't stop receiving messages */
	//cs.StopRecvMsg();
	

	/* Game start ! */
	gi.ClearScreen();
	RenderFramework(bp,gi,cd);
	RenderRoomText(gi);
	_socket_packet_cardinfo cardlu,cardru,cardme,handcard;
	cardlupos.Y=_input_window.Top+5;
	cardlupos.X=_input_window.Left+5;
	//cd.RenderOCards(cardlu,cardlupos);
	cardrupos.Y=_input_window.Top+5;
	cardrupos.X=_input_window.Right-12;
	//cd.RenderOCards(cardru,cardrupos);
	cardmepos.Y=_input_window.Bottom-10;
	cardmepos.X=_input_window.Left+17;
	bool msld[64];
	memset(msld,0,sizeof(msld));
	//cd.RenderMCards(cardme,cardmepos,msld);
	handcardpos.Y=_input_window.Bottom-7;
	handcardpos.X=_input_window.Right-14;
	handcard.cnt=3;
	handcard.cards[1].card=handcard.cards[1].color=handcard.cards[1].style=' ';
	handcard.cards[2].card=handcard.cards[2].color=handcard.cards[2].style=' ';
	handcard.cards[3].card=handcard.cards[3].color=handcard.cards[3].style=' ';
	cd.RenderLimitedOutCards(handcard,handcardpos);

	cardmopos=cardmepos;
	cardmopos.Y-=6;
	cardluopos=cardmopos;
	cardluopos.Y-=8;
	cardluopos.X-=4;
	cardruopos=cardluopos;
	cardruopos.Y-=8;
	cardruopos.X+=5+5;

	countlupos=cardluopos;
	countlupos.X+=4;
	countrupos=cardruopos;
	countrupos.X+=32;
	countmepos=cardmopos;
	countmepos.Y++;
	countmepos.X+=18;

	/* Don't stop receiving messages */
	//cs.StartRecvMsg();
	_socket_packet cp;
	_stoc stoc={0};
	stoc.gi=&gi;
	stoc.sec=_PACKET_WAITCDZ_TIMEOUT;
	HANDLE hWaitCDZ=NULL;
	int pdz=-1;
	while(1)
	{
		Sleep(100);
		cp=cs.RecvMsg();
		if(cp.type==0)
			continue;
		assert(cp.type==_PACKET_ROOM_INROOM);
		int cpos=-1;
		if(cp.aa==pindex)
			cpos=0;
		else if(cp.aa==(pindex+1)%3)
			cpos=1;
		else if(cp.aa==(pindex+2)%3)
			cpos=2;
		switch(cpos)
		{
			case 0:
				stoc.pos=countmepos;
				break;
			case 1:
				stoc.pos=countrupos;
				break;
			case 2:
				stoc.pos=countlupos;
				break;
			default:
				stoc.pos.X=stoc.pos.Y=-1;
		}
		if(cp.stype==_PACKET_ROOM_CARDINFO)
		{
			/* Clear the original content */
			/*gi.ChangePos(countmepos);
			printf("        ");
			gi.ChangePos(countrupos);
			printf("        ");
			gi.ChangePos(countlupos);
			printf("        ");*/
			/* Don't buffer the output, showing that it's changing cards */
			cd.SetBufferActive(false);
			cd.ClearLimitedOutCards(cardmopos);
			cd.ClearLimitedOutCards(cardruopos);
			cd.ClearLimitedOutCards(cardluopos);
			switch(cpos)
			{
				case 0:
					cardme=cp.data.cardinfo;
					cd.RenderMCards(cp.data.cardinfo,cardmepos,msld);
					break;
				case 1:
					cardru=cp.data.cardinfo;
					cd.RenderOCards(cp.data.cardinfo,cardrupos);
					break;
				case 2:
					cardlu=cp.data.cardinfo;
					cd.RenderOCards(cp.data.cardinfo,cardlupos);
					break;
			}
			cd.SetBufferActive(true);
		}
		else if(cp.stype==_PACKET_ROOM_CARDINFO_CDZWAITING)
		{
			if(hWaitCDZ)
			{
				TerminateThread(hWaitCDZ,0);
				hWaitCDZ=NULL;
				Sleep(100);
			}
			assert(stoc.pos.X>0 && stoc.pos.Y>0);
			hWaitCDZ=CreateThread(NULL,0,ShowTimeoutCount,&stoc,0,NULL);
			/* cp.a is roomindex, use cp.aa instead */
			if(cp.aa==pindex)
			{
				Sleep(200);
				HANDLE hWaitCDZI=CreateThread(NULL,0,WaitForCDZI,&gi,0,NULL);
				//WaitForSingleObject(hWaitCDZI,(stoc.sec-1)*1000);
				DWORD exc=0,exc1=0;
				DWORD psTime=GetTickCount();
				while(1)
				{
					Sleep(200);
					if(GetExitCodeThread(hWaitCDZI,&exc)==0 || GetTickCount()>psTime+stoc.sec*1000)
					{
						gi.SetWaitInputFlag(true,true);
						//TerminateThread(hWaitCDZI,0);
						exc=0;
						break;
					}
					if(exc!=STILL_ACTIVE)
						break;
				}
				if(GetExitCodeThread(hWaitCDZ,&exc1)==0 || exc1==STILL_ACTIVE)
					TerminateThread(hWaitCDZ,0);
				gi.SetWaitInputFlag(true,true);
				WaitForSingleObject(hWaitCDZI,INFINITE);
				gi.SetWaitInputFlag(false,false);
				hWaitCDZ=NULL;
				_socket_packet rp;
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_CARDINFO_CDZ;
				rp.a=roomindex;
				rp.aa=pindex;
				rp.data.a=exc;
				cs.SendMsg(rp);
				//gi.DrawOption(cdzopt,_INPUT_DRAWOPTION_CLEAR);
			}
		}
		else if(cp.stype==_PACKET_ROOM_CARDINFO_CDZ)
		{
			if(hWaitCDZ)
			{
				TerminateThread(hWaitCDZ,0);
				hWaitCDZ=NULL;
				Sleep(100);
			}
			/* Clear the original content */
			assert(stoc.pos.X>0 && stoc.pos.Y>0);
			gi.ChangePos(stoc.pos);
			gi.OutputStr("         ");
			gi.ChangePos(stoc.pos);
			gi.OutputStr(cp.data.a==0?"Not call\n":"Call DZ\n");
		}
		else if(cp.stype==_PACKET_ROOM_CARDINFO_SUREDZ)
		{
			pdz=cp.aa;
			handcard=cp.data.cardinfo;
			cd.RenderLimitedOutCards(handcard,handcardpos);
		}
		else if(cp.stype==_PACKET_ROOM_MULTIPLE)
		{
			/* Don't forget to do this every time receives the data */
			if(pdz>=0)
			{
				strcat(cp.data.roominfo.players[pdz]," (DZ)");
				for(int p=0;p<3;p++)
					if(p!=pdz)
						strcat(cp.data.roominfo.players[p]," (FM)");
			}
			cd.RenderPlayerFramework(cp.data.roominfo,pindex);
			sprintf(cp.data.b,"mul -> %d",cp.data.roominfo.mul);
			AddRoomText(cp);
			RenderRoomText(gi);
		}
		else if(cp.stype==_PACKET_ROOM_TEXT)
		{
			AddRoomText(cp);
			RenderRoomText(gi);
		}
		else if(cp.stype==_PACKET_ROOM_CARDINFO_START)
			break;
	}

	/* Game really starts! */
	HANDLE hWaitOUT=NULL;
	stoc.sec=_PACKET_WAITOUT_TIMEOUT;
	_socket_packet_cardinfo prevout;
	while(1)
	{
		Sleep(100);
		cp=cs.RecvMsg();
		if(cp.type==0)
			continue;
		assert(cp.type==_PACKET_ROOM_INROOM);
		int cpos=-1;
		if(cp.aa==pindex)
			cpos=0;
		else if(cp.aa==(pindex+1)%3)
			cpos=1;
		else if(cp.aa==(pindex+2)%3)
			cpos=2;
		switch(cpos)
		{
			case 0:
				stoc.pos=countmepos;
				break;
			case 1:
				stoc.pos=countrupos;
				break;
			case 2:
				stoc.pos=countlupos;
				break;
			default:
				stoc.pos.X=stoc.pos.Y=-1;
		}
		
		if(cp.stype==_PACKET_ROOM_CARDINFO_OUTWAITING)
		{
			switch(cpos)
			{
				case 0:
					cd.ClearLimitedOutCards(cardmopos);
					break;
				case 1:
					cd.ClearLimitedOutCards(cardruopos);
					break;
				case 2:
					cd.ClearLimitedOutCards(cardluopos);
					break;
				default:
					assert(0);
			}
			if(hWaitOUT)
			{
				TerminateThread(hWaitOUT,0);
				hWaitOUT=NULL;
				Sleep(100);
			}
			assert(stoc.pos.X>0 && stoc.pos.Y>0);
			hWaitOUT=CreateThread(NULL,0,ShowTimeoutCount,&stoc,0,NULL);
			if(cp.aa==pindex)
			{
				Sleep(200);
				memset(&_p_wfouti_cards,0,sizeof(_p_wfouti_cards));
				_stoo stoo;
				stoo.cards=cardme;
				stoo.cd=&cd;
				stoo.pos=cardmepos;
				/* An additional state showing whether it's a must to out */
				if(cp.aaa)
					memset(&prevout,0,sizeof(prevout));
				stoo.prevout=prevout;
				HANDLE hWaitOUTI=CreateThread(NULL,0,WaitForOUTI,&stoo,0,NULL);
				DWORD exc=0,exc1=0;
				DWORD psTime=GetTickCount();
				while(1)
				{
					Sleep(200);
					if(GetExitCodeThread(hWaitOUTI,&exc)==0 || GetTickCount()>psTime+stoc.sec*1000)
					{
						gi.SetWaitInputFlag(true,true);
						exc=0;
						break;
					}
					if(exc!=STILL_ACTIVE)
						break;
				}
				if(GetExitCodeThread(hWaitOUT,&exc1)==0 || exc1==STILL_ACTIVE)
					TerminateThread(hWaitOUT,0);
				hWaitOUT=NULL;
				/* Don't terminate, we still need it to clean its content */
				gi.SetWaitInputFlag(true,true);
				WaitForSingleObject(hWaitOUTI,INFINITE);
				gi.SetWaitInputFlag(false,false);

				/* Must play at least 1 card */
				if(cp.aaa && _p_wfouti_cards.cnt==0)
				{
					_p_wfouti_cards.cnt=1;
					_p_wfouti_cards.cards[1]=cardme.cards[1];
				}
				_socket_packet rp;
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_CARDINFO_OUT;
				rp.a=roomindex;
				rp.aa=pindex;
				rp.data.cardinfo=_p_wfouti_cards;
				cs.SendMsg(rp);
				/* Restore my cards */
				cardme.cnt=_gcard_RemoveCardsFrom(cardme.cards,cardme.cnt,_p_wfouti_cards.cards,_p_wfouti_cards.cnt);
				memset(msld,0,sizeof(msld));
				cd.RenderMCards(cardme,cardmepos,msld);
			}
		}
		else if(cp.stype==_PACKET_ROOM_CARDINFO_OUT)
		{
			if(hWaitOUT)
			{
				TerminateThread(hWaitOUT,0);
				hWaitOUT=NULL;
				Sleep(100);
			}
			if(cp.data.cardinfo.cnt)
				prevout=cp.data.cardinfo;
			switch(cpos)
			{
				case 0:
					cd.RenderMOCards(cp.data.cardinfo,cardmopos);
					if(cp.data.cardinfo.cnt==0)
					{
						gi.ChangePos(stoc.pos);
						gi.OutputStr("         ");
						gi.ChangePos(stoc.pos);
						gi.OutputStr("Skip");
					}
					/* These have been done already */
					//cardme.cnt=_gcard_RemoveCardsFrom(cardme.cards,cardme.cnt,cp.data.cardinfo.cards,cp.data.cardinfo.cnt);
					//memset(msld,0,sizeof(msld));
					//cd.RenderMCards(cardme,cardmepos,msld);
					break;
				case 1:
					cd.RenderRUOCards(cp.data.cardinfo,cardruopos);
					if(cp.data.cardinfo.cnt==0)
					{
						gi.ChangePos(stoc.pos);
						gi.OutputStr("         ");
						gi.ChangePos(stoc.pos);
						gi.OutputStr("Skip");
					}
					cardru.cnt=_gcard_RemoveCardsFrom(cardru.cards,cardru.cnt,cp.data.cardinfo.cards,cp.data.cardinfo.cnt);
					cd.RenderOCards(cardru,cardrupos);
					break;
				case 2:
					cd.RenderLUOCards(cp.data.cardinfo,cardluopos);
					if(cp.data.cardinfo.cnt==0)
					{
						gi.ChangePos(stoc.pos);
						gi.OutputStr("         ");
						gi.ChangePos(stoc.pos);
						gi.OutputStr("Skip");
					}
					cardlu.cnt=_gcard_RemoveCardsFrom(cardlu.cards,cardlu.cnt,cp.data.cardinfo.cards,cp.data.cardinfo.cnt);
					cd.RenderOCards(cardlu,cardlupos);
					break;
				default:
					assert(0);
			}
		}
		else if(cp.stype==_PACKET_ROOM_MULTIPLE)
		{
			/* Don't forget to do this every time receives the data */
			strcat(cp.data.roominfo.players[pdz]," (DZ)");
			for(int p=0;p<3;p++)
				if(p!=pdz)
					strcat(cp.data.roominfo.players[p]," (FM)");
			cd.RenderPlayerFramework(cp.data.roominfo,pindex);
			sprintf(cp.data.b,"mul -> %d",cp.data.roominfo.mul);
			AddRoomText(cp);
			RenderRoomText(gi);
		}
		else if(cp.stype==_PACKET_ROOM_TEXT)
		{
			AddRoomText(cp);
			RenderRoomText(gi);
		}
		if(cp.stype==_PACKET_ROOM_CARDINFO_END)
		{
			/* Don't forget to do this every time receives the data */
			strcat(cp.data.roominfo.players[pdz]," (DZ)");
			for(int p=0;p<3;p++)
				if(p!=pdz)
					strcat(cp.data.roominfo.players[p]," (FM)");
			cd.RenderPlayerFramework(cp.data.roominfo,pindex);
			userdzs=cp.data.roominfo.dzs[pindex];
			break;
		}
	}

	/* Show all the players' cards */
	if(cardme.cnt)
		cd.RenderMOCards(cardme,cardmopos);
	if(cardlu.cnt)
		cd.RenderLUOCards(cardlu,cardluopos);
	if(cardru.cnt)
		cd.RenderRUOCards(cardru,cardruopos);
	Sleep(1000);
	gi.ShowOptions(NULL,rrsopt,vib,0,_input_none);

	/* Save the latest roominfo package to bp */
	_socket_packet sbp=cs.RecvMsg();
	while(sbp.type)
	{
		bp=sbp;
		sbp=cs.RecvMsg();
	}
	/* Don't stop receiving messages */
	//cs.StopRecvMsg();
	goto roomstart;

	if(hOnlineSend!=NULL)
	{
		TerminateThread(hOnlineSend,0);
		hOnlineSend=NULL;
	}
	return;
}

DWORD WINAPI CheckMenuPressed(LPVOID p)
{
	_input*gi=(_input*)p;
	// Be limited to technological issues, we have only to do this.
res:
	if(!gi->GetWaitInputFlag() && !gi->chinput.GetBlockInputFlag() && GetForegroundWindow()==GetConsoleWindow()
		&& (GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState('M') & 0x8000))
	{
		/* This menu is shown in the Layer 1 */
		gi->chinput.SetBlockInputFlag(true);
		/* Don't block _input, because it may interrupt normal process */
		//gi->SetWaitInputFlag(true);
		Sleep(200);

		/* Create/Attach to a new screen buffer instead of clearing the original one */
		_input ngi;
		/* We've change the active _input instance to ngi instead of gi */
		_p_c_hook_gi = &ngi;
		HANDLE hOutput=CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,
			NULL,CONSOLE_TEXTMODE_BUFFER,NULL);
		if(hOutput!=INVALID_HANDLE_VALUE)
		{
			SetConsoleActiveScreenBuffer(hOutput);
			ngi.SetOutputHandle(hOutput);
			ngi.SetCursorVis(false);
			/* Clear the buffer */
			ngi.ClearRect(_input_window);
		}
		
		int ret=0;
		while(true)
		{
			std::vector<SMALL_RECT>adbox;
			std::vector<_option>menuopt;
			_option nopt;
			nopt.name="Close";
			nopt.pos.X=((_input_menu_box.Left+_input_menu_box.Right-strlen(nopt.name))/2);
			nopt.pos.Y=_input_menu_box.Bottom-1;
			menuopt.push_back(nopt);
			SMALL_RECT rect={_input_menu_box.Left+1,nopt.pos.Y-1,_input_menu_box.Right-1,nopt.pos.Y-1};
			adbox.push_back(rect);
			nopt.name="Help";
			nopt.pos.X++;
			nopt.pos.Y-=2;
			menuopt.push_back(nopt);
			SMALL_RECT rect1={_input_menu_box.Left+1,nopt.pos.Y-1,_input_menu_box.Right-1,nopt.pos.Y-1};
			adbox.push_back(rect1);
			nopt.name="Settings";
			nopt.pos.X-=2;
			nopt.pos.Y-=2;
			menuopt.push_back(nopt);
			SMALL_RECT rect2={_input_menu_box.Left+1,nopt.pos.Y-1,_input_menu_box.Right-1,nopt.pos.Y-1};
			adbox.push_back(rect2);
			nopt.name="Userinfo";
			nopt.pos.X++;
			nopt.pos.Y-=2;
			menuopt.push_back(nopt);
			SMALL_RECT rect3={_input_menu_box.Left+1,nopt.pos.Y-1,_input_menu_box.Right-1,nopt.pos.Y-1};
			adbox.push_back(rect3);
			nopt.name="License";
			nopt.pos.X--;
			nopt.pos.Y-=2;
			menuopt.push_back(nopt);
			SMALL_RECT rect4={_input_menu_box.Left+1,nopt.pos.Y-1,_input_menu_box.Right-1,nopt.pos.Y-1};
			adbox.push_back(rect4);
			nopt.name="About";
			nopt.pos.X++;
			nopt.pos.Y-=2;
			menuopt.push_back(nopt);
			SMALL_RECT rect5={_input_menu_box.Left+1,nopt.pos.Y-1,_input_menu_box.Right-1,nopt.pos.Y-1};
			adbox.push_back(rect5);

			for(int i=0;i<menuopt.size();i++)
				menuopt[i].jmp.reset(-1,-1,(i!=menuopt.size()-1 ? i+1 : -1),(i ? i-1 : -1));

			ret=ngi.ShowOptions(" An Ugly Menu",menuopt,vib,ret,_input_menu_box,adbox,1,false,true,true);
			/* Clear option attributes */
			ngi.DrawOption(menuopt,ret,true,false);
			if(ret==0)
				break;
			if(ret==1)
				ShowUsage(ngi);
			if(ret==3)
			{
				if(!username || strlen(username)==0)
					ngi.ShowOptions("\n\n\n\n\n\n\nIt seems that you haven't logged in yet !",bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,false);
				else
				{
					std::vector<_option>musropt;
					musropt.push_back(dslopt[2]);
					musropt[0].name="Edit Password";
					musropt.push_back(dslopt[4]);
					musropt[1].name="Close";
					musropt.push_back(dslopt[3]);
					musropt[2].name="Logout";
					musropt[0].jmp.reset();
					musropt[1].jmp.reset();
					musropt[2].jmp.reset();
					musropt[0].pos.Y-=3;musropt[0].pos.X-=10;
					musropt[1].pos.Y-=7;musropt[1].pos.X+=6;
					musropt[2].pos.Y-=3;musropt[2].pos.X+=7;
					char buf[1024]={0};
					sprintf(buf,"\n\nUserInfo\n\n\nUsername: %s\n\n\ndzs: %d\n\n",username,userdzs);
					int sret=ngi.ShowOptions(buf,musropt,vib,1,_input_default_box,_input_none_vbox,0,true,false);
					if(sret==0)
					{
						std::vector<_input_box>vni;
						vni.clear();
						_input_box ni1;
						ni1.rstr=NULL;
						ni1.show=true;ni1.defedit=true;
						ni1.defvalue=NULL;
						ni1.pos=_input_default_input_box;
						ni1.pos.Top-=1;ni1.pos.Bottom-=1;
						ni1.controlby[0]=0;ni1.controlby[1]=1;
						vni.push_back(ni1);
						std::vector<_option>msldopt;
						msldopt.clear();
						msldopt.push_back(dslopt[2]);
						msldopt.push_back(dslopt[3]);
						msldopt.push_back(dslopt[4]);
						msldopt[0].jmp.reset(-1,1,-1,2);
						msldopt[1].jmp.reset(0,-1,-1,2);
						msldopt[2].jmp.reset(-1,-1,0,-1);
						msldopt[0].pos.Y-=3;msldopt[1].pos.Y-=3;
						msldopt[2].pos.Y-=4;
						SMALL_RECT rc=_input_default_box;
						rc.Bottom+=2;
						sret=ngi.ShowOptions("\nInput your new password\n\n",msldopt,vni,0,rc,_input_none_vbox,0,true,false);
						_socket_packet sp;
						sp.type=_PACKET_USER_EDITPASS;
						strcpy(sp.data.userinfo.name,username);
						strcpy(sp.data.userinfo.pass,vni[0].rstr);
						_p_c_hook_cs->SendMsg(sp);
						ngi.ShowOptions("\n\n\n\n\n\n\nRequest has been sent to the server !",bakopt,vib,0,_input_default_box,_input_none_vbox,0,true,false);
					}
					else if(sret==1)
					{
						;
					}
					else if(sret==2)
					{
						;
					}
				}
			}
			if(ret==4)
				ValidateLicense(ngi,false);
			if(ret==5)
				ngi.ShowOptions("\nAbout\n\n\nCCDDZ\n\nDesigned by mnihyc\n\nProgrammed by mnihyc\n\nFrom 2018/7 to `time`\n\nEmail: rmnihyc@gmail.com",msgopt,vib,0,_input_default_box,_input_none_vbox,0,true,false);
			
		}

		
		/* Attach to the original screen buffer */
		if(hOutput != INVALID_HANDLE_VALUE)
		{
			SetConsoleActiveScreenBuffer(gi->GetOutputHandle());
			CloseHandle(hOutput);
		}
		
		_p_c_hook_gi = gi;
		/* Clear console input before resuming the main thread */
		gi->chinput.FlushBuffer();
		gi->chinput.SetBlockInputFlag(false);

		//gi->SetWaitInputFlag(false);
	}
	Sleep(50);
	goto res;

	return 0;
}

DWORD WINAPI WaitForOUTI(LPVOID p)
{
	_stoo*stoo=(_stoo*)p;
	_p_wfouti_cards=stoo->cd->SelectMCards(stoo->cards,stoo->pos,stoo->prevout);
	return 0;
}

DWORD WINAPI WaitForCDZI(LPVOID p)
{
	_input*gi=(_input*)p;
	return gi->ShowOptions(NULL,cdzopt,vib,0,_input_none,_input_none_vbox,0,false);
}

DWORD WINAPI ShowTimeoutCount(LPVOID p)
{
	_stoc stoc=*(_stoc*)p;
	/* Clear the original content */
	assert(stoc.pos.X>0 && stoc.pos.Y>0);
	//stoc.gi->ChangePos(stoc.pos);
	stoc.gi->OutputStrAt("         ",stoc.pos);
	for(;stoc.sec>=0;stoc.sec--)
	{
		//stoc.gi->ChangePos(stoc.pos);
		char buf[8];
		sprintf(buf,"%2d",stoc.sec);
		stoc.gi->OutputStrAt(buf,stoc.pos);
		Sleep(1000);
	}
	return 0;
}

void AddRoomText(_socket_packet bp)
{
	if(strlen(_rf_text))
		strcat(_rf_text,"\n");
	strcat(_rf_text,bp.data.b);
	std::vector<std::string>setences;
	std::string ns="";
	for(int i=0;i<strlen(_rf_text);i++)
	{
		if(_rf_text[i]=='\n')
		{
			setences.push_back(ns);
			ns="";
			continue;
		}
		ns+=_rf_text[i];
	}
	setences.push_back(ns);

	memset(_rf_text,0,sizeof(_rf_text));
	for(int i=max((int)setences.size()-1-6,0);i<setences.size();i++)
	{
		strcat(_rf_text,setences[i].c_str());
		strcat(_rf_text,"\n");
	}
	_rf_text[strlen(_rf_text)-1]=0;
}

void RenderRoomText(_input&gi)
{
	SMALL_RECT textrect={1,_input_window.Bottom-8,15,_input_window.Bottom-1};
	gi.ClearRect(textrect);
	//gi.DrawBox(textrect,false);
	/* Fix the boundary of the box manually */
	SMALL_RECT rc={textrect.Left,textrect.Top,textrect.Right,textrect.Bottom-1};
	gi.DrawBox(rc,false);
	SMALL_RECT rc1={rc.Left+1,rc.Bottom,rc.Right-1,rc.Bottom};
	gi.DrawBox(rc1,2);
	rc.Right=rc.Left;
	rc.Top+=2;rc.Bottom=textrect.Bottom;
	gi.DrawBox(rc,false);
	rc.Left=rc.Right=textrect.Right;
	gi.DrawBox(rc,false);
	
	
	textrect.Top-=2;
	textrect.Left--;textrect.Right++;
	gi.ShowTexts(_rf_text,textrect);
}

void RenderFramework(_socket_packet bp,_input&gi,_card&cd)
{
	/* Don't backup bp here, since it's been edited before */
	cd.RenderPlayerFramework(bp.data.roominfo,pindex);
	gi.DrawBox(_input_window,false);
	gi.ShowTexts(roomtitle,_input_window);
}

DWORD WINAPI WaitForReadyMessage(LPVOID p)
{
	_input*gi=(_input*)p;
	/* Draw this box manually, for better the clean of it can be buffered */
	gi->DrawBox(_input_window,false);
	gi->ShowTexts(roomtitle,_input_window);
	/* The content is based on the original one, so couldn't overwrite it */
	return gi->ShowOptions(NULL,pstopt,vib,0,_input_none,_input_none_vbox,0,false,false);
}

DWORD WINAPI RefreshRoomCenter(LPVOID p)
{
	_input*gi=(_input*)p;
	/* Draw this box manually, for better the clean of it can be buffered */
	gi->DrawBox(_input_window,false);
	gi->ShowTexts("Join/Create a room\n",_input_window);
	return gi->ShowOptions(NULL,romopt,vib,_r_refresh_default,_input_none,_input_none_vbox,0,false,true);
}

_socket_packet LoopRecv(_socket&cs,_input&gi,bool show)
{
	bool bNst=cs.bStopRecv;
	if(bNst)
		cs.StartRecvMsg();
	_socket_packet sp;
	DWORD lasttime=GetTickCount();
	while(1)
	{
		Sleep(100);
		/* Abandoned function */
		/*if(gi.GetWaitInputFlag())
		{
			if(show)
				gi.StopShowWaitingTexts();
			while(gi.GetWaitInputFlag())
				Sleep(100);
			lasttime=GetTickCount();
			if(show)
			{
				gi.ShowTexts("Communicating with the server .......",_input_default_input_box_center);
				gi.StartShowWaitingTexts();
			}
		}*/
		if(GetTickCount()-lasttime >= _PACKET_TIMEOUT)
			break;
		sp=cs.RecvMsg();
		if(sp.type==0)
			continue;
		/* Abandoned function */
		//else if(sp.type==_PACKET_MESSAGE_KEEPALIVE)
		//	cs.SendMsg(sp);
		else break;
	}
	if(bNst)
		cs.StopRecvMsg();
	return sp;
}


bool CommunicateW(_socket_packet msg,_socket&cs,_input&gi,_socket_packet&sp,bool show)
{
	/* Clear recv buffer, coz we want a communication */
	while(cs.RecvMsg().type!=0)
		;
	Sleep(50);
	char sbuf[1024]={0};
	if(show)
	{
		gi.ShowTexts("Communicating with the server .......",_input_default_input_box_center);
		gi.StartShowWaitingTexts();
	}
	int sret=cs.SendMsg(msg);
	if(sret==-1)
	{
		gi.StopShowWaitingTexts();
		gi.ClearScreen();
		sprintf(sbuf,"\n\n\n\n\n\nCouldn't send the packet to the server...\n\n\n"
			"sendto(%d) with exited code: %d",WSAGetLastError(),sret);
		gi.ShowOptions(sbuf,bakopt,vib,0,_input_default_box,_input_none_vbox,0,true);
		return false;
	}
	sp=LoopRecv(cs,gi,show);
	if(show)
		gi.StopShowWaitingTexts();
	if(sp.type==0)
	{
		gi.ClearScreen();
		gi.ShowOptions("\n\n\n\n\n\nCouldn't communicate with the server...\n\n\n"
			"No response received from the server\n\n",bakopt,vib,0,_input_default_box,_input_none_vbox,0,true);
		return false;
	}
	return true;
}


DWORD WINAPI OnlineSendMessage(LPVOID p)
{
	_socket*cs=(_socket*)p;
	_socket_packet sp;
	sp.type=_PACKET_MESSAGE_KEEPALIVE;
	strcpy(sp.data.b,username);
	while(1)
	{
		cs->SendMsg(sp);
		Sleep(_USER_SEND_PERTIME);
	}
	return 0;
}


DWORD _p_c_hook_lasttime;
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{	
	/* Whether CTRL_C_EVENT CTRL_BREAK_EVENT CTRL_CLOSE_EVENT CTRL_LOGOFF_EVENT CTRL_SHUTDOWN_EVENT */
	/* Prevent repeated CTRL+C executions through a time limit set */
	if(!_p_c_hook_bBlock && GetTickCount()-_p_c_hook_lasttime>50)
	{
		/* There's no need to do this, 'cos we've done the screen stuffs */
		/* Suspend the main thread */
		//SuspendThread(_h_main_thread);

		bool backup1=_p_c_hook_gi->chinput.GetBlockInputFlag();
		_p_c_hook_gi->chinput.SetBlockInputFlag(true);
		_p_c_hook_bBlock=true;
		/* Don't block _input, because it may interrupt normal process */
		//bool backup2=_p_c_hook_gi->GetWaitInputFlag();
		//_p_c_hook_gi->SetWaitInputFlag(true);
		Sleep(200);
		//_p_c_hook_gi->ClearRect(_input_window);

		/* Create/Attach to a new screen buffer instead of clearing the original one */
		_input ngi;
		HANDLE hOutput=CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,
			NULL,CONSOLE_TEXTMODE_BUFFER,NULL);
		if(hOutput!=INVALID_HANDLE_VALUE)
		{
			SetConsoleActiveScreenBuffer(hOutput);
			ngi.SetOutputHandle(hOutput);
			ngi.SetCursorVis(false);
			/* Clear the buffer */
			ngi.ClearRect(_input_window);
		}

		_socket_packet sp;
		if(username!=NULL)
		{
			sp.type=_PACKET_MESSAGE_OFFLINE;
			strcpy(sp.data.b,username);
		}
		if(CEvent==CTRL_CLOSE_EVENT)
		{
			if(_p_c_hook_cs && username!=NULL)
				_p_c_hook_cs->SendMsg(sp);
			ngi.DrawBox(_input_default_box);
			ngi.ShowTexts("\n\n\n\n\n\nThe program is going to exit in few seconds......\n\n\n"
				"Use CTRL+C or CTRL+BREAK instead \n"
				"if you want to exit safely.",_input_default_box);
			Sleep(500);
			_p_c_hook_cs->CleanUp();
			WSACleanup();
			exit(0);
		}
		/* Use different _input instances to realize normal dialog functions */
		int sret=ngi.ShowOptions("\n\n\n\n\n\nAre you sure to exit?",ynopt,vib,1,_input_default_box,_input_none_vbox,0,true);
		if(sret==0)
		{
			if(_p_c_hook_cs && username!=NULL)
				_p_c_hook_cs->SendMsg(sp);
			Sleep(100);
			_p_c_hook_cs->CleanUp();
			WSACleanup();
			/* Exit safely with exit() but not ExitProcess() */
			exit(0);
		}
		//_p_c_hook_gi->SetWaitInputFlag(backup2);
		_p_c_hook_bBlock=false;

		/* Attach to the original screen buffer */
		if(hOutput != INVALID_HANDLE_VALUE)
		{
			SetConsoleActiveScreenBuffer(_p_c_hook_gi->GetOutputHandle());
			CloseHandle(hOutput);
		}

		/* Clear console input before resuming the main thread */
		/* Are mainly to prevent repeated CTRL+C executions */
		_p_c_hook_gi->chinput.FlushBuffer();
		_p_c_hook_gi->chinput.SetBlockInputFlag(backup1);
		_p_c_hook_lasttime=GetTickCount();

		/* Resume the main thread */
		//ResumeThread(_h_main_thread);
	}
	/* Block this message */
	return TRUE;
}








