#pragma once
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <windows.h>
#include "other.h"

#pragma pack(1)
struct SockDataHeader{
	int length;
};
struct SockData{
	int ID;
	union{
		long p1;
		char*p2;
		void*p3;
	};
};
#pragma pack()

class MySocket{
	public:
		bool CheckInited();
		void SetSocket(SOCKET);
		SOCKET GetSocket();
		bool ConnectTo(const char*,long);
		bool ListenAt(long);
		MySocket();
		~MySocket();
		bool Send(SockData);
		bool Recv(SockData&);
		bool SetSendTimeout(int TimeOut);
		bool SetRecvTimeout(int TimeOut);
		SOCKET Accept(char**,long&);
	private:
		bool InitSocket();
		bool Inited;
		sockaddr_in SetSockAddrIn(const char*,long);
		
		SOCKET sock;
		//HANDLE Thread;
};
