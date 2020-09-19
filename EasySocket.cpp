#include "EasySocket.h"

int _socket::global_init()
{
	return WSAStartup(MAKEWORD(2,2),&wsa);
}


in_addr** _socket::getipbyhost(LPCSTR hn)
{
	hostent *phst=gethostbyname(hn);
	if(phst==NULL)
		return NULL;
	return (in_addr**)phst->h_addr_list;
}


int _socket::BindPort(int port,bool dl)
{
	if(sock==NULL)
	{
		if(tcpmode)
			sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		else
			sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	}
	if(sock==SOCKET_ERROR)
		return 0;
	sAddr2.sin_family=AF_INET;
	sAddr2.sin_port=htons(port);
	sAddr2.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	int ret=bind(sock,(sockaddr*)&sAddr2,sizeof(sockaddr));
	if(ret!=0)
		return 0;
	unsigned long ul=1;
	/* Don't block this  */
	ret=ioctlsocket(sock,FIONBIO,(unsigned long *)&ul);
	if(ret==SOCKET_ERROR)
		return 0;
	ret=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&ul,sizeof(ul));
	if(ret==SOCKET_ERROR)
		return 0;
	if(tcpmode && !dl)
	{
		ret=listen(sock,128);
		if(ret!=0)
			return 0;
		FD_SET(sock,&m_readset);
		FD_SET(sock,&m_writeset);
	}
	ret=getsockname(sock,(SOCKADDR *)&sAddrBDs,&sockaddrLen);
	if(ret==SOCKET_ERROR)
		return 0;
	return sAddrBDs.sin_port;
}

_socket::_socket(bool tcpmode)
{
	this->tcpmode=tcpmode;
	FD_ZERO(&m_readset);
	FD_ZERO(&m_writeset);
	rtsock.clear();rtsockmap.clear();
	memset(&sAddrCPs,0,sizeof(sockaddr_in));
	memset(&sAddrBDs,0,sizeof(sockaddr_in));
	client_roomindex=0;
	memset(&sAddr1,0,sizeof(sockaddr_in));
	memset(&sAddr2,0,sizeof(sockaddr_in));
	sock=NULL;
	sockaddrLen=sizeof(SOCKADDR);
	memset(rvdt,0,sizeof(rvdt));
	front=rear=count=0;
	bStopRecv=true;
	hThread=NULL;
}

void _socket::CleanUp()
{
	for(int i=0;i<rtsock.size();i++)
		closesocket(rtsockmap[rtsock[i]]);
	if(sock)
		closesocket(sock);
}

_socket::~_socket()
{
	//for(int i=0;i<rtsock.size();i++)
	//	closesocket(rtsockmap[rtsock[i]]);
	if(sock)
		closesocket(sock);
	/* MMD, why am I thinking that this should work for multiple instances...... */
	//WSACleanup();
}


bool _socket::ConnectPort(ULONG ip,int port)
{
	if(sock==NULL)
	{
		if(tcpmode)
			sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		else
			sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	}
	if(sock==SOCKET_ERROR)
		return false;
	sAddr1.sin_family=AF_INET;
	sAddr1.sin_port=htons(port);
	sAddr1.sin_addr.S_un.S_addr=ip;
	sAddrCPs=sAddr1;
	/* Don't block this  */
	unsigned long ul=1;
	int ret=ioctlsocket(sock,FIONBIO,(unsigned long *)&ul);
	if(ret==SOCKET_ERROR)
		return false;
	if(tcpmode)
	{
		ret=connect(sock,(SOCKADDR *)&sAddr1,sizeof(sockaddr_in));
		if(ret!=0)
		{
			fd_set writeset;
			FD_ZERO(&writeset);
			FD_SET(sock,&writeset);
			timeval tv={10,0};
			ret=select(0,NULL,&writeset,NULL,&tv);
			if(ret<=0 || !FD_ISSET(sock,&writeset))
				return false;
		}
		FD_SET(sock,&m_readset);
		FD_SET(sock,&m_writeset);
		/* In TCP mode, this->sock is used to accept() but not recv() */
		rtsock.push_back(sAddr1);
		rtsockmap[sAddr1]=sock;
		sock=NULL;
	}
	return true;
}


int _socket::SendMsg(_socket_packet msg)
{
	_socket_packet_enc enc;
	b64_encode((const unsigned char*)&msg,sizeof(_socket_packet),(char*)&enc);
	if(tcpmode)
	{
		DWORD lasttime=GetTickCount();
		while(1)
		{
			assert(GetTickCount()-lasttime <= _PACKET_TIMEOUT);
			fd_set writeset=m_writeset;
			timeval tv={0,100*1000};
			int ret=select(0,NULL,&writeset,NULL,&tv);
			assert(ret!=SOCKET_ERROR);
			if(ret==0)
				continue;

			/* The socket may change due to reconnecting */
			if(!bStopRecv)
				EnterCriticalSection(&cs);
			assert(rtsockmap.find(sAddr1)!=rtsockmap.end());
			SOCKET sock=rtsockmap[sAddr1];
			if(!bStopRecv)
				LeaveCriticalSection(&cs);

			/* The socket will be unwritable if it's been closed */
			if(!FD_ISSET(sock,&writeset))
				continue;
			/* IMPORTANT: send() until the data are fully saved ! */
			unsigned tot=0;
			DWORD lasttime=GetTickCount();
resend:
			assert(GetTickCount()-lasttime <= _PACKET_TIMEOUT);
			ret=send(sock,(char*)&enc+tot,sizeof(_socket_packet_enc)-tot,0);
			if(ret==SOCKET_ERROR || ret==0)
			{
				assert(WSAGetLastError()==WSAEWOULDBLOCK);
				Sleep(10);
				goto resend;
			}
			else tot+=ret;
			if(tot<sizeof(_socket_packet_enc))
				goto resend;
			return ret;
		}
	}
	else
	{
		int ret=sendto(sock,(char*)&enc,sizeof(_socket_packet_enc),0,(SOCKADDR*)&sAddr1,sockaddrLen);
		return ret;
	}
}

/*sockaddr_in _socket::sAddr1={0},_socket::sAddrt={0};
SOCKET _socket::sock=NULL;
int _socket::sockaddrLen=sizeof(SOCKADDR);
_socket_packet _socket::rvd[_LOOPLIST_MAXLENGTH+1];
sockaddr_in _socket::rvdt[_LOOPLIST_MAXLENGTH+1]={0};
int _socket::front=0,_socket::rear=0,_socket::count=0;
CRITICAL_SECTION _socket::cs;
bool _socket::bStopRecv=true;*/

/* Fixed a critical BUG ! */

DWORD WINAPI _socket::LoopRecvMsg(LPVOID p)
{
	_socket*_this=(_socket*)p;
	while(1)
	{
		if(_this->bStopRecv)
			break;
		_socket_packet_enc enc;
		Sleep(50);
		if(_this->tcpmode)
		{
			fd_set readset=_this->m_readset,writeset=_this->m_writeset;
			timeval tv={0,100*1000};
			int ret=select(0,&readset,&writeset,NULL,&tv);
			assert(ret!=SOCKET_ERROR);
			if(ret==0)
				continue;
			if(_this->sock && FD_ISSET(_this->sock,&readset))
			{
				SOCKET sock=accept(_this->sock,(SOCKADDR *)&_this->sAddrt,&_this->sockaddrLen);
				if(sock!=INVALID_SOCKET)
				{
					unsigned long ul=1;
					/* Don't block this  */
					ret=ioctlsocket(sock,FIONBIO,(unsigned long *)&ul);
					assert(ret!=SOCKET_ERROR);
					FD_SET(sock,&_this->m_readset);
					FD_SET(sock,&_this->m_writeset);

					EnterCriticalSection(&_this->cs);
					_this->rtsockmap[_this->sAddrt]=sock;
					_this->rtsock.push_back(_this->sAddrt);
					LeaveCriticalSection(&_this->cs);
				}
			}
			
			for(int i=0;i<_this->rtsock.size();i++)
				if(_this->rtsockmap[_this->rtsock[i]] && FD_ISSET(_this->rtsockmap[_this->rtsock[i]],&readset))
				{
					/* IMPORTANT: recv() until the data are fully saved ! */
					unsigned tot=0;
					DWORD lasttime=GetTickCount();
rerecv:
					assert(GetTickCount()-lasttime <= _PACKET_TIMEOUT);
					if(!FD_ISSET(_this->rtsockmap[_this->rtsock[i]],&readset))
						continue;
					int ret=recv(_this->rtsockmap[_this->rtsock[i]],((char*)&enc)+tot,sizeof(_socket_packet_enc)-tot,0);
					if(ret==SOCKET_ERROR || ret==0)
					{
						/* Broken socket */
						if(WSAGetLastError()!=WSAEWOULDBLOCK)
							continue;
						Sleep(10);
						goto rerecv;
					}
					else tot+=ret;
					if(tot<sizeof(_socket_packet_enc))
						goto rerecv;

					/* The socket has been closed remotely, reconnect */
					if(ret==0)
					{
						closesocket(_this->rtsockmap[_this->rtsock[i]]);
						// TODO
						continue;
					}
					
					_socket_packet msg;
					b64_decode((const char*)&enc,sizeof(enc),(unsigned char*)&msg);

					EnterCriticalSection(&_this->cs);
					/* Overflow, throw old data away */
					if(_this->count>=_LOOPLIST_MAXLENGTH)
						_this->count=0;
					_this->count++;
					_this->rvd[_this->rear]=msg;
					/* Packets may from different clients !!!!!! */
					_this->rvdt[_this->rear]=_this->rtsock[i];
					_this->rear=(_this->rear+1)%_LOOPLIST_MAXLENGTH;
					LeaveCriticalSection(&_this->cs);
				}

		}
		else
		{
			int ret=recvfrom(_this->sock,(char*)&enc,sizeof(_socket_packet_enc),0,(SOCKADDR*)&_this->sAddrt,&_this->sockaddrLen);
			if(ret<=0)
				continue;
			_socket_packet msg;
			b64_decode((const char*)&enc,sizeof(enc),(unsigned char*)&msg);
			EnterCriticalSection(&_this->cs);
			/* Overflow, throw old data away */
			if(_this->count>=_LOOPLIST_MAXLENGTH)
				_this->count=0;
			_this->count++;
			_this->rvd[_this->rear]=msg;
			/* Packets may from different clients !!!!!! */
			_this->rvdt[_this->rear]=_this->sAddrt;
			_this->rear=(_this->rear+1)%_LOOPLIST_MAXLENGTH;
			LeaveCriticalSection(&_this->cs);
		}
	}
	return 0;
}

void _socket::StartRecvMsg()
{
	if(hThread!=NULL)
		return;
	InitializeCriticalSection(&cs);
	bStopRecv=false;
	DWORD pid;
	hThread=CreateThread(NULL,0,LoopRecvMsg,this,0,&pid);
}

void _socket::StopRecvMsg()
{
	/* IMPORTANT: TerminateThread() will cause CRITICAL_SECTION to crash !!! */
	//TerminateThread(hThread,0);
	bStopRecv=true;
	if(hThread==NULL)
		return;
	WaitForSingleObject(hThread,INFINITE);
	CloseHandle(hThread);
	hThread=NULL;
	
	DeleteCriticalSection(&cs);
}


_socket_packet _socket::RecvMsg()
{
	/* Since this function is multi-thread,
		we have to create a mutex for it, 
		though front will not be changed in LoopRecvMsg()*/
	_socket_packet sr;
	sr.type=0;
	if(count==0)
		return sr;
	EnterCriticalSection(&cs);
	count--;
	sr=rvd[front];
	/* Don't overwrite sAddr1 which is also used by SendMsg() ! */
	//sAddr1=rvdt[front];
	/* Save players' information to this variant */
	sAddresult=rvdt[front];
	front=(front+1)%_LOOPLIST_MAXLENGTH;
	LeaveCriticalSection(&cs);
	return sr;
}






