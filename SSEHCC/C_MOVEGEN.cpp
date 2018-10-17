#include "stdafx.h"
#include "C_BOARD.h"
#include "C_BB_64.h"
#include "C_MOVEGEN.h"

#pragma region Magic Move Generation Routines

C_BB_64 C_MOVEGEN::mgBlockermask_rook  (C_BOARD *pBoard,int sq64)
{
	int   sq120 = SQ120(sq64);
	int   t_sq120;
	int   pce;
	C_BB_64 bb64 = 0;

	for (int rkDir : RkDir)
	{
		t_sq120 = sq120 + rkDir;
		pce = pBoard->pieces[t_sq120];
		while (pce != OFFBOARD)
		{
			bb64.SetBit(SQ64(t_sq120));
			t_sq120 += rkDir;
			pce = pBoard->pieces[t_sq120];
		}

		// don't need edge squares
		bb64.ClrBit(SQ64(t_sq120 - rkDir));
	}

	return bb64;
}
C_BB_64 C_MOVEGEN::mgBlockermask_bishop(C_BOARD *pBoard, int sq64)
{
	int   sq120 = SQ120(sq64);
	int   t_sq120;
	int   pce;
	C_BB_64 bb64 = 0;

	for (int biDir : BiDir)
	{
		t_sq120 = sq120 + biDir;
		pce = pBoard->pieces[t_sq120];
		while (pce != OFFBOARD)
		{
			bb64.SetBit(SQ64(t_sq120));
			t_sq120 += biDir;
			pce = pBoard->pieces[t_sq120];
		}

		// don't need edge squares
		bb64.ClrBit(SQ64(t_sq120 - biDir));
	}

	return bb64;
}

C_BB_64 C_MOVEGEN::mgMoveboard_rook    (int square, U64 blockerboard)
{

	return C_BB_64(0);
}
C_BB_64 C_MOVEGEN::mgMoveboard_bishop  (int square, U64 blockerboard)
{

	return C_BB_64(0);
}

C_BB_64 C_MOVEGEN::mgBlockerboard      (int index, U64 blockermask)
{

	return C_BB_64(0);
}

#pragma endregion

C_BB_64 C_MOVEGEN::mgDAndAntiDMoves     (C_BB_64 bb64_occupiedSQs, const int sq64)
{
	int div8 = (sq64 / 8);
	int mod8 = (sq64 % 8);

	S64 binarySQ = (S64)SetMask[sq64];
	S64 SQ2 = (2 * binarySQ);
	S64 rBinarySQ = (S64)C_BB_64((U64)binarySQ).BitReverse().value;
	S64 rSQ2 = (2 * rBinarySQ);

	// long posdiag = ((occupied&diagonalMasks8[div8+mod8]) - SQ2
	U64 possibilitiesDiagonal = (
		(C_BB_64(C_BB_64(bb64_occupiedSQs & (U64)((S64)DiagonalMasks8[div8 + mod8])).value - SQ2).value)
		^
		(C_BB_64(C_BB_64(bb64_occupiedSQs & (U64)((S64)DiagonalMasks8[div8 + mod8])).BitReverse().value - rSQ2).BitReverse().value)
		);
	U64 possibilitiesAntiDiagonal = (
		(bb64_occupiedSQs.value        & (U64)((S64)AntiDiagonalMasks8[div8 + 7 - mod8])) - SQ2)
		^
		(C_BB_64(C_BB_64(bb64_occupiedSQs & (U64)((S64)AntiDiagonalMasks8[div8 + 7 - mod8])).BitReverse().value - rSQ2).BitReverse().value
			);

	U64 bb = (possibilitiesDiagonal     & DiagonalMasks8[div8 + mod8])
		| (possibilitiesAntiDiagonal & AntiDiagonalMasks8[div8 + 7 - mod8])
		;

	return (C_BB_64(bb));
}
C_BB_64 C_MOVEGEN::mgHAndVMoves         (C_BB_64 bb64_occupiedSQs, const int sq64)
{
	S64 binarySQ = (S64)SetMask[sq64];
	S64 SQ2 = (2 * binarySQ);
	S64 rBinarySQ = (S64)C_BB_64((U64)binarySQ).BitReverse().value;
	S64 rSQ2 = (2 * rBinarySQ);

	U64 possibilitiesHorizontal = (
		((U64)((S64)bb64_occupiedSQs.value - SQ2))
		^
		(C_BB_64((U64)((S64)bb64_occupiedSQs.BitReverse().value - rSQ2)).BitReverse().value)
		);

	U64 possibilitiesVertical = (
		((U64)(((S64)((bb64_occupiedSQs.value & FileMasks8[sq64 % 8])) - SQ2)))
		^
		(C_BB_64((U64)(((S64)(C_BB_64(bb64_occupiedSQs.value & FileMasks8[sq64 % 8])).BitReverse().value - rSQ2))).BitReverse().value)
		)
		;

	return C_BB_64((possibilitiesHorizontal & RankMasks8[sq64 / 8]) | (possibilitiesVertical & FileMasks8[sq64 % 8]));

}

bool  C_MOVEGEN::mgSqAttacked         (C_BOARD *pBoard, const int sq120, const int sideToNotMove)
{

	int pce, tosq120;

	ASSERT(SqOnBoard(sq120));
	ASSERT(SideValid(pBoard->side));
	ASSERT(pBoard->CheckBoard());

	// pawns
	if (sideToNotMove == WHITE)
	{
		if (pBoard->pieces[sq120 - 11] == wP || pBoard->pieces[sq120 - 9] == wP)
			return true;
	}
	else
	{
		if (pBoard->pieces[sq120 + 11] == bP || pBoard->pieces[sq120 + 9] == bP)
			return true;
	}

	// knights
	for (int knDir : KnDir)
	{
		pce = pBoard->pieces[sq120 + knDir];
		ASSERT(PceValidEmptyOffbrd(pce));
		if (pce != OFFBOARD && IsKn(pce) && PieceCol[pce] == sideToNotMove)
			return true;
	}

	// rooks, queens
	for (int rkDir : RkDir)
	{
		tosq120 = sq120 + rkDir;
		ASSERT(SqIs120(tosq120));
		pce = pBoard->pieces[tosq120];
		ASSERT(PceValidEmptyOffbrd(pce));
		while (pce != OFFBOARD)
		{
			if (pce != EMPTY)
			{ // bumped into something, were done
				if (IsRQ(pce) && PieceCol[pce] == sideToNotMove)
					return true;
				break;
			}
			tosq120 += rkDir;
			ASSERT(SqIs120(tosq120));
			pce = pBoard->pieces[tosq120];
		}
	}

	// bishops, queens
	for (int biDir : BiDir)
	{
		tosq120 = sq120 + biDir;
		ASSERT(SqIs120(tosq120));
		pce = pBoard->pieces[tosq120];
		ASSERT(PceValidEmptyOffbrd(pce));
		while (pce != OFFBOARD)
		{
			if (pce != EMPTY)
			{ // bumped into something, were done
				if (IsBQ(pce) && PieceCol[pce] == sideToNotMove)
					return true;
				break;
			}
			tosq120 += biDir;
			ASSERT(SqIs120(tosq120));
			pce = pBoard->pieces[tosq120];
		}
	}

	// kings
	for (int kiDir : KiDir)
	{
		pce = pBoard->pieces[sq120 + kiDir];
		ASSERT(PceValidEmptyOffbrd(pce));
		if (pce != OFFBOARD && IsKi(pce) && PieceCol[pce] == sideToNotMove)
			return true;
	}

	return false;

}

void  C_MOVEGEN::mgAddQuietMove       (C_BOARD *pBoard, S_MOVELIST  *movelist, const int move                               )
{
	ASSERT(SqOnBoard(MV_FROMSQ(move)));
	ASSERT(SqOnBoard(MV_TOSQ(move)));
	ASSERT(pBoard->CheckBoard());
	ASSERT(pBoard->ply >= 0 && pBoard->ply < MAX_PLY_DEPTH);

	S_MOVE moveEntry;

	moveEntry.move = move;

	// get score
	if (pBoard->searchKillers[0][pBoard->ply] == move)
		moveEntry.score = 900000;
	else
	if (pBoard->searchKillers[1][pBoard->ply] == move)
		moveEntry.score = 800000;
	else
		moveEntry.score = pBoard->searchHistory[pBoard->pieces[MV_FROMSQ(move)]][MV_TOSQ(move)];

	movelist->Add(moveEntry); 
}
void  C_MOVEGEN::mgAddCaptureMove     (C_BOARD *pBoard, S_MOVELIST  *movelist, const int move                               )
{
	ASSERT(SqOnBoard(MV_FROMSQ(move)));
	ASSERT(SqOnBoard(MV_TOSQ(move)));
	ASSERT(PieceValid(MV_CAPTURED(move)));
	ASSERT(pBoard->CheckBoard());

	S_MOVE moveEntry;
	moveEntry.move = move;
	moveEntry.score = MvvLvaScores[MV_CAPTURED(move)][pBoard->pieces[MV_FROMSQ(move)]] + 1000000;
	
	movelist->Add(moveEntry); 
}

void  C_MOVEGEN::mgAddEnPassantMove   (C_BOARD *pBoard, S_MOVELIST  *movelist, const int move                               )
{
	ASSERT(SqOnBoard(MV_FROMSQ(move)));
	ASSERT(SqOnBoard(MV_TOSQ(move)));
	ASSERT(pBoard->CheckBoard());
	ASSERT((RanksBrd[MV_TOSQ(move)] == RANK_6 && pBoard->side == WHITE) || (RanksBrd[MV_TOSQ(move)] == RANK_3 && pBoard->side == BLACK));

	S_MOVE moveEntry;
	moveEntry.move = move;
	moveEntry.score = MvvLvaScores[MV_CAPTURED(move)][pBoard->pieces[MV_FROMSQ(move)]] + 1000000;
	movelist->Add(moveEntry);
}
void  C_MOVEGEN::mgAddWhitePawnCapMove(C_BOARD *pBoard, S_MOVELIST  *movelist, const int from, const int to, const int cap  ) {

	ASSERT(PieceValidEmpty(cap));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(pBoard->CheckBoard());

	if (RanksBrd[from] == RANK_7)
	{
		C_MOVEGEN::mgAddCaptureMove(pBoard,movelist, MOVE(from, to, cap, wQ, 0));
		C_MOVEGEN::mgAddCaptureMove(pBoard,movelist, MOVE(from, to, cap, wR, 0));
		C_MOVEGEN::mgAddCaptureMove(pBoard,movelist, MOVE(from, to, cap, wB, 0));
		C_MOVEGEN::mgAddCaptureMove(pBoard,movelist, MOVE(from, to, cap, wN, 0));
	}
	else
	{
		C_MOVEGEN::mgAddCaptureMove(pBoard,movelist, MOVE(from, to, cap, EMPTY, 0));
	}
}
void  C_MOVEGEN::mgAddWhitePawnMove   (C_BOARD *pBoard, S_MOVELIST  *movelist, const int from, const int to                 ) {

	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(pBoard->CheckBoard());

	if (RanksBrd[from] == RANK_7)
	{
		C_MOVEGEN::mgAddQuietMove(pBoard,movelist, MOVE(from, to, EMPTY, wQ, 0));
		C_MOVEGEN::mgAddQuietMove(pBoard,movelist, MOVE(from, to, EMPTY, wR, 0));
		C_MOVEGEN::mgAddQuietMove(pBoard,movelist, MOVE(from, to, EMPTY, wB, 0));
		C_MOVEGEN::mgAddQuietMove(pBoard,movelist, MOVE(from, to, EMPTY, wN, 0));
	}
	else
	{
		C_MOVEGEN::mgAddQuietMove(pBoard,movelist, MOVE(from, to, EMPTY, EMPTY, 0));
	}
}
void  C_MOVEGEN::mgAddBlackPawnCapMove(C_BOARD *pBoard, S_MOVELIST  *movelist, const int from, const int to, const int cap  ) {

	ASSERT(PieceValidEmpty(cap));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(pBoard->CheckBoard());

	if (RanksBrd[from] == RANK_2)
	{
		C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from, to, cap, bQ, 0));
		C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from, to, cap, bR, 0));
		C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from, to, cap, bB, 0));
		C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from, to, cap, bN, 0));
	}
	else {
		C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from, to, cap, EMPTY, 0));
	}
}
void  C_MOVEGEN::mgAddBlackPawnMove   (C_BOARD *pBoard, S_MOVELIST  *movelist, const int from, const int to                 ) {

	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(pBoard->CheckBoard());

	if (RanksBrd[from] == RANK_2)
	{
		C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from, to, EMPTY, bQ, 0));
		C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from, to, EMPTY, bR, 0));
		C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from, to, EMPTY, bB, 0));
		C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from, to, EMPTY, bN, 0));
	}
	else
	{
		C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from, to, EMPTY, EMPTY, 0));
	}
}



#pragma region Handle White Pieces
void  C_MOVEGEN::mgHandleWhitePawns   (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = C_BB_64(pBoard->bb64_wPawns);

	while (bb64 != 0)
	{
		int sq120 = SQ120(bb64.BitPop());

		if ((pBoard->pieces[sq120 + 10] == EMPTY) && !capturesOnly)
		{
			C_MOVEGEN::mgAddWhitePawnMove(pBoard,movelist, sq120, sq120 + 10);
			if (RanksBrd[sq120] == RANK_2 && pBoard->pieces[sq120 + 20] == EMPTY)
				C_MOVEGEN::mgAddQuietMove(pBoard,movelist, MOVE(sq120, (sq120 + 20), EMPTY, EMPTY, MVFLAG_PawnStart));
		}

		for (int tosq120 : { sq120 + 9, sq120 + 11 })
		{
			if (!SQOFFBOARD(tosq120))
			{
				if ((pBoard->bb64_bPiecesSQs & SetMask[SQ64(tosq120)]) != 0) // black piece attacked
				{
					C_MOVEGEN::mgAddWhitePawnCapMove(pBoard,movelist, sq120, tosq120, pBoard->pieces[tosq120]);
					pBoard->bb64_wPawnAttks.SetBit(SQ64(tosq120));
				}
				else
				if ((pBoard->bb64_wPiecesSQs & SetMask[SQ64(tosq120)]) != 0) // white piece protected
				{
					if (pBoard->pieces[tosq120] != wK)
						pBoard->bb64_wPawnProts.SetBit(SQ64(tosq120));
				}
			}

		}

		if (pBoard->SQenPas120 != NO_EP_SQ)
		{
			if (sq120 + 9 == pBoard->SQenPas120)
			{
				C_MOVEGEN::mgAddEnPassantMove(pBoard, movelist, MOVE(sq120, sq120 + 9, EMPTY, EMPTY, MVFLAG_EnPassant));
				pBoard->bb64_wPawnAttks.SetBit(SQ64(sq120 + 9));
			}
			else
			if (sq120 + 11 == pBoard->SQenPas120)
			{
				C_MOVEGEN::mgAddEnPassantMove(pBoard, movelist, MOVE(sq120, sq120 + 11, EMPTY, EMPTY, MVFLAG_EnPassant));
				pBoard->bb64_wPawnAttks.SetBit(SQ64(sq120 + 11));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleWhiteKnights (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64_possibility;
	C_BB_64 bb64 = pBoard->bb64_wKnights;

	while (bb64 != 0)
	{
		int sq64 = bb64.BitPop();
		int sq120 = SQ120(sq64);

		bb64_possibility = KnightSpans[sq64];

		while (bb64_possibility != 0)
		{
			int tosq64 = bb64_possibility.BitPop();
			int tosq120 = SQ120(tosq64);

			if ((pBoard->bb64_bPiecesSQs & SetMask[tosq64]) != 0)
			{
				// black piece attacked
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(sq120, tosq120, pBoard->pieces[tosq120], EMPTY, 0));
				pBoard->bb64_wKnightAttks.SetBit(SQ64(tosq120));
			}
			else
			if ((pBoard->bb64_wPiecesSQs & SetMask[tosq64]) != 0)
			{
				// white piece protected
				if (pBoard->pieces[tosq120] != wK)
					pBoard->bb64_wKnightProts.SetBit(SQ64(tosq120));
			}
			else
			if (!capturesOnly)
			{
				// no attack and no protect
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(sq120, tosq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleWhiteKing    (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64_possibility;
	C_BB_64 bb64 = C_BB_64(pBoard->bb64_wKings);

	while (bb64 != 0)
	{
		int sq64 = bb64.BitPop();
		int sq120 = SQ120(sq64);
		ASSERT(pBoard->bb64_wKings.BitCount() == 1);
		ASSERT(bb64.BitCount() == 0);

		bb64_possibility = KingSpans[sq64];

		while (bb64_possibility != 0)
		{
			int tosq64 = bb64_possibility.BitPop();
			int tosq120 = SQ120(tosq64);

			if ((pBoard->bb64_bPiecesSQs & SetMask[tosq64]) != 0)
			{   // black piece attacked
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(sq120, tosq120, pBoard->pieces[tosq120], EMPTY, 0));
				pBoard->bb64_wKingAttks.SetBit(SQ64(tosq120));
			}
			else
			if ((pBoard->bb64_wPiecesSQs & SetMask[tosq64]) != 0)
			{   // white piece protected
				pBoard->bb64_wKingProts.SetBit(SQ64(tosq120));
			}
			else
			if (!capturesOnly)
			{
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(sq120, tosq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleWhiteQueens  (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = pBoard->bb64_wQueens;
	C_BB_64 bb64_possibility;

	while (bb64 != 0)
	{
		int   from_sq64 = bb64.BitPop();
		int   from_sq120 = SQ120(from_sq64);

		bb64_possibility = C_MOVEGEN::mgHAndVMoves    (pBoard->bb64_OccupiedSQs, from_sq64)      // & (bb64_EmptySQs|bb64_bPiecesSQs) 
		       	         | C_MOVEGEN::mgDAndAntiDMoves(pBoard->bb64_OccupiedSQs, from_sq64);     // & (bb64_EmptySQs|bb64_bPiecesSQs); 

		while (bb64_possibility != 0)
		{
			int to_sq64 = bb64_possibility.BitPop();
			int to_sq120 = SQ120(to_sq64);

			if ((pBoard->bb64_bPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// black piece attacked                                                                                                                         
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from_sq120, to_sq120, pBoard->pieces[to_sq120], EMPTY, 0));
				pBoard->bb64_wQueenAttks.SetBit(to_sq64);
			}
			else
			if ((pBoard->bb64_wPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// white piece protected
				if (pBoard->pieces[to_sq120] != wK)
					pBoard->bb64_wQueenProts.SetBit(to_sq64);
			}
			else
			if (!capturesOnly)
			{
				// no attack and no protect
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from_sq120, to_sq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleWhiteRooks   (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = pBoard->bb64_wRooks;
	C_BB_64 bb64_possibility;

	while (bb64 != 0)
	{
		int   from_sq64 = bb64.BitPop();
		int   from_sq120 = SQ120(from_sq64);

		bb64_possibility = C_MOVEGEN::mgHAndVMoves(pBoard->bb64_OccupiedSQs, from_sq64);      // & (bb64_EmptySQs|bb64_bPiecesSQs)

		while (bb64_possibility != 0)
		{
			int to_sq64 = bb64_possibility.BitPop();
			int to_sq120 = SQ120(to_sq64);

			if ((pBoard->bb64_bPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// black piece attacked                                                                                                                         
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from_sq120, to_sq120, pBoard->pieces[to_sq120], EMPTY, 0));
				pBoard->bb64_wRookAttks.SetBit(to_sq64);
			}
			else
			if ((pBoard->bb64_wPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// white piece protected
				if (pBoard->pieces[to_sq120] != wK)
					pBoard->bb64_wRookProts.SetBit(to_sq64);
			}
			else
			if (!capturesOnly)
			{
				// no attack and no protect
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from_sq120, to_sq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleWhiteBishops (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = pBoard->bb64_wBishops;
	C_BB_64 bb64_possibility;

	while (bb64 != 0)
	{
		int   from_sq64 = bb64.BitPop();
		int   from_sq120 = SQ120(from_sq64);

		bb64_possibility = C_MOVEGEN::mgDAndAntiDMoves(pBoard->bb64_OccupiedSQs, from_sq64);      // & (bb64_EmptySQs|bb64_bPiecesSQs)

		while (bb64_possibility != 0)
		{
			int to_sq64  = bb64_possibility.BitPop();
			int to_sq120 = SQ120(to_sq64);

			if ((pBoard->bb64_bPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// black piece attacked                                                                                                                         
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from_sq120, to_sq120, pBoard->pieces[to_sq120], EMPTY, 0));
				pBoard->bb64_wBishopAttks.SetBit(to_sq64);
			}
			else
			if ((pBoard->bb64_wPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// white piece protected
				if (pBoard->pieces[to_sq120] != wK)
					pBoard->bb64_wBishopProts.SetBit(to_sq64);
			}
			else
			if (!capturesOnly)
			{
				// no attack and no protect
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from_sq120, to_sq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleWhiteCastle  (C_BOARD *pBoard, S_MOVELIST  *movelist                                               )
{
#pragma region Handle Possible WHITE Castle Moves(King Side)
	if ((pBoard->castlePerm & WK_SIDECASTLING) != 0)
	{
		if ((pBoard->bb64_OccupiedSQs & (SetMask[SQ64_F1] | SetMask[SQ64_G1])) == 0)
		{
			if (   !C_MOVEGEN::mgSqAttacked(pBoard, E1, BLACK) 
				&& !C_MOVEGEN::mgSqAttacked(pBoard, F1, BLACK) 
				&& !C_MOVEGEN::mgSqAttacked(pBoard, G1, BLACK) 
			   )
			{
				ASSERT(pBoard->pieces[E1] == wK);
				ASSERT(pBoard->pieces[F1] == EMPTY);
				ASSERT(pBoard->pieces[G1] == EMPTY);
				ASSERT(pBoard->pieces[H1] == wR);
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(E1, G1, EMPTY, EMPTY, MVFLAG_Castle));
			}
		}
	}
#pragma endregion
#pragma region Handle Possible WHITE Castle Move(Queen Side)
	if ((pBoard->castlePerm & WQ_SIDECASTLING) != 0)
	{
		if ((pBoard->bb64_OccupiedSQs & (SetMask[SQ64_D1] | SetMask[SQ64_C1] | SetMask[SQ64_B1])) == 0)
		{
			if (   !C_MOVEGEN::mgSqAttacked(pBoard, E1, BLACK) 
				&& !C_MOVEGEN::mgSqAttacked(pBoard, D1, BLACK) 
				&& !C_MOVEGEN::mgSqAttacked(pBoard, C1, BLACK)
			   )
			{
				ASSERT(pBoard->pieces[E1] == wK);
				ASSERT(pBoard->pieces[D1] == EMPTY);
				ASSERT(pBoard->pieces[C1] == EMPTY);
				ASSERT(pBoard->pieces[B1] == EMPTY);
				ASSERT(pBoard->pieces[A1] == wR);
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(E1, C1, EMPTY, EMPTY, MVFLAG_Castle));
			}
		}
	}
#pragma endregion
}
#pragma endregion

#pragma region Handle Black Pieces
void  C_MOVEGEN::mgHandleBlackPawns   (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = pBoard->bb64_bPawns;

	while (bb64 != 0)
	{
		int sq120 = SQ120(bb64.BitPop());
		ASSERT(pBoard->pieces[sq120] == bP);


		if (pBoard->pieces[sq120 - 10] == EMPTY && !capturesOnly)
		{
			C_MOVEGEN::mgAddBlackPawnMove(pBoard,movelist, sq120, sq120 - 10);
			if (RanksBrd[sq120] == RANK_7 && pBoard->pieces[sq120 - 20] == EMPTY)
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(sq120, (sq120 - 20), EMPTY, EMPTY, MVFLAG_PawnStart));
		}

		for (int tosq120 : { sq120 - 9, sq120 - 11 })
		{
			if (!SQOFFBOARD(tosq120))
			{
				if ((pBoard->bb64_wPiecesSQs & SetMask[SQ64(tosq120)]) != 0) // white piece attacked
				{
					C_MOVEGEN::mgAddBlackPawnCapMove(pBoard, movelist, sq120, tosq120, pBoard->pieces[tosq120]);
					pBoard->bb64_bPawnAttks.SetBit(SQ64(tosq120));
				}
				else
				if ((pBoard->bb64_bPiecesSQs & SetMask[SQ64(tosq120)]) != 0) // black piece protected
				{
					pBoard->bb64_bPawnProts.SetBit(SQ64(tosq120));
				}
			}

		}

		if (pBoard->SQenPas120 != NO_EP_SQ)
		{
			if (sq120 - 9 == pBoard->SQenPas120)
			{
				C_MOVEGEN::mgAddEnPassantMove(pBoard, movelist, MOVE(sq120, sq120 - 9, EMPTY, EMPTY, MVFLAG_EnPassant));
				pBoard->bb64_bPawnAttks.SetBit(SQ64(sq120 - 9));
			}
			else
				if (sq120 - 11 == pBoard->SQenPas120)
				{
					C_MOVEGEN::mgAddEnPassantMove(pBoard, movelist, MOVE(sq120, sq120 - 11, EMPTY, EMPTY, MVFLAG_EnPassant));
					pBoard->bb64_bPawnAttks.SetBit(SQ64(sq120 - 11));
				}
		}
	}
}
void  C_MOVEGEN::mgHandleBlackKnights (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64_possibility;
	C_BB_64 bb64 = pBoard->bb64_bKnights;

	while (bb64 != 0)
	{
		int sq64 = bb64.BitPop();
		int sq120 = SQ120(sq64);

		bb64_possibility = KnightSpans[sq64];

		while (bb64_possibility != 0)
		{
			int tosq64 = bb64_possibility.BitPop();
			int tosq120 = SQ120(tosq64);

			if ((pBoard->bb64_wPiecesSQs & SetMask[tosq64]) != 0)
			{
				// white piece attacked
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(sq120, tosq120, pBoard->pieces[tosq120], EMPTY, 0));
				pBoard->bb64_bKnightAttks.SetBit(SQ64(tosq120));
			}
			else
			if ((pBoard->bb64_bPiecesSQs & SetMask[tosq64]) != 0)
			{
				// black piece protected
				if (pBoard->pieces[tosq120] != bK)
					pBoard->bb64_bKnightProts.SetBit(SQ64(tosq120));
			}
			else
			if (!capturesOnly)
			{
				// no attack and no protect
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(sq120, tosq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleBlackKing    (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64_possibility;
	C_BB_64 bb64 = C_BB_64(pBoard->bb64_bKings);

	while (bb64 != 0)
	{
		int sq64 = bb64.BitPop();
		int sq120 = SQ120(sq64);
		ASSERT(pBoard->bb64_bKings.BitCount() == 1);
		ASSERT(bb64.BitCount() == 0);

		bb64_possibility = KingSpans[sq64];

		while (bb64_possibility != 0)
		{
			int tosq64 = bb64_possibility.BitPop();
			int tosq120 = SQ120(tosq64);

			if ((pBoard->bb64_wPiecesSQs & SetMask[tosq64]) != 0)
			{   // white piece attacked
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(sq120, tosq120, pBoard->pieces[tosq120], EMPTY, 0));
				pBoard->bb64_bKingAttks.SetBit(SQ64(tosq120));
			}
			else
			if ((pBoard->bb64_bPiecesSQs & SetMask[tosq64]) != 0)
			{   // black piece protected
				pBoard->bb64_bKingProts.SetBit(SQ64(tosq120));
			}
			else
			if (!capturesOnly)
			{
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(sq120, tosq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleBlackQueens  (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = pBoard->bb64_bQueens;
	C_BB_64 bb64_possibility;

	while (bb64 != 0)
	{
		int   from_sq64 = bb64.BitPop();
		int   from_sq120 = SQ120(from_sq64);

		bb64_possibility = C_MOVEGEN::mgHAndVMoves    (pBoard->bb64_OccupiedSQs, from_sq64)      // & (bb64_EmptySQs|bb64_wPiecesSQs) 
			             | C_MOVEGEN::mgDAndAntiDMoves(pBoard->bb64_OccupiedSQs, from_sq64);     // & (bb64_EmptySQs|bb64_wPiecesSQs); 

		while (bb64_possibility != 0)
		{
			int to_sq64 = bb64_possibility.BitPop();
			int to_sq120 = SQ120(to_sq64);

			if ((pBoard->bb64_wPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// white piece attacked                                                                                                                         
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from_sq120, to_sq120, pBoard->pieces[to_sq120], EMPTY, 0));
				pBoard->bb64_bQueenAttks.SetBit(to_sq64);
			}
			else
			if ((pBoard->bb64_bPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// black piece protected
				if (pBoard->pieces[to_sq120] != bK)
					pBoard->bb64_bQueenProts.SetBit(to_sq64);
			}
			else
			if (!capturesOnly)
			{
				// no attack and no protect
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from_sq120, to_sq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleBlackRooks   (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = pBoard->bb64_bRooks;
	C_BB_64 bb64_possibility;

	while (bb64 != 0)
	{
		int   from_sq64 = bb64.BitPop();
		int   from_sq120 = SQ120(from_sq64);

		bb64_possibility = C_MOVEGEN::mgHAndVMoves(pBoard->bb64_OccupiedSQs, from_sq64);      // & (bb64_EmptySQs|bb64_wPiecesSQs)

		while (bb64_possibility != 0)
		{
			int to_sq64 = bb64_possibility.BitPop();
			int to_sq120 = SQ120(to_sq64);

			if ((pBoard->bb64_wPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// white piece attacked                                                                                                                         
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from_sq120, to_sq120, pBoard->pieces[to_sq120], EMPTY, 0));
				pBoard->bb64_bRookAttks.SetBit(to_sq64);
			}
			else
				if ((pBoard->bb64_bPiecesSQs & SetMask[to_sq64]) != 0)
				{
					// black piece protected
					if (pBoard->pieces[to_sq120] != bK)
						pBoard->bb64_bRookProts.SetBit(to_sq64);
				}
				else
				if (!capturesOnly)
				{
					// no attack and no protect
					C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from_sq120, to_sq120, EMPTY, EMPTY, 0));
				}
		}
	}
}
void  C_MOVEGEN::mgHandleBlackBishops (C_BOARD *pBoard, S_MOVELIST  *movelist, const bool capturesOnly                      )
{
	C_BB_64 bb64 = pBoard->bb64_bBishops;
	C_BB_64 bb64_possibility;

	while (bb64 != 0)
	{
		int   from_sq64 = bb64.BitPop();
		int   from_sq120 = SQ120(from_sq64);

		bb64_possibility = C_MOVEGEN::mgDAndAntiDMoves(pBoard->bb64_OccupiedSQs, from_sq64);      // & (bb64_EmptySQs|bb64_wPiecesSQs)

		while (bb64_possibility != 0)
		{
			int to_sq64 = bb64_possibility.BitPop();
			int to_sq120 = SQ120(to_sq64);

			if ((pBoard->bb64_wPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// white piece attacked                                                                                                                         
				C_MOVEGEN::mgAddCaptureMove(pBoard, movelist, MOVE(from_sq120, to_sq120, pBoard->pieces[to_sq120], EMPTY, 0));
				pBoard->bb64_bBishopAttks.SetBit(to_sq64);
			}
			else
			if ((pBoard->bb64_bPiecesSQs & SetMask[to_sq64]) != 0)
			{
				// black piece protected
				if (pBoard->pieces[to_sq120] != bK)
					pBoard->bb64_bBishopProts.SetBit(to_sq64);
			}
			else
			if (!capturesOnly)
			{
				// no attack and no protect
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(from_sq120, to_sq120, EMPTY, EMPTY, 0));
			}
		}
	}
}
void  C_MOVEGEN::mgHandleBlackCastle  (C_BOARD *pBoard, S_MOVELIST  *movelist                                               )
{
	#pragma region Handle Possible WHITE Castle Move(Queen Side)
	if ((pBoard->castlePerm & BK_SIDECASTLING) != 0)
	{
		if ((pBoard->bb64_OccupiedSQs & (SetMask[SQ64_F8] | SetMask[SQ64_G8])) == 0)
		{
			if (   !C_MOVEGEN::mgSqAttacked(pBoard, E8, WHITE) 
				&& !C_MOVEGEN::mgSqAttacked(pBoard, F8, WHITE)
				&& !C_MOVEGEN::mgSqAttacked(pBoard, G8, WHITE)
			   )
			{
				ASSERT(pBoard->pieces[E8] == bK);
				ASSERT(pBoard->pieces[F8] == EMPTY);
				ASSERT(pBoard->pieces[G8] == EMPTY);
				ASSERT(pBoard->pieces[H8] == bR);
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(E8, G8, EMPTY, EMPTY, MVFLAG_Castle));
			}
		}
	}
	#pragma endregion

	#pragma region Handle Possible BLACK Castle Move(Queen Side)
	if ((pBoard->castlePerm & BQ_SIDECASTLING) != 0)
	{
		if ((pBoard->bb64_OccupiedSQs & (SetMask[SQ64_D8] | SetMask[SQ64_C8] | SetMask[SQ64_B8])) == 0)
		{
			if (   !C_MOVEGEN::mgSqAttacked(pBoard, E8, WHITE) 
				&& !C_MOVEGEN::mgSqAttacked(pBoard, D8, WHITE) 
				&& !C_MOVEGEN::mgSqAttacked(pBoard, C8, WHITE)
			   )
			{
				ASSERT(pBoard->pieces[E8] == bK);
				ASSERT(pBoard->pieces[D8] == EMPTY);
				ASSERT(pBoard->pieces[C8] == EMPTY);
				ASSERT(pBoard->pieces[B8] == EMPTY);
				ASSERT(pBoard->pieces[A8] == bR);
				C_MOVEGEN::mgAddQuietMove(pBoard, movelist, MOVE(E8, C8, EMPTY, EMPTY, MVFLAG_Castle));
			}
		}
	}
	#pragma endregion

}
#pragma endregion


void  C_MOVEGEN::mgGenerateMoves      (C_BOARD *pBoard, S_MOVELIST *movelist, const bool capturesOnly)
{
	ASSERT(pBoard->CheckBoard());
	movelist->Clear();
	pBoard->bb64_wPiecesSQs  = pBoard->bb64_wPawns     | pBoard->bb64_wKnights | pBoard->bb64_wBishops | pBoard->bb64_wRooks | pBoard->bb64_wQueens | pBoard->bb64_wKings;
	pBoard->bb64_bPiecesSQs  = pBoard->bb64_bPawns     | pBoard->bb64_bKnights | pBoard->bb64_bBishops | pBoard->bb64_bRooks | pBoard->bb64_bQueens | pBoard->bb64_bKings;
	pBoard->bb64_OccupiedSQs = pBoard->bb64_wPiecesSQs | pBoard->bb64_bPiecesSQs;
	pBoard->bb64_EmptySQs    = ~pBoard->bb64_OccupiedSQs;

	if (pBoard->side == WHITE)
	{
		pBoard->bb64_wPawnAttks   = 0;  pBoard->bb64_wPawnProts   = 0;
		pBoard->bb64_wKnightAttks = 0;  pBoard->bb64_wKnightProts = 0;
		pBoard->bb64_wBishopAttks = 0;  pBoard->bb64_wBishopProts = 0;
		pBoard->bb64_wRookAttks   = 0;  pBoard->bb64_wRookProts   = 0;
		pBoard->bb64_wQueenAttks  = 0;  pBoard->bb64_wQueenProts  = 0;
		pBoard->bb64_wKingAttks   = 0;  pBoard->bb64_wKingProts   = 0;
		pBoard->wTotalAttks       = 0;  pBoard->wTotalProts       = 0;

		#pragma region Generate White Moves
		// TODO: Generate Move Threads
		//       Following thread calls work:
		//            vector<thread> threads; 
		//               :
		//            threads.emplace_back(thread(C_MOVEGEN::mgHandleWhitePawns,pBoard, movelist, capturesOnly));    
		//               :
		//            for (int i=0; i<threads.size(); i++)
		//    	          threads[i].join();
		//       BUT are very slow (too much thread startup overhead)
		//       boost thred call 'THREAD_START(boost::bind(C_MOVEGEN::mgHandleWhitePawns  , pBoard, movelist, capturesOnly));  ' has same problem  -- too slow 
		//
		//       I need to:
		//            - build & start a thread for each 'mgHandlexxxxx' which never end
		//            - we just send a msg (as appropriate) to threads with the pBoard pointer and get a result sent back 
		//            - once all result are back we continue
		//

		//for (int t = wP; t <= wK; t++) C_MOVEGEN::mgHandler(pBoard, movelist, capturesOnly, t);
		C_MOVEGEN::mgHandleWhitePawns  ( pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleWhiteRooks  ( pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleWhiteKnights( pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleWhiteBishops( pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleWhiteQueens ( pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleWhiteKing   ( pBoard, movelist, capturesOnly);
		if (!capturesOnly)
			C_MOVEGEN::mgHandleWhiteCastle(pBoard,movelist);

	

		pBoard->bb64_wPiecesAttksSQs = pBoard->bb64_wPawnAttks | pBoard->bb64_wKnightAttks | pBoard->bb64_wBishopAttks | pBoard->bb64_wRookAttks | pBoard->bb64_wQueenAttks | pBoard->bb64_wKingAttks;
		pBoard->bb64_wPiecesProtsSQs = pBoard->bb64_wPawnProts | pBoard->bb64_wKnightProts | pBoard->bb64_wBishopProts | pBoard->bb64_wRookProts | pBoard->bb64_wQueenProts | pBoard->bb64_wKingProts;

		pBoard->wTotalAttks = pBoard->bb64_wPawnAttks.BitCount() + pBoard->bb64_wKnightAttks.BitCount() + pBoard->bb64_wBishopAttks.BitCount() + pBoard->bb64_wRookAttks.BitCount() + pBoard->bb64_wQueenAttks.BitCount() + pBoard->bb64_wKingAttks.BitCount();
		pBoard->wTotalProts = pBoard->bb64_wPawnProts.BitCount() + pBoard->bb64_wKnightProts.BitCount() + pBoard->bb64_wBishopProts.BitCount() + pBoard->bb64_wRookProts.BitCount() + pBoard->bb64_wQueenProts.BitCount() + pBoard->bb64_wKingProts.BitCount();
		#pragma endregion
	}
	else
	{
		pBoard->bb64_bPawnAttks   = 0;  pBoard->bb64_bPawnProts   = 0;
		pBoard->bb64_bKnightAttks = 0;  pBoard->bb64_bKnightProts = 0;
		pBoard->bb64_bBishopAttks = 0;  pBoard->bb64_bBishopProts = 0;
		pBoard->bb64_bRookAttks   = 0;  pBoard->bb64_bRookProts   = 0;
		pBoard->bb64_bQueenAttks  = 0;  pBoard->bb64_bQueenProts  = 0;
		pBoard->bb64_bKingAttks   = 0;  pBoard->bb64_bKingProts   = 0;
		pBoard->bTotalAttks       = 0;  pBoard->bTotalProts       = 0;

		#pragma region Generate Black Moves
		C_MOVEGEN::mgHandleBlackPawns  (pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleBlackRooks  (pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleBlackKnights(pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleBlackBishops(pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleBlackQueens (pBoard, movelist, capturesOnly);
		C_MOVEGEN::mgHandleBlackKing   (pBoard, movelist, capturesOnly);

		if (!capturesOnly)
			C_MOVEGEN::mgHandleBlackCastle(pBoard,movelist);

		pBoard->bb64_bPiecesAttksSQs = pBoard->bb64_bPawnAttks | pBoard->bb64_bKnightAttks | pBoard->bb64_bBishopAttks | pBoard->bb64_bRookAttks | pBoard->bb64_bQueenAttks | pBoard->bb64_bKingAttks;
		pBoard->bb64_bPiecesProtsSQs = pBoard->bb64_bPawnProts | pBoard->bb64_bKnightProts | pBoard->bb64_bBishopProts | pBoard->bb64_bRookProts | pBoard->bb64_bQueenProts | pBoard->bb64_bKingProts;

		pBoard->bTotalAttks = pBoard->bb64_bPawnAttks.BitCount() + pBoard->bb64_bKnightAttks.BitCount() + pBoard->bb64_bBishopAttks.BitCount() + pBoard->bb64_bRookAttks.BitCount() + pBoard->bb64_bQueenAttks.BitCount() + pBoard->bb64_bKingAttks.BitCount();
		pBoard->bTotalProts = pBoard->bb64_bPawnProts.BitCount() + pBoard->bb64_bKnightProts.BitCount() + pBoard->bb64_bBishopProts.BitCount() + pBoard->bb64_bRookProts.BitCount() + pBoard->bb64_bQueenProts.BitCount() + pBoard->bb64_bKingProts.BitCount();
		#pragma endregion
	}

	if (highestmovecount < movelist->size)
		highestmovecount = movelist->size;
}
