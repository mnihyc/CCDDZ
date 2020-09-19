#include "MySocket.h"


bool MySocket::SetSendTimeout(int TimeOut)
{
	if(setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&TimeOut,sizeof(int))==SOCKET_ERROR && errno!=0)  
	{
		error("setsockopt(Send) error:%s,errorcode :%d",strerror(errno),WSAGetLastError());
		return false;
	}
	return true;
}
bool MySocket::SetRecvTimeout(int TimeOut)
{
	if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&TimeOut,sizeof(int))==SOCKET_ERROR && errno!=0)  
	{
		error("setsockopt(Recv) error:%s,errorcode :%d",strerror(errno),WSAGetLastError());
		return false;
	}
	return true;
}
bool MySocket::CheckInited()
{
	return Inited;
}
bool MySocket::InitSocket()
{
	WSAData ws;
	int ret=WSAStartup(MAKEWORD(2,2),&ws);
	if(ret!=0 || ws.wVersion!=MAKEWORD(2,2))
	{
		error("Couldn't load the WinSocket 2(WSAStartup(%ld)) !",GetLastError());
		WSACleanup();
		return false;
	}
	return true;
}
MySocket::MySocket()
{
	/*Thread=NULL;*/sock=NULL;
	Inited=InitSocket();
}
MySocket::~MySocket()
{
	WSACleanup();
}
sockaddr_in MySocket::SetSockAddrIn(const char*IP,long port)
{
	sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_addr.s_addr=inet_addr(IP);
	saddr.sin_port=port;
	return saddr;
}
void MySocket::SetSocket(SOCKET sNew)
{
	sock=sNew;
}
SOCKET MySocket::GetSocket()
{
	return sock;
}
bool MySocket::ConnectTo(const char*IP,long port)
{
	closesocket(sock);
	sock=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,0);
	if(sock==INVALID_SOCKET)
	{
		error("Create Socket Failed::%ld",GetLastError());
		return false;
	}
	sockaddr_in saddr=SetSockAddrIn(IP,port);
	int err=connect(sock,(struct sockaddr*)&saddr,sizeof(saddr));
	if (err==SOCKET_ERROR)
	{
		error("Connect Error::%d",GetLastError());
		return false;
	}
	else
	{
		info("Connected to '%s' at '%ld' !",IP,port);
		return true;
	}
}

bool MySocket::ListenAt(long port)
{
	closesocket(sock);
	if((sock=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,0))==INVALID_SOCKET) 
	{
		if (WSAGetLastError()==WSANOTINITIALISED)
		{
			error("Called WSAStartup() but WSANOTINITIALISED !!???");
			return false;
		}
		else
		{
			int err=WSAGetLastError();
			error("Socket error:%s,errorcode :%d",strerror(errno),err);
			return false;
		}
	}
	sockaddr_in ServerAddr=SetSockAddrIn("0.0.0.0",port);
	if(bind(sock,(struct sockaddr *)(&ServerAddr),sizeof(struct sockaddr))==-1)
	{
		int err=WSAGetLastError();
		error("Bind at %ld error:%s,errorcode :%d",port,strerror(errno),err);
		return false;
	}
	if(listen(sock,50)==-1)
	{
		error("Listen error:%s",strerror(errno));
		return false;
	}
	info("Listening at '%ld' ...",port);
	return true;
}

SOCKET MySocket::Accept(char**IP,long&port)
{
	SOCKET socket;
	int sin_size=sizeof(struct sockaddr_in);
	sockaddr_in client_addr={0};
	if((socket=accept(sock,(struct sockaddr *)(&client_addr),&sin_size))==INVALID_SOCKET)
	{
		warning("Accept error:%s(%ld)",strerror(errno),errno);
		return INVALID_SOCKET;
	}
	info("Got connection from '%s' at '%ld'",inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
	//if(*IP!=NULL)delete[]*IP;
	*IP=inet_ntoa(client_addr.sin_addr);
	port=client_addr.sin_port;
	return socket;
}

bool MySocket::Send(SockData ToSend)
{
	SockDataHeader SD;SD.length=sizeof(ToSend);
	int ret=send(sock,(char*)&SD,sizeof(SD),0);
	if(ret==0 || ret==SOCKET_ERROR)
	{
		error("Send data headers failed ! (%d)",ret);
		return false;
	}
	ret=send(sock,(char*)&ToSend,sizeof(ToSend),0);
	if(ret==0 || ret==SOCKET_ERROR)
	{
		error("Send datas failed ! (%d)",ret);
		return false;
	}
	return true;
}
bool MySocket::Recv(SockData&ToRecv)
{
	SockDataHeader SD;
	int ret=recv(sock,(char*)&SD,sizeof(SD),0);
	if(ret==0 || ret==SOCKET_ERROR)
	{
		error("Recv data headers failed ! (%d)",ret);
		return false;
	}
	memset(&ToRecv,0,sizeof(ToRecv));
	ret=recv(sock,(char*)&ToRecv,SD.length,0);
	if(ret==0 || ret==SOCKET_ERROR)
	{
		error("Recv datas failed ! (%d)",ret);
		return false;
	}
	return true;
}
