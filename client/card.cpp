#include "card.h"


_card::_card(_input*gi)
{
	this->gi=gi;
	this->dontbuff=false;
}

/* 虽然很多地方都溢出了，但是我不管，能正常运行就可以了（ */
/* 写的比较乱 */
/* 太鬼畜了这（原）代码，改得（现）比较能看了一点 */
void _card::RenderPlayerFramework(_socket_packet_roominfo bp,/*LPCSTR username*/int pindex,bool showready)
{
	if(!this->dontbuff)
		gi->StartOutputBuffer();
	/* Show room name */
	SMALL_RECT rc=_input_window;
	rc.Bottom=rc.Top;
	rc.Left++;rc.Right--;
	rc.Top++;rc.Bottom++;
	gi->ClearRect(rc);
	gi->ShowOneLineText(bp.name,0,rc);

	/* Show players */
	int k;
	/*for(k=0;k<3;k++)
		if(strcmp(bp.players[k],username)==0)
			break;
	assert(k<3);*/
	k=pindex;
	/* Me */
	char buf[_BUFFER_ONELENGTH]={0};
	memset(buf,0,sizeof(buf));
	sprintf(buf,"dzs: %d",bp.dzs[k]);
	rc=_input_window;
	rc.Left+=25;rc.Right-=25;
	rc.Top=rc.Bottom-=2;
	gi->ClearRect(rc);
	gi->ShowOneLineText(bp.players[k],0,rc);
	rc.Top++;rc.Bottom++;
	gi->ClearRect(rc);
	gi->ShowOneLineText(buf,0,rc);

	/* RU */
	k=(k+1)%3;
	/* Clear the rectangle first ! */
	rc=_input_window;
	rc.Bottom=rc.Top+=4;
	rc.Left=rc.Right-2-28;
	rc.Bottom--;rc.Top--;
	rc.Left++;rc.Right--;
	gi->ClearRect(rc);
	rc.Left--;rc.Right++;
	rc.Top++;rc.Bottom++;
	rc.Left++;rc.Right--;
	gi->ClearRect(rc);
	rc.Left--;rc.Right++;
	rc.Top--;rc.Bottom--;
	if(strlen(bp.players[k])!=0)
	{
		memset(buf,0,sizeof(buf));
		sprintf(buf,"dzs: %d",bp.dzs[k]);
		gi->ShowOneLineText(bp.players[k],0,rc);
		rc.Top++;rc.Bottom++;
		gi->ShowOneLineText(buf,0,rc);
	}

	/* LU */
	k=(k+1)%3;
	rc=_input_window;
	rc.Bottom=rc.Top+=4;
	rc.Right=rc.Left+2+28;
	rc.Bottom--;rc.Top--;
	rc.Left++;rc.Right--;
	gi->ClearRect(rc);
	rc.Left--;rc.Right++;
	rc.Top++;rc.Bottom++;
	rc.Left++;rc.Right--;
	gi->ClearRect(rc);
	rc.Left--;rc.Right++;
	rc.Top--;rc.Bottom--;
	if(strlen(bp.players[k])!=0)
	{
		memset(buf,0,sizeof(buf));
		sprintf(buf,"dzs: %d",bp.dzs[k]);
		gi->ShowOneLineText(bp.players[k],0,rc);
		rc.Top++;rc.Bottom++;
		gi->ShowOneLineText(buf,0,rc);
	}

	/* Show ready status */
	if(showready)
	{
		/* Me */
		k=(k+1)%3;
		rc=_input_window;
		rc.Top=rc.Bottom-16;
		rc.Bottom=++rc.Top;
		rc.Left=(rc.Right+rc.Left)/2-5;
		rc.Right=rc.Left+10;
		rc.Left++;rc.Right--;
		gi->ClearRect(rc);
		rc.Left--;rc.Right++;
		if(strlen(bp.players[k])!=0 && bp.ready[k])
			gi->ShowOneLineText("READY",0,rc);

		/* RU */
		k=(k+1)%3;
		rc=_input_window;
		rc.Top+=14;
		rc.Bottom=++rc.Top;
		rc.Left=rc.Right-2-25;
		rc.Right=rc.Left+12;
		rc.Left++;rc.Right--;
		gi->ClearRect(rc);
		rc.Left--;rc.Right++;
		if(strlen(bp.players[k])!=0 && bp.ready[k])
			gi->ShowOneLineText("READY",0,rc);

		/* LU */
		k=(k+1)%3;
		rc=_input_window;
		rc.Top+=14;
		rc.Bottom=++rc.Top;
		rc.Right=rc.Left+2+25;
		rc.Left=rc.Right-12;
		rc.Left++;rc.Right--;
		gi->ClearRect(rc);
		rc.Left--;rc.Right++;
		if(strlen(bp.players[k])!=0 && bp.ready[k])
			gi->ShowOneLineText("READY",0,rc);
	}

	/* Show multiple */
	rc=_input_window;
	rc.Left=(5*(rc.Right-rc.Left)/7+rc.Left);
	rc.Right-=(rc.Right-rc.Left)/7;
	rc.Top=--rc.Bottom;
	rc.Left++;rc.Right--;
	gi->ClearRect(rc);
	rc.Left--;rc.Right++;

	memset(buf,0,sizeof(buf));
	sprintf(buf,"mul: %d",bp.mul);
	gi->ShowOneLineText(buf,0,rc);

	if(!this->dontbuff)
		gi->FlushOutputBuffer();
	return;
}


#define _CARD_BOX_WIDTH 6
#define _CARD_BOX_WIDTH_SPACE 2
#define _CARD_BOX_HEIGHT 6
#define _CARD_BOX_HEIGHT_SPACE 1

void _card::RenderOCards(_socket_packet_cardinfo cardinfo,COORD pos)
{
	if(cardinfo.cnt && !this->dontbuff)
		gi->StartOutputBuffer();
	ClearOCards(pos);
	if(!cardinfo.cnt)
		return;
	SMALL_RECT rc;
	for(int i=1;i<=cardinfo.cnt;pos.Y+=_CARD_BOX_HEIGHT_SPACE,i++)
	{
		rc.Left=pos.X;
		rc.Right=rc.Left+_CARD_BOX_WIDTH;
		rc.Top=pos.Y;
		rc.Bottom=rc.Top+_CARD_BOX_HEIGHT;
		gi->ClearRect(rc);
		gi->DrawBox(rc,false);
	}

	/* Print number of cards */
	pos.X=(rc.Left+rc.Right)/2;
	pos.Y=(rc.Top+rc.Bottom)/2;
	gi->ChangePos(pos);
	char buf[32]={0};
	sprintf(buf,"%d",cardinfo.cnt);
	gi->OutputStr(buf);

	if(!this->dontbuff)
		gi->FlushOutputBuffer();
	return;
}


void _card::RenderMCards(_socket_packet_cardinfo cardinfo,COORD pos,bool sld[])
{
	if(cardinfo.cnt && !this->dontbuff)
		gi->StartOutputBuffer();
	ClearMCards(pos);
	if(!cardinfo.cnt)
		return;
	if(cardinfo.cnt<20)
	{
		// Set to center position
		int cps=(20-cardinfo.cnt)/2;
		pos.X+=(_CARD_BOX_WIDTH_SPACE)*cps;
	}
	SMALL_RECT rc;
	for(int i=1;i<=cardinfo.cnt;pos.X+=_CARD_BOX_WIDTH_SPACE,i++)
	{
		if(sld[i])
			pos.Y--;
		rc.Left=pos.X;
		rc.Right=rc.Left+_CARD_BOX_WIDTH;
		rc.Top=pos.Y;
		rc.Bottom=rc.Top+_CARD_BOX_HEIGHT;
		gi->ClearRect(rc);
		gi->DrawBox(rc,false);
		pos.X++;pos.Y++;
		gi->ChangePos(pos);
		DWORD dwRet;
		WriteConsole(gi->hOutput,&cardinfo.cards[i].card,1,&dwRet,NULL);
		pos.Y+=2;
		gi->ChangePos(pos);
		WriteConsole(gi->hOutput,&cardinfo.cards[i].style,1,&dwRet,NULL);
		pos.Y+=2;
		gi->ChangePos(pos);
		WriteConsole(gi->hOutput,&cardinfo.cards[i].color,1,&dwRet,NULL);
		pos.Y-=2;
		pos.Y-=2;
		pos.X--;pos.Y--;
		if(sld[i])
			pos.Y++;
	}

	/* Print number of cards */
	pos.X=(rc.Left+rc.Right)/2;
	pos.Y=(rc.Top+rc.Bottom)/2;
	gi->ChangePos(pos);
	char buf[32]={0};
	sprintf(buf,"%d",cardinfo.cnt);
	gi->OutputStr(buf);
	
	if(!this->dontbuff)
		gi->FlushOutputBuffer();
	return;
}


#define _CARD_OUTBOX_WIDTH 4
#define _CARD_OUTBOX_HEIGHT 4

void _card::RenderLUOCards(_socket_packet_cardinfo cardinfo,COORD pos)
{
	if(cardinfo.cnt && !this->dontbuff)
		gi->StartOutputBuffer();
	ClearLimitedOutCards(pos);
	if(!cardinfo.cnt)
		return;
	RenderLimitedOutCards(cardinfo,pos);
	if(!this->dontbuff)
		gi->FlushOutputBuffer();
}


void _card::RenderRUOCards(_socket_packet_cardinfo cardinfo,COORD pos)
{
	if(cardinfo.cnt && !this->dontbuff)
		gi->StartOutputBuffer();
	ClearLimitedOutCards(pos);
	/* being divided by 0 will raise an error..... */
	if(!cardinfo.cnt)
		return;
	pos.X+=_CARD_BOX_WIDTH_SPACE*(20-cardinfo.cnt);
	RenderLimitedOutCards(cardinfo,pos);
	if(!this->dontbuff)
		gi->FlushOutputBuffer();
}

void _card::RenderMOCards(_socket_packet_cardinfo cardinfo,COORD pos)
{
	if(cardinfo.cnt && !this->dontbuff)
		gi->StartOutputBuffer();
	ClearLimitedOutCards(pos);
	if(!cardinfo.cnt)
		return;
	if(cardinfo.cnt<20)
	{
		// Set to center position
		int cps=(20-cardinfo.cnt)/2;
		pos.X+=(_CARD_BOX_WIDTH_SPACE)*cps;
	}
	RenderLimitedOutCards(cardinfo,pos);
	if(!this->dontbuff)
		gi->FlushOutputBuffer();
}


void _card::RenderLimitedOutCards(_socket_packet_cardinfo cardinfo,COORD pos)
{
	if(!cardinfo.cnt)
		return;
	SMALL_RECT rc;
	for(int i=1;i<=cardinfo.cnt;pos.X+=_CARD_BOX_WIDTH_SPACE,i++)
	{
		rc.Left=pos.X;
		rc.Right=rc.Left+_CARD_OUTBOX_WIDTH;
		rc.Top=pos.Y;
		rc.Bottom=rc.Top+_CARD_OUTBOX_HEIGHT;
		gi->ClearRect(rc);
		gi->DrawBox(rc,false);
		pos.X++;pos.Y++;
		gi->ChangePos(pos);
		WriteConsole(gi->hOutput,&cardinfo.cards[i].card,1,NULL,NULL);
		pos.Y++;
		gi->ChangePos(pos);
		WriteConsole(gi->hOutput,&cardinfo.cards[i].style,1,NULL,NULL);
		pos.Y++;
		gi->ChangePos(pos);
		WriteConsole(gi->hOutput,&cardinfo.cards[i].color,1,NULL,NULL);
		pos.Y--;
		pos.Y--;
		pos.X--;pos.Y--;
	}
}


void _card::ClearLimitedOutCards(COORD pos)
{
	SMALL_RECT rc;
	rc.Left=pos.X;rc.Top=pos.Y;
	/* Maximize the position, clear totally */
	rc.Right=rc.Left+_CARD_BOX_WIDTH_SPACE*20+_CARD_OUTBOX_WIDTH-1;
	rc.Bottom=rc.Top+_CARD_OUTBOX_HEIGHT;
	gi->ClearRect(rc);
}


void _card::ClearMCards(COORD pos)
{
	SMALL_RECT rc;
	rc.Left=pos.X;rc.Top=pos.Y-1;
	/* Maximize the position, clear totally (including the selection) */
	rc.Right=rc.Left+_CARD_BOX_WIDTH_SPACE*20+_CARD_BOX_WIDTH;
	rc.Bottom=rc.Top+_CARD_BOX_HEIGHT+1;
	gi->ClearRect(rc);
}


void _card::ClearOCards(COORD pos)
{
	SMALL_RECT rc;
	rc.Left=pos.X;rc.Top=pos.Y;
	/* Maximize the position, clear totally */
	rc.Right=rc.Left+_CARD_BOX_WIDTH;
	rc.Bottom=rc.Top+_CARD_BOX_HEIGHT_SPACE*20+_CARD_BOX_HEIGHT-1;
	gi->ClearRect(rc);
}

_socket_packet_cardinfo _card::SelectMCards(_socket_packet_cardinfo cardinfo,COORD pos,_socket_packet_cardinfo prevout)
{
	COORD opos=pos;
	bool sld[64]={0};
	if(cardinfo.cnt<20)
	{
		// Set to center position
		int cps=(20-cardinfo.cnt)/2;
		pos.X+=(_CARD_BOX_WIDTH_SPACE)*cps;
	}
	pos.Y+=_CARD_BOX_HEIGHT+1;
	
	_input::_option nopt;
	nopt.name="\x1e";
	std::vector<_input::_option>tmpopt,outopt;
	tmpopt.push_back(nopt);
	nopt.name="Out";
	nopt.pos.X=opos.X+(_CARD_BOX_WIDTH_SPACE*20+_CARD_BOX_WIDTH)/2-10;
	nopt.pos.Y=opos.Y-2;
	outopt.push_back(nopt);
	nopt.name="Skip";
	nopt.pos.X+=12;
	outopt.push_back(nopt);
	gi->DrawOption(outopt,2);

	_socket_packet_cardinfo scard;
	memset(&scard,0,sizeof(scard));
	int npos=1,where=1;
	tmpopt[0].pos=pos;
	tmpopt[0].pos.X+=_CARD_BOX_WIDTH_SPACE*(npos-1)+1;
	gi->DrawOption(tmpopt,0,false,false);
	gi->chinput.FlushBuffer();
	while(1)
	{
		Sleep(10);
		if(gi->bInputFlag)
		{
			/* Do remaining jobs */
			if(gi->bInputEnd)
				break;
			while(gi->bInputFlag)
				Sleep(100);
		}
		DWORD vKey=gi->chinput.ReadKeyPush();
		if(vKey==0)
			continue;

		if(where==1)
		{
			if(vKey==VK_LEFT && npos>1)
			{
				gi->DrawOption(tmpopt,_INPUT_DRAWOPTION_CLEAR,false);
				npos--;
				tmpopt[0].pos=pos;
				tmpopt[0].pos.X+=_CARD_BOX_WIDTH_SPACE*(npos-1)+1;
				gi->DrawOption(tmpopt,0,false,false);
			}
			else if(vKey==VK_RIGHT && npos<cardinfo.cnt)
			{
				gi->DrawOption(tmpopt,_INPUT_DRAWOPTION_CLEAR,false);
				npos++;
				tmpopt[0].pos=pos;
				tmpopt[0].pos.X+=_CARD_BOX_WIDTH_SPACE*(npos-1)+1;
				gi->DrawOption(tmpopt,0,false,false);
			}
			else if(vKey==VK_UP)
			{
				sld[npos]=true;
				this->RenderMCards(cardinfo,opos,sld);
			}
			else if(vKey==VK_DOWN)
			{
				sld[npos]=false;
				this->RenderMCards(cardinfo,opos,sld);
			}
			else if(vKey==VK_RETURN)
			{
				where=2;
				gi->DrawOption(outopt,0);
			}
			else if(vKey==VK_SPACE)
			{
				memset(sld,0,sizeof(sld));
				this->RenderMCards(cardinfo,opos,sld);
			}
		}
		else
			if(vKey==VK_LEFT && where==3)
			{
				gi->DrawOption(outopt,0);
				where=2;
			}
			else if(vKey==VK_RIGHT && where==2)
			{
				gi->DrawOption(outopt,1);
				where=3;
			}
			else if(vKey==VK_DOWN)
			{
				gi->DrawOption(outopt,2);
				where=1;
			}
			else if(vKey==VK_RETURN)
				if(where==3 && prevout.cnt)
					break;
				else if(where==2)
				{
					int cnt=0;
					for(int i=1;i<=cardinfo.cnt;i++)
						if(sld[i])
							cnt++;
					if(cnt)
					{
						for(int i=1;i<=cardinfo.cnt;i++)
							if(sld[i])
								scard.cards[++scard.cnt]=cardinfo.cards[i];
						if(_gcard_GetCardsType(scard.cards,scard.cnt).first>0 && \
							(_gcard_GetCardsLevel(scard.cards,scard.cnt) > _gcard_GetCardsLevel(prevout.cards,prevout.cnt) \
								|| (_gcard_GetCardsLevel(scard.cards,scard.cnt) == _gcard_GetCardsLevel(prevout.cards,prevout.cnt) \
									&& _gcard_GetCardsType(scard.cards,scard.cnt).first == _gcard_GetCardsType(prevout.cards,prevout.cnt).first \
									&& _gcard_GetCardsType(scard.cards,scard.cnt).second > _gcard_GetCardsType(prevout.cards,prevout.cnt).second)))
										break;
					}
					memset(&scard,0,sizeof(scard));
				}
	}
	this->ClearMCards(opos);
	opos.Y++;
	this->ClearMCards(opos);
	
	return scard;
}

void _card::SetBufferActive(bool sb)
{
	this->dontbuff=!sb;
}










