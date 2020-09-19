#include "input.h"

// Fix the issue that the variable is beyond its definition domain.
#define for if(0);else for

_input::_input()
{
	hOutput=GetStdHandle(STD_OUTPUT_HANDLE);
	SetCursorVis(false);
	SetWaitInputFlag(false);
}

void _input::StartOutputBuffer()
{
	this->hOBuffer=CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,
		NULL,CONSOLE_TEXTMODE_BUFFER,NULL);
	COORD pos={0,0};
	char buf[6400+1]={0};
	DWORD dwRet;
	ReadConsoleOutputCharacter(this->hOutput,buf,3200,pos,&dwRet);
	WriteConsoleOutputCharacter(this->hOBuffer,buf,3200,pos,&dwRet);
	/* Remember to copy the buffer attributes as well! */
	WORD wbuf[6400+1]={0};
	ReadConsoleOutputAttribute(this->hOutput,wbuf,3200,pos,&dwRet);
	WriteConsoleOutputAttribute(this->hOBuffer,wbuf,3200,pos,&dwRet);
	this->hOutputBak=this->hOutput;
	this->hOutput=this->hOBuffer;
	/* Prevent cursor from showing in the wrong place. */
	this->SetCursorVis(false);
}

void _input::FlushOutputBuffer()
{
	COORD pos={0,0};
	char buf[6400+1]={0};
	DWORD dwRet;
	ReadConsoleOutputCharacter(this->hOBuffer,buf,3200,pos,&dwRet);
	WriteConsoleOutputCharacter(this->hOutputBak,buf,3200,pos,&dwRet);
	/* Remember to copy the buffer attributes as well! */
	WORD wbuf[6400+1]={0};
	ReadConsoleOutputAttribute(this->hOBuffer,wbuf,3200,pos,&dwRet);
	WriteConsoleOutputAttribute(this->hOutputBak,wbuf,3200,pos,&dwRet);
	this->hOutput=this->hOutputBak;
	CloseHandle(this->hOBuffer);
	this->hOBuffer=NULL;
}

void _input::SetOutputHandle(HANDLE hOutput)
{
	this->hOutput=hOutput;
}

HANDLE _input::GetOutputHandle()
{
	return this->hOutput;
}

void _input::DrawBox(SMALL_RECT rc,short bDouble)
{
	if(rc.Bottom<0 || rc.Left<0 || rc.Right<0 || rc.Top<0)
		return;
	char chBox[6];
	COORD pos;
	DWORD dwWritten;

	/* Only support page code 437 */
	if(bDouble==1)
	{
		chBox[0]=(char)0xc9; // LU
		chBox[1]=(char)0xbb; // RU
		chBox[2]=(char)0xc8; // LD
		chBox[3]=(char)0xbc; // RD
		chBox[4]=(char)0xcd; // H
		chBox[5]=(char)0xba; // V
	}
	else if(bDouble==0)
	{
		chBox[0]=(char)0xda; // LU
		chBox[1]=(char)0xbf; // RU
		chBox[2]=(char)0xc0; // LD
		chBox[3]=(char)0xd9; // RD
		chBox[4]=(char)0xc4; // H
		chBox[5]=(char)0xb3; // V
	}
	else
	{
		chBox[0]=' '; // LU
		chBox[1]=' '; // RU
		chBox[2]=' '; // LD
		chBox[3]=' '; // RD
		chBox[4]=' '; // H
		chBox[5]=' '; // V
	}
	
	for(pos.X=rc.Left;pos.X<=rc.Right;pos.X++)
	{
		if(pos.X==rc.Left && rc.Top!=rc.Bottom)
		{
			pos.Y=rc.Top; // LU
			WriteConsoleOutputCharacter(hOutput,&chBox[0],1,pos,&dwWritten);
			pos.Y = rc.Bottom;
			WriteConsoleOutputCharacter(hOutput,&chBox[2],1,pos,&dwWritten);
		}
		else if(pos.X==rc.Right && rc.Top!=rc.Bottom)
		{
			pos.Y=rc.Top; // RU
			WriteConsoleOutputCharacter(hOutput,&chBox[1],1,pos,&dwWritten);
			pos.Y=rc.Bottom; // RD
			WriteConsoleOutputCharacter(hOutput,&chBox[3],1,pos,&dwWritten);
		}
		else
		{
			pos.Y=rc.Top; // HU
			WriteConsoleOutputCharacter(hOutput,&chBox[4],1,pos,&dwWritten);
			pos.Y=rc.Bottom; // HD
			WriteConsoleOutputCharacter(hOutput,&chBox[4],1,pos,&dwWritten);
		}
	}
	
	for (pos.Y=rc.Top+(rc.Left==rc.Right ? 0 : 1);pos.Y<=rc.Bottom-(rc.Left==rc.Right ? 0 : 1);pos.Y++)
	{
		pos.X=rc.Left; // VL
		WriteConsoleOutputCharacter(hOutput,&chBox[5],1,pos,&dwWritten);
		pos.X=rc.Right; // VR
		WriteConsoleOutputCharacter(hOutput,&chBox[5],1,pos,&dwWritten);
	}

	return;
}


void _input::DrawBoxAnimatedly(SMALL_RECT rc,bool bDouble,int delay,int step,bool zoom)
{
	if(rc.Bottom<0 || rc.Left<0 || rc.Right<0 || rc.Top<0)
		return;
	SMALL_RECT nc;
	WORD midv=(rc.Top+rc.Bottom)/2,midh=(rc.Left+rc.Right)/2;
	nc.Top=midv;nc.Bottom=midv;
	nc.Left=midh;nc.Right=midh;

	int lh=1,lv=1;
	int dsh=rc.Right-rc.Left,dsv=rc.Bottom-rc.Top;
	if(dsh>dsv)
		lv=dsh/dsv;
	else
		lh=dsv/dsh;

	
	int times=0;
	/* Zoom in */
	while(!zoom)
	{
		times++;
		int ret=0;
		if(nc.Top-step>=rc.Top)
			nc.Top-=step*(times%lv==0);
		else
			ret++;
		if(nc.Bottom+step<=rc.Bottom)
			nc.Bottom+=step*(times%lv==0);
		else
			ret++;
		if(nc.Left-step>=rc.Left)
			nc.Left-=step*(times%lh==0);
		else
			ret++;
		if(nc.Right+step<=rc.Right)
			nc.Right+=step*(times%lh==0);
		else
			ret++;
		DrawBox(nc,bDouble);
		Sleep(delay);
		DrawBox(nc,2);
		if(ret==4)
		{
			DrawBox(rc,bDouble);
			break;
		}
	}

	times=0;
	/* Zoom out */
	if(zoom)
		DrawBox(rc,2);
	while(zoom)
	{
		times++;
		int ret=0;
		if(rc.Top+step<=nc.Top)
			rc.Top+=step*(times%lv==0);
		else
			ret++;
		if(rc.Bottom-step>=nc.Bottom)
			rc.Bottom-=step*(times%lv==0);
		else
			ret++;
		if(rc.Left+step<=nc.Left)
			rc.Left+=step*(times%lh==0);
		else
			ret++;
		if(rc.Right-step>=nc.Right)
			rc.Right-=step*(times%lh==0);
		else
			ret++;
		DrawBox(rc,bDouble);
		Sleep(delay);
		DrawBox(rc,2);
		if(ret==4)
		{
			// Nothing to do
			break;
		}
	}

	return;
}


void _input::DrawOption(std::vector<_option>opt,int sld,bool extra,bool highlight)
{
	if(sld!=_INPUT_DRAWOPTION_CLEAR && sld<0)
		sld+=opt.size();
	for(int i=0;i<opt.size();i++)
	{
		/* especially for >< */
		if(extra)
			opt[i].pos.X--;
		ChangePos(opt[i].pos);
		if(sld==_INPUT_DRAWOPTION_CLEAR)
		{
			/* especially for >< */
			int len=strlen(opt[i].name)+(extra ? 2 : 0);
			char*nc=new char[len+1];
			memset(nc,' ',len);
			nc[len]='\0';
			OutputStr(nc,_input_default_color);
			delete[]nc;
			continue;
		}

		if(i==sld)
		{
			/* especially for >< */
			if(extra)
				OutputStr(">",_input_default_color);
			OutputStr(opt[i].name,(highlight ? _input_default_option_color : _input_default_color));
			if(extra)
				OutputStr("<",_input_default_color);
		}
		else
		{
			/* especially for >< */
			if(extra)
				OutputStr(" ",_input_default_color);
			OutputStr(opt[i].name,_input_default_color);
			if(extra)
				OutputStr(" ",_input_default_color);
		}
	}
	return;
}


void _input::ChangePos(COORD pos)
{
	SetConsoleCursorPosition(hOutput,pos);
	return;
}


void _input::SetCursorVis(bool vis)
{
	CONSOLE_CURSOR_INFO cci;
	GetConsoleCursorInfo(hOutput,&cci);
	cci.bVisible=vis;
	SetConsoleCursorInfo(hOutput,&cci);
	return;
}


void _input::OutputStrAt(LPCSTR s,COORD pos)
{
	if(s==NULL || strlen(s)==0)
		return;
	DWORD dwRet;
	WriteConsoleOutputCharacter(hOutput,s,strlen(s),pos,&dwRet);
	return;
}


void _input::OutputStr(LPCSTR s,WORD att)
{
	if(s==NULL || strlen(s)==0)
		return;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hOutput,&csbi);
	if(att)
		SetConsoleTextAttribute(hOutput,att);
	DWORD dwRet;
	WriteConsole(hOutput,s,strlen(s),&dwRet,NULL);
	if(att)
		SetConsoleTextAttribute(hOutput,csbi.wAttributes);
	return;
}


void _input::InitializeOptions(std::vector<_option>&opt)
{
	int i;
	for(i=0;i<opt.size();i++)
		if(opt[i].jmp.ljmp>=0 || opt[i].jmp.rjmp>=0 || opt[i].jmp.ujmp>=0 || opt[i].jmp.djmp>=0)
			return;
	for(i=0;i<opt.size();i++)
	{
		opt[i].jmp.ljmp=(i ? i-1 : -1);
		opt[i].jmp.rjmp=(i+1<opt.size() ? i+1 : -1);
		opt[i].jmp.ujmp=opt[i].jmp.ljmp;
		opt[i].jmp.djmp=opt[i].jmp.rjmp;
	}
}

int _input::ShowOptions(LPCSTR ss,std::vector<_option>opt,std::vector<_input_box>&vib,int sld,SMALL_RECT pbox,const std::vector<SMALL_RECT>&adbox,short dbline,bool animated,bool clear,bool remain,bool manual)
{
	bool banimated=animated;
	/* Return the default selection */
	if(bInputEnd)
		return sld;
	/* Check whether it's blocked first */
	while(bInputFlag)
		Sleep(100);
	animated=banimated;
#ifdef _DEBUG
 	animated=false;clear=true; // FASTER DEBUG
#endif
	if(!manual)
	{
 		if(clear)
		{
			ClearRect(pbox);
			for(int i=0;i<adbox.size();i++)
				ClearRect(adbox[i]);
		}
		if(animated)
			DrawBoxAnimatedly(pbox,(dbline&1)>0,5,1,false);
		else
			DrawBox(pbox,(dbline&1)>0);
		for(int i=0;i<adbox.size();i++)
				DrawBox(adbox[i],(dbline&2)>0);
		ShowTexts(ss,pbox);
	}
	InitializeOptions(opt);
	DrawOption(opt,sld);

	std::vector<COORD>vcpos;
	vcpos.resize(vib.size());
	char vbuf[16][128][128];
	memset(vbuf,' ',sizeof(vbuf));
	bool isedit=false;
	int editpos;
	for(int k=0;k<vib.size();k++)
	{
		_input_box&ib=vib[k];
		COORD&cpos=vcpos[k];
		if(ib.show)
		{
			if(ib.defedit)
			{
				editpos=k;
				isedit=true;
				SetCursorVis(true);
			}
			DrawBox(ib.pos,true);
			/* We're going to use font size FONT_SIZE_X/Y */
			cpos.X=ib.pos.Left+1;
			cpos.Y=ib.pos.Top+1;
			ChangePos(cpos);
			if(ib.defvalue!=NULL)
			{
				/* Have to output with handle hOutput */
				//printf("%s",ib.defvalue);
				OutputStr(ib.defvalue);
				for(int i=0;i<strlen(ib.defvalue);i++)
					vbuf[k][0][i]=ib.defvalue[i];
				cpos.X+=strlen(ib.defvalue);
			}
		}
	}
	if(isedit)
	{
		COORD cpos;
		cpos.X=vib[editpos].pos.Left+1;
		cpos.Y=vib[editpos].pos.Top+1;
		ChangePos(cpos);
	}

	/* Flush the input buffer */
	chinput.FlushBuffer();
	while(1)
	{
		Sleep(10);
		/* Pause temporarily (loop here, keep the original content) */
		if(bInputFlag)
		{
			/* Return where cursor stops in time */
			/* Do remaining jobs, don't return now */
			if(bInputEnd)
				break;
			while(bInputFlag)
				Sleep(100);
			/* Restore the cursor state */
			if(isedit)
				SetCursorVis(true);
		}
		DWORD vKey=chinput.ReadKeyPush();
		if(vKey==0)
			continue;

		/* Multiple _input_box instances, find the correct one */
		int k;
		for(k=0;k<vib.size();k++)
			if(vib[k].controlby[0]==sld || vib[k].controlby[1]==sld)
				break;
		if(isedit)
			assert(k<vib.size());
		_input_box&ib=vib[k];
		COORD&cpos=vcpos[k];

recheck:
		if(!isedit)
		{
			if(vKey==VK_RETURN || vKey==VK_SPACE)
			{
				/* Especially for _INPUT_BOX */
				if(strcmp(opt[sld].name,_INPUT_DEFAULT_INPUT_NAME)==0 && ib.show)
				{
					assert(k<vib.size());
					isedit=true;
					SetCursorVis(true);
					ChangePos(cpos);
				}
				else if(strcmp(opt[sld].name,_INPUT_DEFAULT_INPUT_CNAME)==0 && ib.show)
				{
					assert(k<vib.size());
					SMALL_RECT cr;
					cr=ib.pos;cr.Bottom--;cr.Right--;
					cr.Top++;cr.Left++;
					ClearRect(cr);
					cpos.X=ib.pos.Left+1;
					cpos.Y=ib.pos.Top+1;
					ChangePos(cpos);
					memset(vbuf[k],0,sizeof(vbuf[k]));
				}
				else break;
			}
			if(vKey==VK_LEFT && opt[sld].jmp.ljmp>=0)
				DrawOption(opt,sld=opt[sld].jmp.ljmp);
			if(vKey==VK_RIGHT && opt[sld].jmp.rjmp>=0)
				DrawOption(opt,sld=opt[sld].jmp.rjmp);
			if(vKey==VK_UP && opt[sld].jmp.ujmp>=0)
				DrawOption(opt,sld=opt[sld].jmp.ujmp);
			if(vKey==VK_DOWN && opt[sld].jmp.djmp>=0)
				DrawOption(opt,sld=opt[sld].jmp.djmp);
		}
		else
		{
			if(vKey==VK_RETURN)
			{
				isedit=false;
				SetCursorVis(false);
				vKey=VK_DOWN;
				goto recheck;
			}
			if(vKey==VK_ESCAPE)
			{
				isedit=false;
				SetCursorVis(false);
			}
			if(vKey==VK_LEFT && cpos.X-1 >= ib.pos.Left+1)
			{
				cpos.X--;
				ChangePos(cpos);
			}
			if(vKey==VK_RIGHT && cpos.X+1 <= ib.pos.Right-1)
			{
				cpos.X++;
				ChangePos(cpos);
			}
			if(vKey==VK_UP && cpos.Y-1 >= ib.pos.Top+1)
			{
				cpos.Y--;
				ChangePos(cpos);
			}
			if(vKey==VK_DOWN && cpos.Y+1 <= ib.pos.Bottom-1)
			{
				cpos.Y++;
				ChangePos(cpos);
			}
			char vchar=tolower(MapVirtualKey(vKey,MAPVK_VK_TO_CHAR));
			/* I have no way .... */
			if(chinput.caps)
				if(vchar>='a' && vchar<='z')
					vchar=vchar-'a'+'A';
			if(chinput.shift)
				if(vchar>='a' && vchar<='z')
					vchar=vchar-'a'+'A';
				else if(vchar>='A' && vchar<='Z')
					vchar=vchar-'A'+'a';
				else if(vchar=='1')
					vchar='!';
				else if(vchar=='2')
					vchar='@';
				else if(vchar=='3')
					vchar='#';
				else if(vchar=='4')
					vchar='$';
				else if(vchar=='5')
					vchar='%';
				else if(vchar=='6')
					vchar='^';
				else if(vchar=='7')
					vchar='&';
				else if(vchar=='8')
					vchar='*';
				else if(vchar=='9')
					vchar='(';
				else if(vchar=='0')
					vchar=')';
				else if(vchar=='-')
					vchar='_';
				else if(vchar=='=')
					vchar='+';
				else if(vchar=='[')
					vchar='{';
				else if(vchar==']')
					vchar='}';
				else if(vchar=='\\')
					vchar='|';
				else if(vchar==';')
					vchar=':';
				else if(vchar=='\'')
					vchar='\"';
				else if(vchar==',')
					vchar='<';
				else if(vchar=='.')
					vchar='>';
				else if(vchar=='/')
					vchar='?';
				else if(vchar=='`')
					vchar='~';
			if(vchar>=32 && vchar<=126)
				/* The line doesn't exceed, jump to the next position */
				if(cpos.X+1<=ib.pos.Right-1)
				{
					vbuf[k][cpos.Y-ib.pos.Top-1][cpos.X-ib.pos.Left-1]=vchar;
					DWORD dwRet;
					WriteConsole(hOutput,&vchar,1,&dwRet,NULL);
					cpos.X++;
				}
				/* If the next new line is valid, jump to it */
				else if(cpos.X+1==ib.pos.Right && cpos.Y+1<=ib.pos.Bottom-1)
				{
					vbuf[k][cpos.Y-ib.pos.Top-1][cpos.X-ib.pos.Left-1]=vchar;
					DWORD dwRet;
					WriteConsole(hOutput,&vchar,1,&dwRet,NULL);
					cpos.X=ib.pos.Left+1;
					cpos.Y++;
					ChangePos(cpos);
				}
				/* The end of the input box, just don't jump */
				else if(cpos.X+1==ib.pos.Right && cpos.Y+1==ib.pos.Bottom)
				{
					vbuf[k][cpos.Y-ib.pos.Top-1][cpos.X-ib.pos.Left-1]=vchar;
					DWORD dwRet;
					WriteConsole(hOutput,&vchar,1,&dwRet,NULL);
					ChangePos(cpos);
				}
			if(vKey==VK_BACK && (vchar=' '))
				/* The line doesn't exceed, jump to the previous position */
				if(cpos.X-1>=ib.pos.Left+1)
				{
					cpos.X--;
					vbuf[k][cpos.Y-ib.pos.Top-1][cpos.X-ib.pos.Left-1]=vchar;
					ChangePos(cpos);
					DWORD dwRet;
					WriteConsole(hOutput,&vchar,1,&dwRet,NULL);
					ChangePos(cpos);
				}
				/* If the previous line is valid, jump to it */
				else if(cpos.X-1==ib.pos.Left && cpos.Y-1>=ib.pos.Top+1)
				{
					cpos.X=ib.pos.Right-1;
					cpos.Y--;
					vbuf[k][cpos.Y-ib.pos.Top-1][cpos.X-ib.pos.Left-1]=vchar;
					ChangePos(cpos);
					DWORD dwRet;
					WriteConsole(hOutput,&vchar,1,&dwRet,NULL);
					ChangePos(cpos);
				}
				/* The begin of the input box, just don't jump */
				else if(cpos.X-1==ib.pos.Left && cpos.Y-1==ib.pos.Top)
				{
					vbuf[k][cpos.Y-ib.pos.Top-1][cpos.X-ib.pos.Left-1]=vchar;
					DWORD dwRet;
					WriteConsole(hOutput,&vchar,1,&dwRet,NULL);
					ChangePos(cpos);
				}
		}
		vKey=0;
	}

	for(int k=0;k<vib.size();k++)
	{
		_input_box&ib=vib[k];
		COORD&cpos=vcpos[k];
		if(ib.show)
		{
			if(ib.rstr!=NULL)
				delete[]ib.rstr;
			ib.rstr=new char[(ib.pos.Right-ib.pos.Left+1)*(ib.pos.Bottom-ib.pos.Top+1)];
			memset(ib.rstr,0,sizeof(ib.rstr));
			int c=0;
			for(int m=ib.pos.Top+1;m<=ib.pos.Bottom-1;m++)
				for(int n=ib.pos.Left+1;n<=ib.pos.Right-1;n++)
					ib.rstr[c++]=vbuf[k][m-ib.pos.Top-1][n-ib.pos.Left-1];
			ib.rstr[c]='\0';
			trim(ib.rstr);
		}
	}

	if(!manual && !remain)
		DrawOption(opt,_INPUT_DRAWOPTION_CLEAR);
	if(!manual)
	{
		if(animated)
			DrawBoxAnimatedly(pbox,(dbline&1)>0,5,1,true);
		else
			if(!remain)
				DrawBox(pbox,2);
		if(!remain)
		{
			for(int i=0;i<adbox.size();i++)
				DrawBox(adbox[i],2);
			ClearRect(pbox);
			for(int i=0;i<adbox.size();i++)
				ClearRect(adbox[i]);
		}
	}

	return sld;
}


void _input::ShowTexts(LPCSTR str,SMALL_RECT pbox)
{
	if(str==NULL || strlen(str)==0 || pbox.Bottom<0 || pbox.Left<0 || pbox.Right<0 || pbox.Top<0)
		return;
	std::vector<std::string>setences;
	std::string ns="";
	for(int i=0;i<strlen(str);i++)
	{
		if(str[i]=='\n')
		{
			setences.push_back(ns);
			ns="";
			continue;
		}
		ns+=str[i];
	}
	setences.push_back(ns);
	
	/* We're going to use font size FONT_SIZE_X/Y */
	int nsk=pbox.Top/FONT_SIZE_X;
	for(int j=0;j<setences.size();j++)
		nsk+=ShowOneLineText(setences[j].c_str(),nsk,pbox);

	return;
}


int _input::ShowOneLineText(LPCSTR str,int skipped,SMALL_RECT box)
{
	int mol=(box.Right-box.Left)-3;
	COORD pos;
	int nsp=strlen(str)/mol;
	for(int i=1;i<=nsp;i++)
	{
		pos.X=box.Left+2;pos.Y=box.Top+skipped+i-1;
		ChangePos(pos);
		DWORD dwRet;
		for(int j=mol*(i-1);j<=mol*i-1;j++)
			WriteConsole(hOutput,&str[j],1,&dwRet,NULL);
	}
	pos.X=box.Left+2+(mol-strlen(&str[mol*nsp]))/2;
	pos.Y=box.Top+skipped+nsp;
	ChangePos(pos);
	OutputStr(&str[mol*nsp]);
	/* Fix the issue, a new line with nothing */
	if(nsp!=0 && strlen(&str[mol*nsp])==0)
		return nsp;
	return nsp+1;
}

//bool _input::bWaitingT=false;
/* This bug has been fixed, especially in EasySocket.h */

DWORD WINAPI _input::ShowWaitingTexts(LPVOID p)
{
	_input*_this=(_input*)p;
	while(1)
	{
		if(!_this->bWaitingT)
			break;
		_this->OutputStr("\b|");
		Sleep(100);
		if(!_this->bWaitingT)
			break;
		_this->OutputStr("\b/");
		Sleep(100);
		if(!_this->bWaitingT)
			break;
		_this->OutputStr("\b-");
		Sleep(100);
		if(!_this->bWaitingT)
			break;
		_this->OutputStr("\b\\");
		Sleep(100);
	}
	return 0;
}

void _input::StartShowWaitingTexts()
{
	DWORD tid;
	bWaitingT=true;
	hWaitingT=CreateThread(NULL,0,ShowWaitingTexts,this,0,&tid);
}

void _input::StopShowWaitingTexts()
{
	//TerminateThread(hWaitingT,0);
	/* BUG of TerminateThread() ?!!! */
	bWaitingT=false;
	if(hWaitingT)
	{
		WaitForSingleObject(hWaitingT,INFINITE);
		CloseHandle(hWaitingT);
	}
	hWaitingT=NULL;
}


void _input::ClearRect(SMALL_RECT cr,char*fw)
{
	if(cr.Bottom<0 || cr.Left<0 || cr.Right<0 || cr.Top<0)
		return;
	COORD cpos;
	cpos.X=cr.Left;
	cpos.Y=cr.Top;
	ChangePos(cpos);
	for(int i=cr.Top;i<=cr.Bottom;i++)
	{
		cpos.Y=i;
		ChangePos(cpos);
		/* Clear the text attributes as well */
		for(int j=cr.Left;j<=cr.Right;j++)
			OutputStr(fw,_input_default_color);
	}
}

void _input::SetWaitInputFlag(bool bW,bool bE)
{
	bInputFlag=bW,bInputEnd=bE;
}

bool _input::GetWaitInputFlag()
{
	return bInputFlag;
}

void _input::ClearScreen()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(this->hOutput,&csbi);
	DWORD dwSize=csbi.dwSize.X*csbi.dwSize.Y,dwWritten;
	COORD pos={0,0};
	FillConsoleOutputCharacter(this->hOutput,' ',dwSize,pos,&dwWritten);
	GetConsoleScreenBufferInfo(this->hOutput,&csbi);
	FillConsoleOutputAttribute(this->hOutput,csbi.wAttributes,dwSize,pos,&dwWritten);
	SetConsoleCursorPosition(this->hOutput,pos);
	/* Prevent cursor from showing in the wrong place. */
	this->SetCursorVis(false);
}









