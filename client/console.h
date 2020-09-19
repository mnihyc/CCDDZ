#ifndef _H_CONSOLE
#define _H_CONSOLE

#pragma once
#define _WIN32_WINNT 0x0500
#include <windows.h>
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow();
#include <conio.h>

class _ConsoleHelper
{
private:
    HANDLE _hIn;
    INPUT_RECORD _InRec;
	bool blockInput;
	int hookAddress;
public:
	bool shift;
	bool caps;
    WORD VKey;
    _ConsoleHelper(void)
	{
        _hIn = GetStdHandle(STD_INPUT_HANDLE);
        VKey=0;
		blockInput=false;
		hookAddress=0;
    }
	/* void funcName(INPUT_RECORD) */
	void InstallHookFunction(int address)
	{
		hookAddress=address;
	}
	void SetBlockInputFlag(bool bI)
	{
		blockInput=bI;
		this->FlushBuffer();
	}
	bool GetBlockInputFlag()
	{
		return blockInput;
	}
	bool PeekOneInput(INPUT_RECORD&InRec)
    {
		if(blockInput)
			return false;
		DWORD _NumRead;
        DWORD dwRet=PeekConsoleInput(_hIn,&InRec,1,&_NumRead);
		if(hookAddress)
			((void(*)(INPUT_RECORD))hookAddress)(InRec);
		return dwRet!=0;
    }
    DWORD ReadKeyPush()
    {
		if(blockInput)
			return 0;
		DWORD _NumRead;
		if(!GetNumberOfConsoleInputEvents(_hIn,&_NumRead))
			return 0;
		if(!_NumRead)
			return 0;
		/* This function will continue blocking if there're no events ! */
        if(!ReadConsoleInput(_hIn,&_InRec,1,&_NumRead))
            return 0;
        if(_InRec.EventType!=KEY_EVENT)
            return 0;
        if(_InRec.Event.KeyEvent.bKeyDown == 0)
            return 0;
        VKey = _InRec.Event.KeyEvent.wVirtualKeyCode;
		shift=(_InRec.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)!=0;
		caps=(_InRec.Event.KeyEvent.dwControlKeyState & CAPSLOCK_ON)!=0;
		if(hookAddress)
			((void(*)(INPUT_RECORD))hookAddress)(_InRec);
        return VKey;
    }
	void FlushBuffer()
	{
		FlushConsoleInputBuffer(_hIn);
	}
    ~_ConsoleHelper(void){}
};





#endif