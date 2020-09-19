#ifndef _H_DEBUG
#define _H_DEBUG

#pragma once

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>



namespace DEBUG{

inline void ChangeColor(UINT Color)
{
	HANDLE Hd=GetStdHandle(STD_OUTPUT_HANDLE);
	if(Hd==NULL)
		ExitProcess(-1);
	if(!SetConsoleTextAttribute(Hd,Color))
		ExitProcess(-1);
}

inline void normal()
{
	ChangeColor(FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
}

inline void green(const char*str, ...)
{
	ChangeColor(FOREGROUND_GREEN);
	va_list ap;
	va_start(ap,str);
	char buf[1024]={0};
	vsprintf(buf,str,ap);
	va_end(ap);
	printf("%s",buf);
	normal();
}

inline void red(const char*str, ...)
{
	ChangeColor(FOREGROUND_RED);
	va_list ap;
	va_start(ap,str);
	char buf[1024]={0};
	vsprintf(buf,str,ap);
	va_end(ap);
	printf("%s",buf);
	normal();
}

inline void highlight(const char*str, ...)
{
	ChangeColor(FOREGROUND_INTENSITY|FOREGROUND_RED| FOREGROUND_GREEN|FOREGROUND_BLUE);
	va_list ap;
	va_start(ap,str);
	char buf[1024]={0};
	vsprintf(buf,str,ap);
	va_end(ap);
	printf("%s",buf);
	normal();
}

}








#endif