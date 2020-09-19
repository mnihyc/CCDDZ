#ifndef _H_G_CARD
#define _H_G_CARD

#pragma warning(disable:4786) // bug of VC6

#pragma once
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cassert>
#include <climits>
#include <vector>

struct _gcard_eachcard
{
	char card;
	char style;
	char color;
	bool operator < (const _gcard_eachcard&oth)const
	{
		if(this->GetCardLevel() == oth.GetCardLevel())
			return this->style < oth.style;
		return this->GetCardLevel() < oth.GetCardLevel();
	}
	bool operator == (const _gcard_eachcard&oth)const
	{
		return (card==oth.card && style==oth.style && color==oth.color);
	}
	int GetCardLevel()const
	{
		if(card==0x01)
			return 18;
		if(card==0x02)
			return 19;
		if(card>='3' && card<='9')
			return (card-'0');
		if(card==0x10)
			return 10;
		if(card=='J')
			return 11;
		if(card=='Q')
			return 12;
		if(card=='K')
			return 13;
		if(card=='A')
			return 14;
		if(card=='2')
			return 15;
		return -1;
	}
};

static int _gcard_ShuffleAllCards(_gcard_eachcard _cards[])
{
	int l_cards=0;
	for(int loops=1;loops<=4;loops++)
		for(int i=0;i<=9+3;i++)
		{
			switch(loops)
			{
				case 1:
					_cards[l_cards].style=0x3;
					_cards[l_cards].color='R';
					break;
				case 2:
					_cards[l_cards].style=0x4;
					_cards[l_cards].color='B';
					break;
				case 3:
					_cards[l_cards].style=0x5;
					_cards[l_cards].color='R';
					break;
				case 4:
					_cards[l_cards].style=0x6;
					_cards[l_cards].color='B';
					break;
			}
			_cards[l_cards].card='0'+i;
			if(i==0)
				_cards[l_cards].card=0x10;
			if(i==1)
				_cards[l_cards].card='A';
			if(i==11-1)
				_cards[l_cards].card='J';
			if(i==12-1)
				_cards[l_cards].card='Q';
			if(i==13-1)
				_cards[l_cards].card='K';
			l_cards++;
		}
	_cards[l_cards].card=0x01;
	_cards[l_cards].style='K';
	_cards[l_cards++].color='B';
	_cards[l_cards].card=0x02;
	_cards[l_cards].style='K';
	_cards[l_cards++].color='R';
	srand(time(NULL));rand();rand();rand();
	std::random_shuffle(&_cards[0],&_cards[l_cards]);
	std::random_shuffle(&_cards[0],&_cards[l_cards]);
	std::random_shuffle(&_cards[0],&_cards[l_cards]);
	return l_cards;
}

static int _gcard_RemoveCardsFrom(_gcard_eachcard cards[],int lc,const _gcard_eachcard b[],int lb)
{
	for(int k=1;k<=lb;k++)
		for(int i=1;i<=lc;i++)
			if(b[k]==cards[i])
			{
				for(int j=i+1;j<=lc;j++)
					cards[j-1]=cards[j];
				lc--;
			}
	return lc;
}

static bool _gcard_cequal(int cnt,...)
{
	va_list valist;
	va_start(valist,cnt);
	_gcard_eachcard o=va_arg(valist,_gcard_eachcard),p;
	bool equ=true;
	for(int i=2;i<=cnt;i++)
		if(o.GetCardLevel() != (p=va_arg(valist,_gcard_eachcard)).GetCardLevel())
		{
			equ=false;
			break;
		}
		else
			o=p;
	va_end(valist);
	return equ;
}

static bool _gcard_ccontinue(int cnt,...)
{
	va_list valist;
	va_start(valist,cnt);
	_gcard_eachcard o=va_arg(valist,_gcard_eachcard),p;
	if(o.GetCardLevel()>14)
		return false;
	bool con=true;
	for(int i=2;i<=cnt;i++)
		if(o.GetCardLevel()+1 != (p=va_arg(valist,_gcard_eachcard)).GetCardLevel())
		{
			con=false;
			break;
		}
		else if(p.GetCardLevel()>14)
		{
			con=false;
			break;
		}
		else
			o=p;
	va_end(valist);
	return con;
}

static int _gcard_GetCardsLevel(_gcard_eachcard cards[],int lc)
{
	if(!lc)
		return 0;
	if(lc==4 && _gcard_cequal(4,cards[1],cards[2],cards[3],cards[4]))
		return 2;
	if(lc==2 && cards[1].GetCardLevel()==18 && cards[2].GetCardLevel()==19)
		return 3;
	return 1;
}

static std::pair<int,int> _gcard_GetCardsType(_gcard_eachcard cards[],int lc)
{
	int level=_gcard_GetCardsLevel(cards,lc);
	// 四炸
	if(level==2)
		return std::make_pair(1,cards[1].GetCardLevel());
	// 王炸
	if(level==3)
		return std::make_pair(2,cards[2].GetCardLevel());
	std::vector<_gcard_eachcard>spt[5];
	int cnt=1;
	for(int i=2;i<=lc;i++)
		if(!_gcard_cequal(2,cards[i],cards[i-1]))
		{
			spt[cnt].push_back(cards[i-1]);
			cnt=1;
		}
		else
			cnt++;
	spt[cnt].push_back(cards[lc]);
	// 单根
	if(lc==1)
		return std::make_pair(3,cards[1].GetCardLevel());
	// 对子
	if(lc==2 && spt[2].size()==1)
		return std::make_pair(4,cards[1].GetCardLevel());
	// 三根/三带
	if(spt[3].size()==1)
	{
		if(lc==3)
			return std::make_pair(5,spt[3][0].GetCardLevel());
		if(lc==4 && spt[1].size()==1)
			return std::make_pair(6,spt[3][0].GetCardLevel());
		if(lc==5 && spt[2].size()==1)
			return std::make_pair(7,spt[3][0].GetCardLevel());
	}
	// 飞机
	if(spt[3].size()==2 && _gcard_ccontinue(2,spt[3][0],spt[3][1]))
	{
		if(lc==6)
			return std::make_pair(8,spt[3][0].GetCardLevel());
		if(lc==8 && (spt[1].size()==2 || spt[2].size()==1))
			return std::make_pair(9,spt[3][0].GetCardLevel());
		if(lc==10 && spt[2].size()==2)
			return std::make_pair(10,spt[3][0].GetCardLevel());
	}
	if(spt[3].size()==3 && _gcard_ccontinue(3,spt[3][0],spt[3][1],spt[3][2]))
	{
		if(lc==9)
			return std::make_pair(11,spt[3][0].GetCardLevel());
		if(lc==12 && (spt[1].size()==3 || (spt[2].size()==1 && spt[1].size()==1)))
			return std::make_pair(12,spt[3][0].GetCardLevel());
		if(lc==15 && spt[2].size()==3)
			return std::make_pair(13,spt[3][0].GetCardLevel());
	}
	if(spt[3].size()==4 && _gcard_ccontinue(4,spt[3][0],spt[3][1],spt[3][2],spt[3][4]))
	{
		if(lc==12)
			return std::make_pair(14,spt[3][0].GetCardLevel());
		if(lc==16 && (spt[1].size()==4 || (spt[2].size()==1 && spt[1].size()==2) || spt[2].size()==2))
			return std::make_pair(15,spt[3][0].GetCardLevel());
		if(lc==20 && spt[2].size()==4)
			return std::make_pair(16,spt[3][0].GetCardLevel());
	}
	// 四带二/一对
	if(spt[4].size()==1)
	{
		if(lc==6 && (spt[1].size()==2 || spt[2].size()==1))
			return std::make_pair(17,spt[4][0].GetCardLevel());
		if(lc==8 && spt[2].size()==2)
			return std::make_pair(18,spt[4][0].GetCardLevel());
	}
	if(lc==8 && spt[4].size()==2)
		return std::make_pair(18,spt[4][1].GetCardLevel());
	// 顺子
	if(lc>=5 && !spt[2].size() && !spt[3].size() && !spt[4].size())
	{
		bool cont=true;
		for(int i=1;i<spt[1].size();i++)
			if(!_gcard_ccontinue(2,spt[1][i-1],spt[1][i]))
			{
				cont=false;
				break;
			}
		if(cont)
			return std::make_pair(19+lc-5,spt[1][0].GetCardLevel());
	}
	// 连对
	if(lc>=6 && !spt[1].size() && !spt[3].size() && !spt[4].size())
	{
		bool cont=true;
		for(int i=1;i<spt[2].size();i++)
			if(!_gcard_ccontinue(2,spt[2][i-1],spt[2][i]))
			{
				cont=false;
				break;
			}
		if(cont)
			return std::make_pair(27+lc/2-3,spt[2][0].GetCardLevel());
	}

	return std::make_pair(-1,-1);
}




#endif