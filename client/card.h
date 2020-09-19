#ifndef _H_CARD
#define _H_CARD

#include "../EasySocket.h"
#include "input.h"
#include <cassert>

class _card
{
//private:
public:
	_input*gi;
	bool dontbuff;

	/*
		Render limited(out) cards
		(_socket_packet_cardinfo)	-> card information
		(COORD)						-> beginning position of the first card
	*/
	void RenderLimitedOutCards(_socket_packet_cardinfo,COORD);

	
	/*
		Clear my cards
		(COORD)		-> beginning position of the first card
	*/
	void ClearMCards(COORD);


	/*
		Clear others' cards
		(COORD)		-> beginning position of the first card
	*/
	void ClearOCards(COORD);


public:
	/*
		Must be initialized with a _input attached
		(_input*)	-> _input instance
	*/
	_card(_input*);

	/*
		Render game framework and players
		(_socket_packet_roominfo)	-> roominfo
		(int)						-> current player index
		(bool)						-> show ready state (DEFAULT:false)
	*/
	void RenderPlayerFramework(_socket_packet_roominfo,int,bool=false);
	

	/*
		Render others' cards
		(_socket_packet_cardinfo)	-> card information
		(COORD)						-> beginning position of the first card
	*/
	void RenderOCards(_socket_packet_cardinfo,COORD);


	/*
		Render my cards
		(_socket_packet_cardinfo)	-> card information
		(COORD)						-> beginning position of the first card
		(bool[])					-> cards which are selected or not
	*/
	void RenderMCards(_socket_packet_cardinfo,COORD,bool[]);


	/*
		Render LU's out cards
		(_socket_packet_cardinfo)	-> card information
		(COORD)						-> beginning position of the first card
	*/
	void RenderLUOCards(_socket_packet_cardinfo,COORD);


	/*
		Render RU's out cards
		(_socket_packet_cardinfo)	-> card information
		(COORD)						-> beginning position of the first card
	*/
	void RenderRUOCards(_socket_packet_cardinfo,COORD);


	/*
		Render ME's out cards
		(_socket_packet_cardinfo)	-> card information
		(COORD)						-> beginning position of the first card
	*/
	void RenderMOCards(_socket_packet_cardinfo,COORD);


	/*
		Clear limited(out) cards
		(COORD)		-> beginning position of the first card
	*/
	void ClearLimitedOutCards(COORD);

	
	/*
		Select cards
		(_socket_packet_cardinfo)	-> card information
		(COORD)						-> beginning position of the first card
		(_socket_packet_cardinfo)	-> previous player's out
		return selected cards in _socket_packet_cardinfo
	*/
	_socket_packet_cardinfo SelectMCards(_socket_packet_cardinfo,COORD,_socket_packet_cardinfo);


	/*
		Decide whether buffer the screen while outputting something
		(bool)	-> true/false
	*/
	void SetBufferActive(bool);

};












#endif

