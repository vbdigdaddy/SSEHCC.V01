#pragma once
#include "stdafx.h"
#include "C_BOARD.h"
#include "CS_EVALUATE.h"

	CS_EVALUATE::CS_EVALUATE() 
	{
		ENDGAME_MAT           = (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK]);

		InitDistanceBonus();
	};
    CS_EVALUATE& CS_EVALUATE::getInstance()
    {
        static CS_EVALUATE instance; // the one and only instance
        return instance;
    }

	// TODO: Distance Bonus info not used yet
	void CS_EVALUATE::InitDistanceBonus      ()
	{
		for (int i = 0; i < 64; i++)
		{
			int col_i = i / 8;
			int row_i = i % 8;
			for (int j = 0; j < 64; j++)
			{
				distBonus[i][j] = 14 - DIST(i,j);
				distKQ[i][j]  = (distBonus[i][j] * 5) / 2;
				distKR[i][j]  =  distBonus[i][j] / 2;
				distKN[i][j]  =  distBonus[i][j];
				/* bk_dist[i][j] takes into account the numbers of the diagonals */
				distKB[i][j]  = distBonus[i][j] / 2;
				distKB[i][j] += distDiagBonus[abs(diag_ne[i] - diag_ne[j])];
				distKB[i][j] += distDiagBonus[abs(diag_nw[i] - diag_nw[j])];
			}
		}
	}

    int  CS_EVALUATE::ManhattanCenterDistance(int sq64)
    { // better known as the "Manhattan Center Distance"
        int f = sq64 &  7;
        int r = sq64 >> 3;
        f ^= (f-4) >> 8;
        r ^= (r-4) >> 8;
        return (f+r) & 7;
    }
    int  CS_EVALUATE::KingToSideBias         (C_BOARD *pBoard)
    {
       return 4* ManhattanCenterDistance(pBoard->KINGSQ64()) + (14-DIST(pBoard->KINGSQ64(),pBoard->OTHERKINGSQ64()));
    }
    int  CS_EVALUATE::CenterControlBias      (C_BOARD *pBoard)
    {
       return 0;
    }
	int  CS_EVALUATE::MaterialDraw           (C_BOARD *pBoard)
	{
		// Check for - no Pawns and Material Draw

		ASSERT(pBoard->CheckBoard());

        Total_Pawns  =  pBoard->bb64_wPawns  .BitCount() + pBoard->bb64_bPawns  .BitCount();
        Total_RQ     =  pBoard->bb64_wRooks  .BitCount() + pBoard->bb64_bRooks  .BitCount() + pBoard->bb64_wQueens .BitCount() + pBoard->bb64_bQueens .BitCount();
        Total_NB     =  pBoard->bb64_wKnights.BitCount() + pBoard->bb64_bKnights.BitCount() + pBoard->bb64_wBishops.BitCount() + pBoard->bb64_bBishops.BitCount();
        Total_Pieces = Total_Pawns + Total_RQ + Total_NB + 2;  // 2 kings of course

        if ( Total_Pawns != 0)
			return false;

		if (Total_RQ == 0)
		{
			#pragma region no rooks or queens on board
			if ( (pBoard->bb64_bBishops | pBoard->bb64_wBishops) == 0 )
			{ // no bishops, count knights
				if ( pBoard->bb64_wKnights.BitCount() < 3 && pBoard->bb64_bKnights.BitCount() <  3 )
					return true;
			}
			else
			if ( (pBoard->bb64_wKnights | pBoard->bb64_bKnights) == 0)
				{ // no knights, count bishops
					if ( abs(pBoard->bb64_wBishops.BitCount()-pBoard->bb64_bBishops.BitCount()) < 2 )
						return true;
				}
			else
			if (    (pBoard->bb64_wKnights.BitCount()<3  && pBoard->bb64_wBishops==0 ) || (pBoard->bb64_wBishops.BitCount()==1 && pBoard->bb64_wKnights==0) )
			{
				if ((pBoard->bb64_bKnights.BitCount()<3  && pBoard->bb64_bBishops==0 ) || (pBoard->bb64_bBishops.BitCount()==1 && pBoard->bb64_bKnights==0) )
					return true;
			}
			#pragma endregion
		}
		else
		if ( (pBoard->bb64_wQueens | pBoard->bb64_bQueens) == 0)
		{ // no queens on board
			if ( (pBoard->bb64_wRooks.BitCount())==1 && pBoard->bb64_bRooks.BitCount()==1 )
			{
				if ( (pBoard->bb64_wKnights.BitCount() + pBoard->bb64_wBishops.BitCount()) < 2 && (pBoard->bb64_bKnights.BitCount() + pBoard->bb64_bBishops.BitCount()) < 2 )
					return true;
			}
			else
			if ( pBoard->bb64_wRooks.BitCount()==1 && pBoard->bb64_bRooks.BitCount()!=0 )
			{
				if (  ( ((pBoard->bb64_wKnights.BitCount()+pBoard->bb64_wBishops.BitCount()) == 0)                                                                               )
				   && ( ((pBoard->bb64_bKnights.BitCount()+pBoard->bb64_bBishops.BitCount()) == 1) || ((pBoard->bb64_bKnights.BitCount()+pBoard->bb64_bBishops.BitCount()) == 2) )
				   )
				   return true;
			}
			else
			if (    pBoard->bb64_bRooks.BitCount() == 1
				 && pBoard->bb64_wRooks.BitCount() != 0
			   )
			{
				if (  ( ( (pBoard->bb64_bKnights.BitCount()+pBoard->bb64_bBishops.BitCount())==0)                                                                              )
					&&( ( (pBoard->bb64_wKnights.BitCount()+pBoard->bb64_wBishops.BitCount())==1) || (pBoard->bb64_wKnights.BitCount() + pBoard->bb64_wBishops.BitCount()) == 2) 
				   )
				   return true;
			}
		}
		return false;
	}
	int  CS_EVALUATE::EvalPosition(C_BOARD *pBoard)
	{
		if (MaterialDraw(pBoard))
			return 0;

		ASSERT(pBoard->bb64_wKings.BitCount() == 1);
		int result = 0;

		if (ENDGAME(pBoard))	result = EvalPositionEndGame   (pBoard);  else
		if (MIDDLEGAME(pBoard))	result = EvalPositionMiddleGame(pBoard);  else
		if (BEGINGAME(pBoard))	result = EvalPositionBeginGame (pBoard);  else
			                    result = EvalPositionGeneral   (pBoard);

		ASSERT(pBoard->bb64_wKings.BitCount() == 1);
		ASSERT(result >= -CHESSINFINITE && result < CHESSINFINITE);
		if (result < -CHESSINFINITE || result > CHESSINFINITE) PUNT("bad eval");
		return result;
	}
    int  CS_EVALUATE::EvalPositionBeginGame  (C_BOARD *pBoard)
	{ 
		return EvalPositionGeneral(pBoard);                             
	}
    int  CS_EVALUATE::EvalPositionMiddleGame (C_BOARD *pBoard)
	{ 
		return EvalPositionGeneral(pBoard) + CenterControlBias(pBoard); 
	}
    int  CS_EVALUATE::EvalPositionEndGame    (C_BOARD *pBoard)
    { 
		int result = 0;

        if (Total_Pieces == 3 && Total_Pawns == 0)
        {  
           #pragma region must be some form of Kxk (where x is Queen or Rook or MaterialDraw would have been caught earlier)
           ASSERT(Total_RQ == 1);
           if ( pBoard->bb64_wRooks != 0 )
           { // KRk or Krk (TODO: Evaluation can be improved)
			   if ( pBoard->side == BLACK )
                  result = EvalPositionGeneral(pBoard) + ManhattanCenterDistance(pBoard->OTHERKINGSQ64())*200;  // try to force other king to side
               else
                  result = EvalPositionGeneral(pBoard);  
			   ASSERT(result >= -CHESSINFINITE && result < CHESSINFINITE);
			   return result;
		   }
           else
           { // KQk or Kqk (TODO: Evaluation can be improved)
               ASSERT( pBoard->bb64_bRooks==0 || pBoard->bb64_bQueens!=0 );
               if ( pBoard->side == WHITE )
                  result = EvalPositionGeneral(pBoard) + ManhattanCenterDistance(pBoard->OTHERKINGSQ64())*200;  // try to force other king to side
               else
                  result = EvalPositionGeneral(pBoard);  
			   ASSERT(result >= -CHESSINFINITE && result < CHESSINFINITE);
			   return result;
		   }
           #pragma endregion
        }
        
        if ( Total_Pieces == 4 )
        {  // Kxxk - TODO
           #pragma region must be some form of KxxK (where x is Pawn, Rook, Knight, Bishop, or Queen)
               ASSERT( (Total_RQ+Total_NB+Total_Pawns) == 2 );
			   result = EvalPositionGeneral(pBoard) + KingToSideBias(pBoard);
			   ASSERT(result >= -CHESSINFINITE && result < CHESSINFINITE);
			   return result;
#pragma endregion
        }

        // no specific patterns detected, return general endgame bias
		result = EvalPositionGeneral(pBoard);
		ASSERT(result >= -CHESSINFINITE && result < CHESSINFINITE);
		return result;
    }
    int  CS_EVALUATE::EvalPositionGeneral    (C_BOARD *pBoard)
	{
		ASSERT(pBoard->CheckBoard());
		C_BB_64  bb64;
		int    sq64;
		int    sq120;
		int    score = pBoard->material[WHITE] - pBoard->material[BLACK];

	    #pragma region Eval White Pawns
		bb64 = C_BB_64(pBoard->bb64_wPawns);
		while (bb64 != 0)
		{
			sq64  = bb64.BitPop();
			sq120 = SQ120(sq64);
			score += PawnTable[sq64];

			if ( (pBoard->bb64_wPawns & IsolatedMask[sq64]) == 0 )
			{
				// cout << "wP Iso:" << PrSq(sq120) << endl;
				score += PawnIsolated;
			}

			if ( (pBoard->bb64_bPawns & WhitePassedMask[sq64]) == 0 )
			{
				// cout << "wP Passed:" << PrSq(sq120) << endl;
				score += PawnPassed[RanksBrd[sq120]];
			}
		}
		#pragma endregion
        #pragma region Eval Black Pawns
		bb64 = C_BB_64(pBoard->bb64_bPawns);
		while (bb64 != 0)
		{
			sq64  = bb64.BitPop();
			sq120 = SQ120(sq64);
			score -= PawnTable[Mirror64[sq64]];

			if ( (pBoard->bb64_bPawns & IsolatedMask[sq64]) == 0 )
			{
				// cout << ("bP Iso:" << PrSq(sq120) << endl;
				score -= PawnIsolated;
			}

			if ( (pBoard->bb64_wPawns & BlackPassedMask[sq64]) == 0 )
			{
				// cout << "bP Passed:" << PrSq(sq120) << endl;
				score -= PawnPassed[7 - RanksBrd[sq120]];
			}
		}
        #pragma endregion

        #pragma region Eval White Knights
		bb64 = C_BB_64(pBoard->bb64_wKnights);
		while (bb64 != 0)
			score += KnightTable[bb64.BitPop()];
        #pragma endregion
		#pragma region Eval Black Knights
		bb64 = C_BB_64(pBoard->bb64_bKnights);
		while (bb64 != 0)
			score -= KnightTable[Mirror64[bb64.BitPop()]];
		#pragma endregion

		#pragma region Eval White Bishops
		bb64 = C_BB_64(pBoard->bb64_wBishops);
		if ( bb64.BitCount() >= 2)
			score += BishopPair;
		while (bb64 != 0)
			score += BishopTable[bb64.BitPop()];
		#pragma endregion
		#pragma region Eval Black Bishops
		bb64 = C_BB_64(pBoard->bb64_bBishops);
		if ( bb64.BitCount() >= 2)
			score -= BishopPair;
		while (bb64 != 0)
			score -= BishopTable[Mirror64[bb64.BitPop()]];
		#pragma endregion

		#pragma region Eval White Rooks
		bb64 = C_BB_64(pBoard->bb64_wRooks);
		while (bb64 != 0)
		{
			sq64  = bb64.BitPop();
			sq120 = SQ120(sq64);
			score += RookTable[sq64];

			if ( ((pBoard->bb64_wPawns | pBoard->bb64_bPawns) & FileBBMask[FilesBrd[sq120]]) == 0 )
			{
				score += RookOpenFile;
			}
			else
			if ( (pBoard->bb64_wPawns & FileBBMask[FilesBrd[sq120]]) == 0 )
			{
				score += RookSemiOpenFile;
			}
		}
		#pragma endregion
		#pragma region Eval Black Rooks
		bb64 = C_BB_64(pBoard->bb64_bRooks);
		while (bb64 != 0)
		{
			sq64  = bb64.BitPop();
			sq120 = SQ120(sq64);
			score -= RookTable[Mirror64[sq64]];

			if ( ((pBoard->bb64_wPawns | pBoard->bb64_bPawns) & FileBBMask[FilesBrd[sq120]]) == 0)
			{
				score -= RookOpenFile;
			}
			else
			if ( (pBoard->bb64_bPawns & FileBBMask[FilesBrd[sq120]]) == 0 )
			{
				score -= RookSemiOpenFile;
			}
		}
		#pragma endregion

		#pragma region Eval White Queens
		bb64 = C_BB_64(pBoard->bb64_wQueens);
		while (bb64 != 0)
		{
			sq64  = bb64.BitPop();
			sq120 = SQ120(sq64);
			if ( ((pBoard->bb64_wPawns | pBoard->bb64_bPawns) & FileBBMask[FilesBrd[sq120]]) == 0 )
			{
				score += QueenOpenFile;
			}
			else
			if ( (pBoard->bb64_wPawns & FileBBMask[FilesBrd[sq120]]) == 0)
			{
				score += QueenSemiOpenFile;
			}
		}
		#pragma endregion
		#pragma region Eval Black Queens
		bb64 = C_BB_64(pBoard->bb64_bQueens);
		while (bb64 != 0)
		{
			sq64  = bb64.BitPop();
			sq120 = SQ120(sq64);
			if ( ((pBoard->bb64_wPawns|pBoard->bb64_bPawns) & FileBBMask[FilesBrd[sq120]]) == 0 )
			{
				score -= QueenOpenFile;
			}
			else
			if ( (pBoard->bb64_bPawns & FileBBMask[FilesBrd[sq120]]) == 0 )
			{
				score -= QueenSemiOpenFile;
			}
		}
		#pragma endregion

        #pragma region Eval White King
		bb64 = C_BB_64(pBoard->bb64_wKings);
		sq64 = bb64.BitPop();
		ASSERT(pBoard->bb64_wKings.BitCount()==1);
		score += (pBoard->material[BLACK] <= ENDGAME_MAT) ? KingE[sq64] : KingO[sq64];
        #pragma endregion
        #pragma region Eval Black King
		bb64   = C_BB_64(pBoard->bb64_bKings);
		sq64   = bb64.BitPop();
		score -= (pBoard->material[WHITE] <= ENDGAME_MAT) ? KingE[Mirror64[sq64]] : KingO[Mirror64[sq64]];
        #pragma endregion

		#pragma region Eval Attks and Prots
		score += (pBoard->wTotalAttks - pBoard->bTotalAttks) + (pBoard->wTotalProts - pBoard->bTotalProts);
		#pragma endregion

		ASSERT(score >= -CHESSINFINITE && score < CHESSINFINITE);
		return (pBoard->side == WHITE) ? score : -score;

	}


