#ifndef _H_EASYSOCKET
#define _H_EASYSOCKET

// Fix the issue that the variable is beyond its definition domain.
#define for if(0);else for

#pragma comment(linker,"/STACK:36777216")

//#pragma pack(1)
#pragma once
#pragma comment(lib,"ws2_32.lib")
#include <WINSOCK2.H>
#include <windows.h>
#include <cstdio>
#include "g_card.h"
#include <map>

#define _PACKET_TIMEOUT 30000
#define _PACKET_WAITCDZ_TIMEOUT 15
#define _PACKET_WAITOUT_TIMEOUT 60
#define _PACKET_THIS_VERSION 0x60
#define _PACKET_MESSAGE_SUCCESS 0x1
#define _PACKET_MESSAGE_FAILURE 0x2
#define _PACKET_MESSAGE_OFFLINE 0xD
#define _PACKET_MESSAGE_KEEPALIVE 0xE
#define _PACKET_MESSAGE_SERVER_VER 0xF
#define _PACKET_USER_LOGIN 0x10
#define _PACKET_USER_REGISTER 0x11
#define _PACKET_USER_INFO 0x12
#define _PACKET_USER_EDITPASS 0x13
#define _PACKET_ALLROOM_INFO 0x20
#define _PACKET_ROOM_INFO 0x21
#define _PACKET_ROOM_CREATE 0x22
#define _PACKET_ROOM_JOIN 0x23
#define _PACKET_ROOM_PLAYER_READY 0x24
#define _PACKET_ROOM_PLAYER_DREADY 0x25
#define _PACKET_ROOM_LEAVE 0x26
#define _PACKET_ROOM_START 0x27
#define _PACKET_ROOM_INROOM 0x28
#define _PACKET_ROOM_RECONNECT 0x29
#define _PACKET_ROOM_CARDINFO 0x1
#define _PACKET_ROOM_CARDINFO_CDZ 0x2
#define _PACKET_ROOM_CARDINFO_CDZWAITING 0x3
#define _PACKET_ROOM_CARDINFO_SUREDZ 0x4
#define _PACKET_ROOM_CARDINFO_START 0x5
#define _PACKET_ROOM_MULTIPLE 0x6
#define _PACKET_ROOM_CARDINFO_OUT 0x7
#define _PACKET_ROOM_CARDINFO_OUTWAITING 0x8
#define _PACKET_ROOM_CARDINFO_END 0x9
#define _PACKET_ROOM_TEXT 0x10


#define _BUFFER_MAXLENGTH 512
#define _BUFFER_ONELENGTH 64

#define _BUFFER_ALLCARDLENGTH 64
#define _BUFFER_CARDLENGTH 32
struct _socket_packet_cardinfo
{
	int cnt;
	_gcard_eachcard cards[_BUFFER_CARDLENGTH];
	int bs;
};

#define _ROOMINFO_MAXNUMBER 8
struct _socket_packet_allroominfo
{
	int num;
	char name[_ROOMINFO_MAXNUMBER][_BUFFER_ONELENGTH+1];
	int playing[_ROOMINFO_MAXNUMBER];
	int maxplayer[_ROOMINFO_MAXNUMBER];
	int index[_ROOMINFO_MAXNUMBER];
};

struct _socket_packet_roominfo
{
	int maxplayer,playing;
	char players[3][_BUFFER_ONELENGTH];
	int dzs[3];
	char name[_BUFFER_ONELENGTH];
	bool ready[3];
	int mul;
};

#define _USER_TIMEOUT 125000
#define _USER_SEND_PERTIME 60000
struct _socket_packet_userinfo
{
	char name[_BUFFER_ONELENGTH+1];
	char pass[_BUFFER_MAXLENGTH+1];
	int dzs;
};

#define _SOCKET_RECVBUF 8192
struct _socket_packet_enc
{
	char buf[_SOCKET_RECVBUF];
	_socket_packet_enc(){memset(buf,0,sizeof(buf));}
};

struct _socket_packet
{
	//int id;int pid; /* Remain TODO */
	int type;int stype;
	int a;int aa;int aaa;
	union _socket_packet_union
	{
		int a;
		char b[_BUFFER_MAXLENGTH+1];

		_socket_packet_userinfo userinfo;
		_socket_packet_allroominfo allroominfo;
		_socket_packet_roominfo roominfo;
		_socket_packet_cardinfo cardinfo;
	}data;
	_socket_packet(){/*id=pid=*/type=stype=a=aa=aaa=0;memset(&data,0,sizeof(data));}
};

struct _comp_sockaddr_in
{
	bool operator ()(const sockaddr_in&a,const sockaddr_in&b)const
	{
		if(a.sin_addr.s_addr != b.sin_addr.s_addr)
			return a.sin_addr.s_addr<b.sin_addr.s_addr;
		return a.sin_port<b.sin_port;
	}
};

#define _LOOPLIST_MAXLENGTH 128

class _socket
{
public:
	_socket(bool=true);
	~_socket();
	int sockaddrLen;

	sockaddr_in sAddr1,sAddresult;
	SOCKET sock;

	/* For TCP protocol */
	std::vector<sockaddr_in>rtsock;
	std::map<sockaddr_in,SOCKET,_comp_sockaddr_in>rtsockmap;
	fd_set m_readset,m_writeset;

	bool bStopRecv;
	CRITICAL_SECTION cs;

	/*
		Initialize a socket, once
		return what WSAStartup() returns
	*/
	int global_init();


	/*
		Resolve hostname to ip address
		(LPCSTR)	-> hostname
		return an in_addr** structure
	*/
	in_addr** getipbyhost(LPCSTR);


	/*
		Bind a socket with a port
		(int)	-> port
		(bool)	-> don't listen in tcpmode (DEFAULT:false)
		return (binded port)/0(failed)
	*/
	int BindPort(int,bool=false);


	/*
		Set a socket to a server
		(ULONG)	-> ip
		(int)	-> port
		return true/false
	*/
	bool ConnectPort(ULONG,int);


	/*
		Send a packet
		(_socket_packet)	-> message
		return what sendto() returns
	*/
	int SendMsg(_socket_packet);


	/*
		Start receiving packets
	*/
	void StartRecvMsg();

	/*
		Stop receiving packets
	*/
	void StopRecvMsg();


	/*
		Receive a packet (saved in rvd[])
		return a message
	*/
	_socket_packet RecvMsg();


	/*
		Shutdown all active sockets
	*/
	void CleanUp();



private:
	bool tcpmode;
	WSADATA wsa;
	sockaddr_in sAddrt,sAddr2;
	sockaddr_in sAddrCPs,sAddrBDs;
	
	/* Save packets with room indexs */
	int client_roomindex;

	_socket_packet rvd[_LOOPLIST_MAXLENGTH+1];
	/* Fixed an excellent BUG */
	sockaddr_in rvdt[_LOOPLIST_MAXLENGTH+1];

	int front,rear,count;
	HANDLE hThread;
	static DWORD WINAPI LoopRecvMsg(LPVOID);


};

/*
Process:
	Server:
		1, global_init();
		2, BindPort();
		3, StartRecvMsg();
		4, RecvMsg();
		5, sAddr1=sAddresult; *(For better Client could work)
		6, SendMsg();
		7, StopRecvMsg();
		8, ~_socket();
	Client:
		1, global_init();
		2, BindPort(dl=true);
		3, ConnectPort();
		4, SendMsg();
		5, StartRecvMsg();
		6, RecvMsg();
		7, StopRecvMsg();
		8, ~_socket();
*/


static const char b64_table[] = {
  '5', '?', 'C', 'T', 'M', '8', 'O', 'z',
  'I', 'i', 'k', 'L', 'c', 'N', '!', 'P',
  'Q', 'p', 'S', 'D', ')', '7', 'e', 'X',
  'R', 'h', 'a', 'j', 'E', 'd', 'W', 'f',
  'g', 'Z', 'J', '%', 'K', 'l', '+', 'o',
  'n', 'Y', 's', '/', 'q', '(', 'u', 'v',
  'w', '-', 'y', 'H', '0', 't', 'm', 'U',
  '9', 'A', 'x', 'V', 'F', '4', 'B', 'G'
};

static void b64_encode(const unsigned char *src,size_t len,char*enc,size_t*encsize=NULL)
{
	int i=0;
	size_t size=0;
	unsigned char buf[4]={0};
	unsigned char tmp[3]={0};

	while(len--)
	{
		tmp[i++]=*(src++);
		if(i==3)
		{
			buf[0]=(tmp[0] & 0xfc)>>2;
			buf[1]=((tmp[0] & 0x03)<<4) + ((tmp[1] & 0xf0)>>4);
			buf[2]=((tmp[1] & 0x0f)<<2) + ((tmp[2] & 0xc0)>>6);
			buf[3]=tmp[2] & 0x3f;
			for(int i=0;i<4;i++)
				 enc[size++]=b64_table[buf[i]];
			i=0;
		}
	}

	if(i>0)
	{
		for(int j=i;j<3;j++)
			tmp[j]=0;

		buf[0]=(tmp[0] & 0xfc)>>2;
		buf[1]=((tmp[0] & 0x03)<<4) + ((tmp[1] & 0xf0)>>4);
		buf[2]=((tmp[1] & 0x0f)<<2) + ((tmp[2] & 0xc0)>>6);
		buf[3]=tmp[2] & 0x3f;

		for(int j=0;j<i+1;j++)
			enc[size++]=b64_table[buf[j]];

		while((i++)<3)
			enc[size++]='*';
	}

	enc[size]=0;
	if(encsize)
		*encsize=size;
}


static void b64_decode(const char*src,size_t len,unsigned char*dec,size_t*decsize=NULL)
{
	int i=0,j=0,l=0;
	size_t size=0;
	unsigned char buf[3]={0};
	unsigned char tmp[4]={0};

	while(len--)
	{
		if(src[j]=='*')
			break;
		if(!(isalnum(src[j]) || src[j]=='%' || src[j]=='(' || src[j]==')'
			|| src[j]=='?' || src[j]=='!' || src[j]=='+' || src[j]=='-' || src[j]=='/'))
				break;

		tmp[i++] = src[j++];

		if(i==4)
		{
			for (i=0;i<4;i++)
				for (l=0;l<64;l++)
					if(tmp[i]==b64_table[l])
					{
						tmp[i]=l;
						break;
					}

			buf[0]=(tmp[0]<<2) + ((tmp[1] & 0x30)>>4);
			buf[1]=((tmp[1] & 0xf)<<4) + ((tmp[2] & 0x3c)>>2);
			buf[2]=((tmp[2] & 0x3)<<6) + tmp[3];

			for(int i=0;i<3;i++)
				dec[size++]=buf[i];

			i=0;
		}
	}

	if(i>0)
	{
		for(int j=i;j<4;j++)
			tmp[j]=0;

		for(int j=0;j<4;j++)
			for(int l=0;l<64;l++)
				if(tmp[j]==b64_table[l])
				{
					tmp[j]=l;
					break;
				}

		buf[0]=(tmp[0]<<2) + ((tmp[1] & 0x30)>>4);
		buf[1]=((tmp[1] & 0xf)<<4) + ((tmp[2] & 0x3c)>>2);
		buf[2]=((tmp[2] & 0x3)<<6) + tmp[3];

		for (int j=0;j<i-1;j++)
			dec[size++]=buf[j];
	}

	dec[size]=0;
	if(decsize)
		*decsize=size;
}


#endif