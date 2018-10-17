// io.cpp
#include "stdafx.h"
#include "C_BOARD.h"
#include "C_MOVEGEN.h"

#pragma warning( disable: 4996)

char *PrSq(int sq) 
{

	static char SqStr[3];

	int file = FilesBrd[sq];
	int rank = RanksBrd[sq];

	sprintf(SqStr, "%c%c", ('a' + file), ('1' + rank));

	return SqStr;

}

char *PrMove(int move) 
{

	static char MvStr[6];

	int ff = FilesBrd[MV_FROMSQ(move)];
	int rf = RanksBrd[MV_FROMSQ(move)];
	int ft = FilesBrd[MV_TOSQ(move)];
	int rt = RanksBrd[MV_TOSQ(move)];

	int promoted = MV_PROMOTED(move);

	if (promoted) 
	{
		char pchar = 'q';
		if (IsKn(promoted)) 
		{
			pchar = 'n';
		}
		else if (IsRQ(promoted) && !IsBQ(promoted)) 
		{
			pchar = 'r';
		}
		else if (!IsRQ(promoted) && IsBQ(promoted)) 
		{
			pchar = 'b';
		}
		sprintf(MvStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar);
	}
	else 
	{
		sprintf(MvStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
	}

	return MvStr;
}

int ParseMove(std::string sMove, C_BOARD *pBoard) 
{

	ASSERT(pBoard->CheckBoard());

	if (sMove.at(1) > '8' || sMove.at(1) < '1') return NOMOVE;
	if (sMove.at(3) > '8' || sMove.at(3) < '1') return NOMOVE;
	if (sMove.at(0) > 'h' || sMove.at(0) < 'a') return NOMOVE;
	if (sMove.at(2) > 'h' || sMove.at(2) < 'a') return NOMOVE;

	int from = FR2SQ(sMove.at(0) - 'a', sMove.at(1) - '1');
	int to   = FR2SQ(sMove.at(2) - 'a', sMove.at(3) - '1');

	ASSERT(SqOnBoard(from) && SqOnBoard(to));

	S_MOVELIST movelist[1];
	C_MOVEGEN::mgGenerateMoves(pBoard,movelist,false);

	int Move    = 0;
	int PromPce = EMPTY;

	for (int MoveNum = 0; MoveNum < movelist->size; ++MoveNum) 
	{
		Move = movelist->moves[MoveNum].move;
		if (MV_FROMSQ(Move) == from && MV_TOSQ(Move) == to) 
		{
			PromPce = MV_PROMOTED(Move);
			if (PromPce != EMPTY) {
				if (IsRQ(PromPce) && !IsBQ(PromPce) && sMove.at(4) == 'r')
				{
					return Move;
				}
				else if (!IsRQ(PromPce) && IsBQ(PromPce) && sMove.at(4) == 'b')
				{
					return Move;
				}
				else if (IsRQ(PromPce) && IsBQ(PromPce) && sMove.at(4) == 'q')
				{
					return Move;
				}
				else if (IsKn(PromPce) && sMove.at(4) == 'n')
				{
					return Move;
				}
				continue;
			}
			return Move;
		}
	}

	return NOMOVE;
}


