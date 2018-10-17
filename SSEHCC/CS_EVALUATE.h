#pragma once
#include "stdafx.h"

class CS_EVALUATE
{

protected:
	CS_EVALUATE();

private:
	int distBonus[BRD_SQ_NUM64][BRD_SQ_NUM64];  // "distBonus[i,j]" ::= distance between sq i and j  
	int distKQ   [BRD_SQ_NUM64][BRD_SQ_NUM64];
	int distKR   [BRD_SQ_NUM64][BRD_SQ_NUM64];
	int distKN   [BRD_SQ_NUM64][BRD_SQ_NUM64];
	int distKB   [BRD_SQ_NUM64][BRD_SQ_NUM64];

	int ENDGAME_MAT;
    int Total_Pawns       = 0; // Total Pawns
    int Total_Pieces      = 0; // Total Pieces
    int Total_RQ          = 0; // Total Rooks and queens
    int Total_NB          = 0; // Total Knights and Bishops

    #define ENDGAME(pBoard)       ( Total_Pieces <= 5 )
    #define BEGINGAME(pBoard)     ( pBoard->hisPly < 8 )
	#define MIDDLEGAME(pBoard)    ( !BEGINGAME(pBoard) && !ENDGAME(pBoard) )     

    const int diag_nw[BRD_SQ_NUM64] = {
	   0, 1, 2, 3, 4, 5, 6, 7,
	   1, 2, 3, 4, 5, 6, 7, 8,
	   2, 3, 4, 5, 6, 7, 8, 9,
	   3, 4, 5, 6, 7, 8, 9,10,
	   4, 5, 6, 7, 8, 9,10,11,
	   5, 6, 7, 8, 9,10,11,12,
	   6, 7, 8, 9,10,11,12,13,
	   7, 8, 9,10,11,12,13,14
	};
 	const int diag_ne[BRD_SQ_NUM64] = {
	   7, 6, 5, 4, 3, 2, 1, 0,
	   8, 7, 6, 5, 4, 3, 2, 1,
	   9, 8, 7, 6, 5, 4, 3, 2,
	  10, 9, 8, 7, 6, 5, 4, 3,
	  11,10, 9, 8, 7, 6, 5, 4,
	  12,11,10, 9, 8, 7, 6, 5,
	  13,12,11,10, 9, 8, 7, 6,
	  14,13,12,11,10, 9, 8, 7
	};
	const int distDiagBonus[15] = {5, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    #pragma region Piece Location Eval Tables

	const int PawnTable   [BRD_SQ_NUM64] =
	{
		 0,  0,  0,  0,  0,	 0,	 0,  0,
		10, 10,  0,-10,-10,	 0,	10, 10,
		 5,  0,  0,  5,  5,	 0,	 0,  5,
		 0,  0, 10,	20, 20, 10,	 0,  0,
		 5,  5,  5,	10, 10,  5,	 5,  5,
		10, 10, 10,	20, 20, 10,	10, 10,
		20, 20, 20,	30, 30, 20,	20, 20,
		 0,  0,  0,	 0,  0,	 0,	 0,  0
	};
	const int KnightTable [BRD_SQ_NUM64] =
	{
		0,-10,  0,  0,  0,  0,-10,  0,
		0,  0,  0,  5,  5,  0,  0,  0,
		0,  0, 10, 10, 10, 10,  0,  0,
		0,  0, 10, 20, 20, 10,  5,  0,
		5, 10, 15, 20, 20, 15, 10,  5,
		5, 10, 10, 20, 20, 10, 10,  5,
		0,  0,  5, 10, 10,  5,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0
	};
	const int BishopTable [BRD_SQ_NUM64] = {
		0,  0,-10,  0,  0,-10,  0,  0,
		0,  0,  0, 10, 10,  0,  0,  0,
		0,  0, 10, 15, 15, 10,  0,  0,
		0, 10, 15, 20, 20, 15, 10,  0,
		0, 10, 15, 20, 20, 15, 10,  0,
		0,  0, 10, 15, 15, 10,  0,  0,
		0,  0,  0, 10, 10,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0
	};
	const int RookTable   [BRD_SQ_NUM64] = {
		0,  0,	5, 10, 10,  5,  0,  0,
		0,  0,	5, 10, 10,  5,  0,  0,
		0,  0,	5, 10, 10,  5,  0,  0,
		0,  0,	5, 10, 10,  5,  0,  0,
		0,  0,	5, 10, 10,  5,  0,  0,
		0,  0,	5, 10, 10,  5,  0,  0,
		25, 25, 25, 25, 25, 25, 25, 25,
		0,  0,	5, 10, 10,  5,  0,  0
	};
	const int KingE       [BRD_SQ_NUM64] = {
		-50,-10,  0,  0,  0,  0,-10,-50,
		-10,  0, 10, 10, 10, 10,  0,-10,
		  0, 10, 20, 20, 20, 20, 10,  0,
		  0, 10, 20, 40, 40, 20, 10,  0,
		  0, 10, 20, 40, 40, 20, 10,  0,
		  0, 10, 20, 20, 20, 20, 10,  0,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-50,-10,  0,  0,  0,  0,-10,-50
	};
	const int KingO       [BRD_SQ_NUM64] = {
		  0,  5,  5,-10,-10,  0, 10,  5,
		-30,-30,-30,-30,-30,-30,-30,-30,
		-50,-50,-50,-50,-50,-50,-50,-50,
		-70,-70,-70,-70,-70,-70,-70,-70,
		-70,-70,-70,-70,-70,-70,-70,-70,
		-70,-70,-70,-70,-70,-70,-70,-70,
		-70,-70,-70,-70,-70,-70,-70,-70,
		-70,-70,-70,-70,-70,-70,-70,-70
	};

    #pragma endregion
    #pragma region Piece Structure Weigths

	const int PawnPassed[8]     = { 0, 5, 10, 20, 35, 60, 100, 200 };
	const int PawnIsolated      = -10;
	const int RookOpenFile      = 10;
	const int RookSemiOpenFile  = 5;
	const int QueenOpenFile     = 5;
	const int QueenSemiOpenFile = 3;
	const int BishopPair        = 30;

    #pragma endregion

	void InitDistanceBonus();
    
    int  ManhattanCenterDistance(int sq64);
    int  KingToSideBias         (C_BOARD *pBoard);
    int  CenterControlBias      (C_BOARD *pBoard);
                                  
    int  EvalPositionGeneral    (C_BOARD *pBoard);
    int  EvalPositionBeginGame  (C_BOARD *pBoard);
    int  EvalPositionMiddleGame (C_BOARD *pBoard);
    int  EvalPositionEndGame    (C_BOARD *pBoard);

public:
    static CS_EVALUATE& getInstance();

	int MaterialDraw(C_BOARD *pBoard);
	int EvalPosition(C_BOARD *pBoard);


};