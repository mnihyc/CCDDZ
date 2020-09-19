#pragma once
#pragma warning(disable:4786) // bug of VC6
#include "../EasySocket.h"
#include "debug.h"
#include "../g_card.h"
#include <vector>
#include <map>
#include <fstream>

std::vector<_socket_packet_userinfo>users;
//std::vector<_socket_packet_roominfo>rooms;
_socket_packet_roominfo rooms[_ROOMINFO_MAXNUMBER];
sockaddr_in rooms_sock[_ROOMINFO_MAXNUMBER][3];
int lrooms;
_socket* rooms_cs[_ROOMINFO_MAXNUMBER];

std::map<int,DWORD>online_users;

DWORD WINAPI DebugInput(LPVOID);
void SaveUsers();
void LoadUsers();
int FindUser(LPCSTR);
_socket_packet GetRoomInfo(int);

#define IsRobotName(_n_) (strcmp((_n_),"Robot0")==0 || strcmp((_n_),"Robot1")==0 || strcmp((_n_),"Robot2")==0)

_socket_packet rrvd[_ROOMINFO_MAXNUMBER][_LOOPLIST_MAXLENGTH+1];
int rfront[_ROOMINFO_MAXNUMBER],rrear[_ROOMINFO_MAXNUMBER],rcount[_ROOMINFO_MAXNUMBER];
CRITICAL_SECTION rcs[_ROOMINFO_MAXNUMBER];

struct _stor
{
	SOCKET rooms_rsock[3];
	int roomindex;
};
DWORD WINAPI RoomPlay(LPVOID);

bool bMainThreadStop;

int port;
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{	
	/* Whether CTRL_C_EVENT CTRL_BREAK_EVENT CTRL_CLOSE_EVENT CTRL_LOGOFF_EVENT CTRL_SHUTDOWN_EVENT */
	bMainThreadStop=true;
	/* Wait until the main thread exits */
	Sleep(INFINITE);
	/* Block this message */
	return TRUE;
}

int main(int argc,char**argv)
{
	SetConsoleTitle("CCDDZ --- Server");
	/* DEBUGing... */
#ifndef _DEBUG
	DEBUG::DEBUG_LEVEL=DEBUG_LEVEL_NECESSARY;
#endif
	DEBUG::warning("Server started with version 0x%x",_PACKET_THIS_VERSION);
	port=16137;
	if(argc>=2)
		port=atoi(argv[1]);
	if(port<=0 || port>=65535)
		DEBUG::fault("Invalid port range...");
	users.clear();
	memset(rooms,0,sizeof(rooms));
	lrooms=0;online_users.clear();
	memset(rooms_sock,0,sizeof(rooms_sock));
	_socket cs;
	cs.global_init();
	int ret=cs.BindPort(port);
	if(!ret)
		DEBUG::fault("bind() failed with %d",WSAGetLastError());
	DEBUG::info("Bind at port %d",port);
	cs.StartRecvMsg();
	_socket_packet greeting;
	greeting.type=_PACKET_MESSAGE_SERVER_VER;
	greeting.a=_PACKET_THIS_VERSION;
	strcpy(greeting.data.b,"Welecome to CCDDZ global common server(vDEBUG) !");
	LoadUsers();
	CreateThread(NULL,0,DebugInput,NULL,0,NULL);
	bMainThreadStop=false;
	/* Install console hook */
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE);
	for(int i=0;i<_ROOMINFO_MAXNUMBER;i++)
		InitializeCriticalSection(&rcs[i]);
	while(1)
	{
		if(bMainThreadStop)
			break;
		Sleep(100);
		_socket_packet sp,rp;
		sp=cs.RecvMsg();
		if(sp.type==0)
			continue;
		cs.sAddr1=cs.sAddresult;
		if(sp.type==_PACKET_MESSAGE_SERVER_VER)
		{
			DEBUG::info("New client connected: %s at %d with version 0x%x",inet_ntoa(cs.sAddr1.sin_addr),cs.sAddr1.sin_port,sp.a);
			cs.SendMsg(greeting);
		}
		else if(sp.type==_PACKET_USER_LOGIN)
		{
			bool fnd=false,psr=false;
			int k=FindUser(sp.data.userinfo.name);
			if(k!=-1)
			{
				fnd=true;
				psr=(strcmp(users[k].pass,sp.data.userinfo.pass)==0);
			}
			rp.type=_PACKET_USER_LOGIN;
			rp.a=_PACKET_MESSAGE_FAILURE;
			if(!fnd)
			{
				strcpy(rp.data.b,"The user doesn't exists!");
				DEBUG::warning("Login failed(user) for user <%s>",sp.data.userinfo.name);
			}
			else if(GetTickCount()-online_users[k]<_USER_TIMEOUT)
			{
				sprintf(rp.data.b,"The user has already logged in!\n\n\nIf the program exits abnormally,\nyou may wait for a timeout of %dmsec...",_USER_TIMEOUT-(GetTickCount()-online_users[k]));
				DEBUG::warning("Login failed(online) for user <%s>",sp.data.userinfo.name);
			}
			else
				if(!psr)
				{
					strcpy(rp.data.b,"The password isn't correct!");
					DEBUG::warning("Login failed(password) for user <%s>",sp.data.userinfo.name);
				}
				else
				{
					rp.a=_PACKET_MESSAGE_SUCCESS;
					rp.aa=users[k].dzs;
					strcpy(rp.data.b,"Login successfully!");
					DEBUG::info("Login successfully for user <%s>",sp.data.userinfo.name);
				}
			cs.SendMsg(rp);
		}
		else if(sp.type==_PACKET_USER_REGISTER)
		{
			rp.type=_PACKET_USER_REGISTER;
			bool fnd=(FindUser(sp.data.userinfo.name)!=-1);
			if(fnd)
			{
				rp.a=_PACKET_MESSAGE_FAILURE;
				strcpy(rp.data.b,"The user does already exist!");
				DEBUG::warning("Sign up failed for user <%s>",sp.data.userinfo.name);
			}
			else if(IsRobotName(sp.data.userinfo.name))
			{
				rp.a=_PACKET_MESSAGE_FAILURE;
				strcpy(rp.data.b,"The user can't be registered!");
				DEBUG::warning("Sign up failed for user <%s>",sp.data.userinfo.name);
			}
			else
			{
				sp.data.userinfo.dzs=10000;
				users.push_back(sp.data.userinfo);
				rp.a=_PACKET_MESSAGE_SUCCESS;
				rp.aa=10000;
				strcpy(rp.data.b,"Sign up successfully!");
				DEBUG::info("Sign up successfully for user <%s>",sp.data.userinfo.name);
			}
			cs.SendMsg(rp);
			SaveUsers();
		}
		else if(sp.type==_PACKET_ALLROOM_INFO)
		{
			rp.type=_PACKET_ALLROOM_INFO;
			rp.data.allroominfo.num=lrooms;
			int l=0;
			for(int i=0;i<_ROOMINFO_MAXNUMBER;i++)
				if(rooms[i].playing!=0)
				{
					rp.data.allroominfo.index[l]=i;
					strcpy(rp.data.allroominfo.name[l],rooms[i].name);
					rp.data.allroominfo.playing[l]=rooms[i].playing;
					rp.data.allroominfo.maxplayer[l]=rooms[i].maxplayer;
					l++;
				}
			cs.SendMsg(rp);
		}
		else if(sp.type==_PACKET_ROOM_CREATE)
		{
			rp.type=_PACKET_ROOM_CREATE;
			rp.a=_PACKET_MESSAGE_FAILURE;
			if(lrooms>=_ROOMINFO_MAXNUMBER)
			{
				strcpy(rp.data.b,"The number of rooms has increased to its maximum.");
				DEBUG::warning("The number of rooms has increased to its maximum.");
			}
			else
			{
				rp.a=_PACKET_MESSAGE_SUCCESS;
				strcpy(rp.data.b,"The room was created successfully!\n\nYou're the owner of this room.");
				int k;
				for(k=0;k<_ROOMINFO_MAXNUMBER;k++)
					if(rooms[k].playing==0)
						break;
				DEBUG::info("Room %d<%s> created by <%s> (%d players)",k,sp.data.roominfo.name,sp.data.roominfo.players[0],sp.a);
				strcpy(rooms[k].name,sp.data.roominfo.name);
				strcpy(rooms[k].players[0],sp.data.roominfo.players[0]);
				rooms[k].playing=1;rooms[k].maxplayer=sp.a;
				for(int i=1;i<=3-sp.a;i++)
				{
					sprintf(rooms[k].players[i],"Robot%d",i);
					rooms[k].dzs[i]=INT_MAX;
					rooms[k].ready[i]=true;
				}
				rooms[k].mul=2;
				lrooms++;
				/* For broadcasting room info to all the players in the room !!!!!!! */
				rooms_sock[k][0]=cs.sAddresult;
				rp.aa=k;
			}
			cs.SendMsg(rp);
		}
		else if(sp.type==_PACKET_ROOM_INFO)
		{
			rp=GetRoomInfo(sp.a);
			cs.SendMsg(rp);
		}
		else if(sp.type==_PACKET_MESSAGE_KEEPALIVE)
		{
			int s=FindUser(sp.data.b);
			if(s==-1)
				DEBUG::fault("Unknown KEEPONLINE <%s>",sp.data.b);
			online_users[s]=GetTickCount();
			DEBUG::debuginfo("Online user <%s>",sp.data.b);
		}
		else if(sp.type==_PACKET_MESSAGE_OFFLINE)
		{
			int s=FindUser(sp.data.b);
			if(s==-1)
				DEBUG::fault("Unknown OFFLINE <%s>",sp.data.b);
			online_users[s]=-1;
			DEBUG::debuginfo("Offline user <%s>",sp.data.b);
		}
		else if(sp.type==_PACKET_ROOM_JOIN)
		{
			DEBUG::info("%s joined room %d<%s>",sp.data.b,sp.a,rooms[sp.a].name);
			rp.type=_PACKET_ROOM_JOIN;
			if(rooms[sp.a].playing>0 && rooms[sp.a].playing<rooms[sp.a].maxplayer)
			{
				rooms[sp.a].playing++;
				rp=GetRoomInfo(sp.a);
				for(int i=0;i<3;i++)
					if(strlen(rooms[sp.a].players[i])==0)
					{
						strcpy(rooms[sp.a].players[i],sp.data.b);
						/* RESET rp !!! */
						rp=GetRoomInfo(sp.a);
						rooms_sock[sp.a][i]=cs.sAddresult;
						/* Broadcast this message to other players in this room */
						sockaddr_in sAddrb=cs.sAddr1;
						for(int p=0;p<3;p++)
							if(p!=i && strlen(rooms[sp.a].players[p])!=0)
							{
								if(IsRobotName(rooms[sp.a].players[p]))
									continue;
								/* For other players, they just need to update their room info !!! */
								rp.type=_PACKET_ROOM_INFO;
								DEBUG::debuginfo("Broadcast(join) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
								cs.sAddr1=rooms_sock[sp.a][p];
								cs.SendMsg(rp);
							}
						rp.type=_PACKET_ROOM_TEXT;
						sprintf(rp.data.b,"player joined");
						for(int p=0;p<3;p++)
							if(p!=i && strlen(rooms[sp.a].players[p])!=0)
							{
								if(IsRobotName(rooms[sp.a].players[p]))
									continue;
								DEBUG::debuginfo("Broadcast(TEXT) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
								cs.sAddr1=rooms_sock[sp.a][p];
								cs.SendMsg(rp);
							}
						cs.sAddr1=sAddrb;
						break;
					}
					/* For this player, it needs the response !!! */
					rp.type=_PACKET_ROOM_JOIN;
			}
			else if(rooms[sp.a].playing==rooms[sp.a].maxplayer)
			{
				rp.a=_PACKET_MESSAGE_FAILURE;
				strcpy(rp.data.b,"This room is currently full!");
			}
			else if(rooms[sp.a].playing==0)
			{
				rp.a=_PACKET_MESSAGE_FAILURE;
				strcpy(rp.data.b,"This room doesn't exists!");
			}
			cs.SendMsg(rp);
		}
		/* For easier code, transfer _PACKET_ROOM_INFO directly for each change */
		else if(sp.type==_PACKET_ROOM_PLAYER_READY)
		{
			DEBUG::debuginfo("<%s> in room %d ready",rooms[sp.a].players[sp.aa],sp.a);
			rooms[sp.a].ready[sp.aa]=true;
			rp=GetRoomInfo(sp.a);
			cs.SendMsg(rp);
			/* Broadcast this message to other players in this room */
			sockaddr_in sAddrb=cs.sAddr1;
			for(int p=0;p<3;p++)
				if(p!=sp.aa && strlen(rooms[sp.a].players[p])!=0)
				{
					if(IsRobotName(rooms[sp.a].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(ready) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
					cs.sAddr1=rooms_sock[sp.a][p];
					cs.SendMsg(rp);
				}
			cs.sAddr1=sAddrb;

			/* When 3 players are all ready, room starts */
			if(rooms[sp.a].playing==rooms[sp.a].maxplayer && rooms[sp.a].ready[0] && rooms[sp.a].ready[1] && rooms[sp.a].ready[2])
			{
				rp=GetRoomInfo(sp.a);
				/* Delay until the last player is rendered */
				Sleep(10);
				rp.type=_PACKET_ROOM_START;
				sockaddr_in sAddrb=cs.sAddr1;
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[sp.a].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(start) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
					cs.sAddr1=rooms_sock[sp.a][p];
					cs.SendMsg(rp);
				}
				cs.sAddr1=sAddrb;
				_stor stor;
				stor.roomindex=sp.a;
				for(int i=0;i<3;i++)
					if(!IsRobotName(rooms[sp.a].players[i]))
						stor.rooms_rsock[i]=cs.rtsockmap[rooms_sock[sp.a][i]];
				CreateThread(NULL,0,RoomPlay,(LPVOID)&stor,0,NULL);
				/* The state will be restored when this thread ends. */
				Sleep(200);
			}
		}
		else if(sp.type==_PACKET_ROOM_PLAYER_DREADY)
		{
			DEBUG::debuginfo("<%s> in room %d dready",rooms[sp.a].players[sp.aa],sp.a);
			rooms[sp.a].ready[sp.aa]=false;
			rp=GetRoomInfo(sp.a);
			cs.SendMsg(rp);
			/* Broadcast this message to other players in this room */
			sockaddr_in sAddrb=cs.sAddr1;
			for(int p=0;p<3;p++)
				if(p!=sp.aa && strlen(rooms[sp.a].players[p])!=0)
				{
					if(IsRobotName(rooms[sp.a].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(dready) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
					cs.sAddr1=rooms_sock[sp.a][p];
					cs.SendMsg(rp);
				}
			cs.sAddr1=sAddrb;
		}
		else if(sp.type==_PACKET_ROOM_LEAVE)
		{
			DEBUG::info("<%s> leaved room %d",sp.data.b,sp.a);
			rooms[sp.a].playing--;
			memset(rooms[sp.a].players[sp.aa],0,sizeof rooms[sp.a].players[sp.aa]);
			rooms[sp.a].ready[sp.aa]=false;
			if(rooms[sp.a].playing==0)
			{
				memset(&rooms[sp.a],0,sizeof rooms[sp.a]);
				rcount[sp.a]=rfront[sp.a]=rrear[sp.a]=0;
				lrooms--;
				DEBUG::info("room %d deleted",sp.a);
			}
			else
			{
				rp=GetRoomInfo(sp.a);
				/* Broadcast this message to other players in this room */
				sockaddr_in sAddrb=cs.sAddr1;
				for(int p=0;p<3;p++)
					if(p!=sp.aa && strlen(rooms[sp.a].players[p])!=0)
					{
						if(IsRobotName(rooms[sp.a].players[p]))
							continue;
						DEBUG::debuginfo("Broadcast(leave) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
						cs.sAddr1=rooms_sock[sp.a][p];
						cs.SendMsg(rp);
					}
				rp.type=_PACKET_ROOM_TEXT;
				sprintf(rp.data.b,"player leaved");
				for(int p=0;p<3;p++)
					if(p!=sp.aa && strlen(rooms[sp.a].players[p])!=0)
					{
						if(IsRobotName(rooms[sp.a].players[p]))
							continue;
						DEBUG::debuginfo("Broadcast(TEXT) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
						cs.sAddr1=rooms_sock[sp.a][p];
						cs.SendMsg(rp);
					}
				cs.sAddr1=sAddrb;
				/* Don't reply to the user which exits */
				//cs.SendMsg(rp);
			}
			/* We don't care about the value of rooms_sock[] because it's not necessary here. */

			/* We don't reply if the room is deleted */
		}
		else if(sp.type==_PACKET_ROOM_INROOM && rooms[sp.a].playing==rooms[sp.a].maxplayer)
		{
			/* This is from a reconnected client, update its socket */
			if(sp.stype==_PACKET_ROOM_RECONNECT)
			{
				DEBUG::debuginfo("Client reconnected: %s at %d in room %d",inet_ntoa(cs.sAddr1.sin_addr),cs.sAddr1.sin_port,sp.a);
				assert(rooms_cs[sp.a]);
				if(!rooms_cs[sp.a]->bStopRecv)
					EnterCriticalSection(&rooms_cs[sp.a]->cs);
				FD_ZERO(&rooms_cs[sp.a]->m_writeset);
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[sp.a].players[p]))
						continue;
					if(!rooms_cs[sp.a]->bStopRecv)
					{
						rooms_cs[sp.a]->rtsockmap[rooms_sock[sp.a][p]]=cs.rtsockmap[rooms_sock[sp.a][p]];
						FD_SET(cs.rtsockmap[rooms_sock[sp.a][p]],&rooms_cs[sp.a]->m_writeset);
					}
				}
				if(!rooms_cs[sp.a]->bStopRecv)
					LeaveCriticalSection(&rooms_cs[sp.a]->cs);
			}
			else
			{
				EnterCriticalSection(&rcs[sp.a]);
				if(rcount[sp.a]>=_LOOPLIST_MAXLENGTH) //Overflow, throw old data away
					rcount[sp.a]=0;
				rcount[sp.a]++;
				rrvd[sp.a][rrear[sp.a]]=sp;
				rrear[sp.a]=(rrear[sp.a]+1)%_LOOPLIST_MAXLENGTH;
				LeaveCriticalSection(&rcs[sp.a]);
			}
		}
		else if(sp.type==_PACKET_USER_EDITPASS)
		{
			int k=FindUser(sp.data.userinfo.name);
			assert(k!=-1);
			memcpy(users[k].pass,sp.data.userinfo.pass,sizeof(users[k].pass));
			DEBUG::info("Password change successfully for user <%s>",sp.data.userinfo.name);
		}
		else
			DEBUG::warning("Unknow packet from the client (type=0x%x)",sp.type);





	}

	DEBUG::warning("The program is going to exit...");
	cs.StopRecvMsg();
	cs.CleanUp();
	cs.~_socket();
	WSACleanup();
	for(int i=0;i<_ROOMINFO_MAXNUMBER;i++)
		DeleteCriticalSection(&rcs[i]);
	ExitProcess(0);

	return 0;
}


DWORD WINAPI DebugInput(LPVOID p)
{
	char buf[1024+1]={0},buf1[1024+1]={0};
	while(1)
	{
		memset(buf,0,sizeof(buf));
		memset(buf1,0,sizeof(buf1));
		scanf("%1024s",buf);
		if(strcmp(buf,"user")==0)
		{
			scanf("%1024s",buf);
			if(strcmp(buf,"show")==0)
			{
				printf("--------------Users------------\n");
				for(int i=0;i<users.size();i++)
					printf("Index: %d\nUsername: %s\nPassword: %s\ndzs: %d\n\n",i,users[i].name,users[i].pass,users[i].dzs);
				printf("\n");
			}
			else if(strcmp(buf,"showi")==0)
			{
				int index;scanf("%d",&index);
				if(index>=0 && index<users.size())
				{
					printf("--------------Users------------\n");
					printf("Index: %d\nUsername: %s\nPassword: %s\ndzs: %d\n\n",index,users[index].name,users[index].pass,users[index].dzs);
					printf("\n");
				}
				else DEBUG::warning("User %d doesn't exist !",index);
			}
			else if(strcmp(buf,"edit")==0)
			{
				scanf("%1024s",buf);
				int index=atoi(buf);
				scanf("%1024s",buf);
				std::cin.getline(buf1,_BUFFER_ONELENGTH);
				if(strcmp(buf,"name")==0)
				{
					DEBUG::info("Change username %d<%s> to <%s> successfully!",index,users[index].name,buf1);
					strcpy(users[index].name,buf1);
				}
				else if(strcmp(buf,"pass")==0)
				{
					DEBUG::info("Change userpass %d<%s> to <%s> successfully!",index,users[index].pass,buf1);
					strcpy(users[index].pass,buf1);
				}
				else if(strcmp(buf,"dzs")==0)
				{
					DEBUG::info("Change userdzs %d(%d) to (%d) successfully!",index,users[index].dzs,atoi(buf1));
					users[index].dzs=atoi(buf1);
				}
				else
					DEBUG::warning("Unknown operation <%s> !",buf);
				SaveUsers();
			}
			else if(strcmp(buf,"add")==0)
			{
				int dzs=10000;scanf("%d\n",buf);
				std::cin.getline(buf,_BUFFER_ONELENGTH);
				std::cin.getline(buf1,_BUFFER_ONELENGTH);
				_socket_packet_userinfo nwuser;
				strcpy(nwuser.name,buf);
				strcpy(nwuser.pass,buf1);
				nwuser.dzs=dzs;
				users.push_back(nwuser);
				SaveUsers();
				DEBUG::info("Add user %d<%s,%s,%d> successfully!",users.size()-1,nwuser.name,nwuser.pass,nwuser.dzs);
			}
			else if(strcmp(buf,"del")==0)
			{
				int index;scanf("%d",&index);
				if(index<users.size() && index>=0)
				{
					DEBUG::info("Deleted user %d<%s> successfully !",index,users[index].name);
					users.erase(users.begin()+index);
					SaveUsers();
				}
				else DEBUG::warning("User %d doesn't exist !",index);
			}
			else
				printf("user help\n"
				"\tuser show\n"
				"\tuser showi [index]\n"
				"\t\tuser showi 0\n"
				"\tuser edit [index] [type] [string]\n"
				"\t\tuser edit 0 name nwusername\n"
				"\t\tuser edit 0 pass nwpassword\n"
				"\t\tuser edit 1 dzs 1234\n"
				"\tuser add [dzs]\\n[username]\\n[password]\\n\n"
				"\t\tuser add 10000 \\n username \\n password \\n\n"
				"\tuser del [index]\n"
				"\t\tuser del 0\n\n"
				);
		}
		else if(strcmp(buf,"room")==0)
		{
			scanf("%s",buf);
			if(strcmp(buf,"show")==0)
			{
				printf("------------ROOM(%d)-----------\n\n",lrooms);
				for(int i=0;i<_ROOMINFO_MAXNUMBER;i++)
					if(rooms[i].playing != 0)
						printf("Index: %d\nName: %s\nPlayers: \n%d:\t<%s>\n%d:\t<%s>\n%d:\t<%s>\n\n",i,rooms[i].name,FindUser(rooms[i].players[0]),rooms[i].players[0],FindUser(rooms[i].players[1]),rooms[i].players[1],FindUser(rooms[i].players[2]),rooms[i].players[2]);
				printf("\n\n");
			}
			else printf(
				"\troom help\n"
				"\troom show\n\n"
				);
		}
		else if(strcmp(buf,"exit")==0 || strcmp(buf,"quit")==0 || strcmp(buf,"stop")==0)
			bMainThreadStop=true;
		else
			printf(
			"\tuser help\n"
			"\troom help\n"
			"\texit\n\n"
			);



	}
	return 0;
}


void SaveUsers()
{
	DEBUG::info("Saving user.db to the harddisk......");
	std::ofstream os("users.db",std::ios::binary);
	int size1=users.size();
	os.write((char*)&size1,sizeof(size1));
	os.write((char*)&users[0],size1*sizeof(_socket_packet_userinfo));
	os.close(); 
}

void LoadUsers()
{
	DEBUG::info("Loading user.db from the harddisk......");
	std::ifstream is("users.db",std::ios::binary);
	if(is.fail())
		return;
	int size1;
	is.read((char*)&size1,sizeof(size1));
	users.resize(size1);
	is.read((char*)&users[0],size1*sizeof(_socket_packet_userinfo)); 
}


int FindUser(LPCSTR name)
{
	if(IsRobotName(name))
		return -2;
	for(int i=0;i<users.size();i++)
		if(strcmp(users[i].name,name)==0)
			return i;
	return -1;
}


_socket_packet GetRoomInfo(int index)
{
	_socket_packet rp;
	memset(&rp,0,sizeof(rp));
	rp.type=_PACKET_ROOM_INFO;
	rp.data.roominfo=rooms[index];
	for(int i=0;i<3;i++)
	{
		int k=FindUser(rp.data.roominfo.players[i]);
		if(k==-1)
			rp.data.roominfo.dzs[i]=0;
		else if(k==-2)
			rp.data.roominfo.dzs[i]=INT_MAX;
		else
			rp.data.roominfo.dzs[i]=users[k].dzs;

	}
	return rp;
}

_socket_packet RoomPlayRecv(int roomindex)
{
	_socket_packet sr;
	if(bMainThreadStop)
		return sr;
	if(rcount[roomindex]>0)
	{
		EnterCriticalSection(&rcs[roomindex]);
		rcount[roomindex]--;
		sr=rrvd[roomindex][rfront[roomindex]];
		rfront[roomindex]=(rfront[roomindex]+1)%_LOOPLIST_MAXLENGTH;
		LeaveCriticalSection(&rcs[roomindex]);
		return sr;
	}
	return sr;
}

DWORD WINAPI RoomPlay(LPVOID param)
{
	_stor stor=*(_stor*)param;
	int roomindex=stor.roomindex;
	DEBUG::debuginfo("Room %d begins.",roomindex);
	sockaddr_in slocal;
	rooms_cs[roomindex]=new _socket;
	_socket cs=*rooms_cs[roomindex];
	cs.ConnectPort(inet_addr("127.0.0.1"),port);
	slocal=cs.sAddr1;
	/* Initialize sockets manually */
	/* Only SendMsg() is called here */
	for(int i=0;i<3;i++)
	{
		if(IsRobotName(rooms[roomindex].players[i]))
			continue;
		cs.rtsockmap[rooms_sock[roomindex][i]]=stor.rooms_rsock[i];
		FD_SET(stor.rooms_rsock[i],&cs.m_writeset);
	}

	_socket_packet sp,rp;
	rp.type=_PACKET_ROOM_INROOM;
	rp.stype=_PACKET_ROOM_TEXT;
	rp.a=roomindex;
	strcpy(rp.data.b,"Room begins");
	for(int p=0;p<3;p++)
	{
		if(IsRobotName(rooms[roomindex].players[p]))
			continue;
		DEBUG::debuginfo("Broadcast(TEXT) to <%s> in room %d",rooms[roomindex].players[p],roomindex);
		cs.sAddr1=rooms_sock[roomindex][p];
		cs.SendMsg(rp);
	}

reselect:
	Sleep(500);
	sp.type=_PACKET_ROOM_INROOM;
	sp.stype=_PACKET_ROOM_CARDINFO;
	sp.a=roomindex;
	/* Send cards */
	_gcard_eachcard _cards[_BUFFER_ALLCARDLENGTH];
	_gcard_eachcard scards[3][_BUFFER_CARDLENGTH];
	int lcards=_gcard_ShuffleAllCards(_cards);
	int nlcds=0;
	_socket_packet_cardinfo cards[3]={0};
	for(int p=0;p<3;p++)
	{
		cards[p].cnt=17;
		for(int i=1;i<=17;i++)
			scards[p][i-1]=_cards[nlcds++];
		std::sort(&scards[p][0],&scards[p][17]);
		for(int i=1;i<=17;i++)
			cards[p].cards[i]=scards[p][i-1];
	}
	_gcard_eachcard dzcards[3];
	for(int i=1;i<=3;i++)
		dzcards[i-1]=_cards[nlcds++];
	std::sort(&dzcards[0],&dzcards[3]);
	for(int i=0;i<3;i++)
	{
		sp.aa=i;
		sp.data.cardinfo=cards[i];
		for(int p=0;p<3;p++)
		{
			if(IsRobotName(rooms[roomindex].players[p]))
				continue;
			DEBUG::debuginfo("Broadcast(CARDINFO %d) to <%s> in room %d",i,rooms[sp.a].players[p],sp.a);
			cs.sAddr1=rooms_sock[sp.a][p];
			cs.SendMsg(sp);
		}
	}


	memset(&sp,0,sizeof(sp));
	sp.type=_PACKET_ROOM_INROOM;
	sp.stype=_PACKET_ROOM_CARDINFO_CDZWAITING;
	sp.a=roomindex;
	srand(time(NULL));rand();rand();rand();
	int a[3]={0,1,2};
	std::random_shuffle(&a[0],&a[3]);
	std::random_shuffle(&a[0],&a[3]);
	std::random_shuffle(&a[0],&a[3]);
	sp.aa=a[rand()%3];
	for(int p=0;p<3;p++)
	{
		if(IsRobotName(rooms[roomindex].players[p]))
			continue;
		DEBUG::debuginfo("Broadcast(CDZW-0) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
		cs.sAddr1=rooms_sock[sp.a][p];
		cs.SendMsg(sp);
	}
	if(IsRobotName(rooms[roomindex].players[sp.aa]))
	{
		// I send to myself (wwwwww
		_socket_packet rp;
		rp.type=_PACKET_ROOM_INROOM;
		rp.stype=_PACKET_ROOM_CARDINFO_CDZ;
		rp.a=roomindex;
		rp.aa=sp.aa;
		rp.data.a=0;
		cs.sAddr1=slocal;
		cs.SendMsg(rp);
	}

	int bcdz[4]={0};
	int ncdz=0;
	while(1)
	{
		while(1)
		{
			Sleep(100);
			sp=RoomPlayRecv(roomindex);
			if(sp.type==0)
				continue;
			break;
			/* TMSB, remember to break !!! */
		}
		rp=sp;
		if(sp.stype==_PACKET_ROOM_CARDINFO_CDZ)
		{
			/* sp.a is roomindex, use sp.aa instead */
			/* Broadcast to all players, including the sender */
			for(int p=0;p<3;p++)
			{
				if(IsRobotName(rooms[roomindex].players[p]))
					continue;
				DEBUG::debuginfo("Broadcast(CDZ) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
				cs.sAddr1=rooms_sock[sp.a][p];
				cs.SendMsg(rp);
			}
			ncdz++;
			if(sp.data.a==1)
			{
				rooms[sp.a].mul*=(rand()%2+2);
				rp=GetRoomInfo(sp.a);
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_MULTIPLE;
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[roomindex].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(MUL) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
					cs.sAddr1=rooms_sock[sp.a][p];
					cs.SendMsg(rp);
				}
				rp=sp;
				if(bcdz[sp.aa])
				{
					/* This is DZ who called twice ! */
					bcdz[sp.aa]=ncdz;
					break;
				}
				bcdz[sp.aa]=ncdz;
				Sleep(100);
			}
			else
				bcdz[sp.aa]=-1;
			int cnt=0,ncnt=0;
			for(int k=0;k<3;k++)
				cnt+=(bcdz[k]>0),ncnt+=(bcdz[k]<0);
			if(cnt==1 && ncnt==2)
				break;
			if(ncdz>=3 && cnt==0)
			{
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_TEXT;
				rp.a=roomindex;
				strcpy(rp.data.b,"No one calls!");
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[roomindex].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(TEXT) to <%s> in room %d",rooms[roomindex].players[p],roomindex);
					cs.sAddr1=rooms_sock[roomindex][p];
					cs.SendMsg(rp);
				}
				Sleep(100);
				goto reselect;
			}
			do
			{
				rp.aa=(rp.aa+1)%3;
			}while(bcdz[rp.aa]<0);
			rp.stype=_PACKET_ROOM_CARDINFO_CDZWAITING;
			for(int p=0;p<3;p++)
			{
				if(IsRobotName(rooms[roomindex].players[p]))
					continue;
				DEBUG::debuginfo("Broadcast(CDZW) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
				cs.sAddr1=rooms_sock[sp.a][p];
				cs.SendMsg(rp);
			}
			if(IsRobotName(rooms[roomindex].players[rp.aa]))
			{
				// I send to myself (wwwwww
				_socket_packet rrp;
				rrp.type=_PACKET_ROOM_INROOM;
				rrp.stype=_PACKET_ROOM_CARDINFO_CDZ;
				rrp.a=roomindex;
				rrp.aa=rp.aa;
				rrp.data.a=0;
				cs.sAddr1=slocal;
				cs.SendMsg(rrp);
			}
		}
		else
			DEBUG::warning("Unknow packet from the client (room=%d, type=0x%x)",roomindex,sp.type);
	}
	int pdz=0,mxbcdz=0;
	for(int k=0;k<3;k++)
		if(bcdz[k]>mxbcdz)
		{
			pdz=k;
			mxbcdz=bcdz[k];
		}
	memset(&rp,0,sizeof(rp));
	rp.type=_PACKET_ROOM_INROOM;
	rp.stype=_PACKET_ROOM_CARDINFO_SUREDZ;
	rp.a=roomindex;
	rp.aa=pdz;
	rp.data.cardinfo.cnt=3;
	for(int i=0;i<3;i++)
		rp.data.cardinfo.cards[i+1]=dzcards[i];
	for(int p=0;p<3;p++)
	{
		if(IsRobotName(rooms[roomindex].players[p]))
			continue;
		DEBUG::debuginfo("Broadcast(SDZ %d) to <%s> in room %d",pdz,rooms[rp.a].players[p],rp.a);
		cs.sAddr1=rooms_sock[rp.a][p];
		cs.SendMsg(rp);
	}
	for(int i=0;i<3;i++)
		scards[pdz][17+i]=dzcards[i];
	std::sort(&scards[pdz][0],&scards[pdz][20]);
	for(int i=1;i<=20;i++)
		cards[pdz].cards[i]=scards[pdz][i-1];
	cards[pdz].cnt=20;
	memset(&rp,0,sizeof(rp));
	rp.type=_PACKET_ROOM_INROOM;
	rp.stype=_PACKET_ROOM_CARDINFO;
	rp.a=roomindex;
	for(int i=0;i<3;i++)
	{
		rp.aa=i;
		rp.data.cardinfo=cards[i];
		for(int p=0;p<3;p++)
		{
			if(IsRobotName(rooms[roomindex].players[p]))
				continue;
			DEBUG::debuginfo("Broadcast(CARDINFO %d) to <%s> in room %d",i,rooms[rp.a].players[p],rp.a);
			cs.sAddr1=rooms_sock[rp.a][p];
			cs.SendMsg(rp);
		}
	}
	for(int i=0;i<3;i++)
		if(dzcards[i].GetCardLevel()>=18)
			rooms[roomindex].mul*=(rand()%2+2);
	if(dzcards[0]==dzcards[1] && dzcards[1]==dzcards[2])
		rooms[roomindex].mul*=(rand()%2+2);
	if(dzcards[0].GetCardLevel()==dzcards[1].GetCardLevel()-1 && dzcards[1].GetCardLevel()==dzcards[2].GetCardLevel()-1)
		rooms[roomindex].mul*=(rand()%2+2);
	if(dzcards[0].color==dzcards[1].color && dzcards[1].color==dzcards[2].color)
		rooms[roomindex].mul*=(rand()%2+2);
	rp=GetRoomInfo(roomindex);
	rp.type=_PACKET_ROOM_INROOM;
	rp.stype=_PACKET_ROOM_MULTIPLE;
	for(int p=0;p<3;p++)
	{
		if(IsRobotName(rooms[roomindex].players[p]))
			continue;
		DEBUG::debuginfo("Broadcast(MUL) to <%s> in room %d",rooms[sp.a].players[p],roomindex);
		cs.sAddr1=rooms_sock[roomindex][p];
		cs.SendMsg(rp);
	}

	Sleep(200);
	memset(&rp,0,sizeof(rp));
	rp.type=_PACKET_ROOM_INROOM;
	rp.stype=_PACKET_ROOM_CARDINFO_START;
	rp.a=roomindex;
	for(int p=0;p<3;p++)
	{
		if(IsRobotName(rooms[roomindex].players[p]))
			continue;
		DEBUG::debuginfo("Broadcast(CSTART) to <%s> in room %d",rooms[rp.a].players[p],rp.a);
		cs.sAddr1=rooms_sock[rp.a][p];
		cs.SendMsg(rp);
	}
	Sleep(200);
	memset(&rp,0,sizeof(rp));
	rp.type=_PACKET_ROOM_INROOM;
	rp.stype=_PACKET_ROOM_CARDINFO_OUTWAITING;
	rp.a=roomindex;
	rp.aa=pdz;
	rp.aaa=1;
	for(int p=0;p<3;p++)
	{
		if(IsRobotName(rooms[roomindex].players[p]))
			continue;
		DEBUG::debuginfo("Broadcast(COUTW-0) to <%s> in room %d",rooms[rp.a].players[p],rp.a);
		cs.sAddr1=rooms_sock[rp.a][p];
		cs.SendMsg(rp);
	}
	
	int turn=pdz,undcnt=0;
	while(1)
	{
		while(1)
		{
			Sleep(100);
			sp=RoomPlayRecv(roomindex);
			if(sp.type>0)
				break;
		}
		rp=sp;
		if(sp.stype==_PACKET_ROOM_CARDINFO_OUT)
		{
			assert(sp.aa==turn);
			turn=(turn+1)%3;
			/* Broadcast to all players, including the sender */
			for(int p=0;p<3;p++)
			{
				if(IsRobotName(rooms[roomindex].players[p]))
					continue;
				DEBUG::debuginfo("Broadcast(OUT %d) to <%s> in room %d",sp.aa,rooms[sp.a].players[p],sp.a);
				cs.sAddr1=rooms_sock[sp.a][p];
				cs.SendMsg(rp);
			}
			/* Record "Skip" rounds */
			if(sp.data.cardinfo.cnt==0)
				undcnt++;
			else
				undcnt=0;
			cards[sp.aa].cnt=_gcard_RemoveCardsFrom(cards[sp.aa].cards,cards[sp.aa].cnt,sp.data.cardinfo.cards,sp.data.cardinfo.cnt);
			Sleep(100);
			if(_gcard_GetCardsLevel(sp.data.cardinfo.cards,sp.data.cardinfo.cnt)>=2)
			{
				rooms[roomindex].mul*=(_gcard_GetCardsLevel(sp.data.cardinfo.cards,sp.data.cardinfo.cnt)+rand()%2);
				rp=GetRoomInfo(roomindex);
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_MULTIPLE;
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[roomindex].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(MUL) to <%s> in room %d",rooms[sp.a].players[p],roomindex);
					cs.sAddr1=rooms_sock[roomindex][p];
					cs.SendMsg(rp);
				}
			}
			Sleep(100);
			/* The room ends */
			if(cards[sp.aa].cnt==0)
			{
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_TEXT;
				rp.a=roomindex;
				strcpy(rp.data.b,"Room ends");
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[roomindex].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(TEXT) to <%s> in room %d",rooms[roomindex].players[p],roomindex);
					cs.sAddr1=rooms_sock[roomindex][p];
					cs.SendMsg(rp);
				}

				Sleep(100);
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_TEXT;
				rp.a=roomindex;
				int adzs=rooms[roomindex].mul*(rand()%50+50);
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[roomindex].players[p]))
						continue;
					/* Whether the winner is DZ or not */
					if(sp.aa==pdz)
						users[FindUser(rooms[roomindex].players[p])].dzs+=(p==sp.aa?2:-1)*adzs;
					else
						users[FindUser(rooms[roomindex].players[p])].dzs+=(p==pdz?-2:1)*adzs;
					sprintf(rp.data.b,"dzs+/- %d",adzs);
					DEBUG::debuginfo("Broadcast(TEXT) to <%s> in room %d",rooms[roomindex].players[p],roomindex);
					cs.sAddr1=rooms_sock[roomindex][p];
					cs.SendMsg(rp);
				}

				Sleep(100);
				rp=GetRoomInfo(sp.a);
				rp.type=_PACKET_ROOM_INROOM;
				rp.stype=_PACKET_ROOM_CARDINFO_END;
				/* Broadcast the end of the room to all players */
				for(int p=0;p<3;p++)
				{
					if(IsRobotName(rooms[roomindex].players[p]))
						continue;
					DEBUG::debuginfo("Broadcast(END) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
					cs.sAddr1=rooms_sock[sp.a][p];
					cs.SendMsg(rp);
				}
				break;
			}
			/* Next round */
			rp.stype=_PACKET_ROOM_CARDINFO_OUTWAITING;
			rp.aa=turn;
			rp.aaa=(undcnt==2);
			for(int p=0;p<3;p++)
			{
				if(IsRobotName(rooms[roomindex].players[p]))
					continue;
				DEBUG::debuginfo("Broadcast(OUTW %d) to <%s> in room %d",rp.aa,rooms[sp.a].players[p],sp.a);
				cs.sAddr1=rooms_sock[sp.a][p];
				cs.SendMsg(rp);
			}
			if(IsRobotName(rooms[roomindex].players[rp.aa]))
			{
				// I send to myself again (wwwwww
				_socket_packet rrp;
				rrp.type=_PACKET_ROOM_INROOM;
				rrp.stype=_PACKET_ROOM_CARDINFO_OUT;
				rrp.a=roomindex;
				rrp.aa=rp.aa;
				rrp.data.cardinfo.cnt=0;
				cs.sAddr1=slocal;
				cs.SendMsg(rrp);
			}
		}
		else
			DEBUG::warning("Unknow packet from the client (room=%d, type=0x%x)",roomindex,sp.type);
	}
	
	Sleep(100);
	/* Clear ready states for the next round */
	memset(&rooms[sp.a].ready[0],0,sizeof(rooms[sp.a].ready));
	/* Restore the Robot's state */
	for(int i=1;i<=3-rooms[sp.a].maxplayer;i++)
	{
		sprintf(rooms[sp.a].players[i],"Robot%d",i);
		rooms[sp.a].dzs[i]=INT_MAX;
		rooms[sp.a].ready[i]=true;
	}
	rooms[sp.a].mul=rand()%2+2;
	/* Send the original room information */
	rp=GetRoomInfo(sp.a);
	for(int p=0;p<3;p++)
	{
		if(IsRobotName(rooms[roomindex].players[p]))
			continue;
		DEBUG::debuginfo("Broadcast(INFO) to <%s> in room %d",rooms[sp.a].players[p],sp.a);
		cs.sAddr1=rooms_sock[sp.a][p];
		cs.SendMsg(rp);
	}
	
	
	DEBUG::debuginfo("Room %d ends.",roomindex);

	delete rooms_cs[roomindex];
	rooms_cs[roomindex]=NULL;
	return 0;
}



