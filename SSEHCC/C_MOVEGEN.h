#pragma once

#define HASH_PCE(pb,pce,sq)    (pb->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA(pb)            (pb->posKey ^= (CastleKeys[(castlePerm)]))
#define HASH_SIDE(pb)          (pb->posKey ^= (SideKey))
#define HASH_EP(pb,SQenPas120) (pb->posKey ^= (PieceKeys[EMPTY][(SQenPas120)]))	

class C_MOVEGEN
{
public:
	#pragma region Magic Move Generation Routines
	static C_BB_64 mgBlockermask_rook    (C_BOARD *pBoard, int sq64);
	static C_BB_64 mgBlockermask_bishop  (C_BOARD *pBoard, int sq64);
	static C_BB_64 mgMoveboard_rook      (int sq64, U64 blockerboard);
	static C_BB_64 mgMoveboard_bishop    (int sq64, U64 blockerboard);
	static C_BB_64 mgBlockerboard        (int index, U64 blockermask);
	#pragma endregion

	static C_BB_64 mgDAndAntiDMoves      (C_BB_64 bb64_occupiedSQs, const int sq64);
	static C_BB_64 mgHAndVMoves          (C_BB_64 bb64_occupiedSQs, const int sq64);

	static bool    mgSqAttacked          (C_BOARD *pBoard, int sq120, int sideToNotMove);
				  
	static void    mgAddQuietMove        (C_BOARD *pBoard, S_MOVELIST *movelist, int move);
    static void    mgAddCaptureMove      (C_BOARD *pBoard, S_MOVELIST *movelist, const int move);
    static void    mgAddEnPassantMove    (C_BOARD *pBoard, S_MOVELIST *movelist, const int move);
	static void    mgAddWhitePawnCapMove (C_BOARD *pBoard, S_MOVELIST *movelist, const int from, const int to, const int cap);
	static void    mgAddWhitePawnMove    (C_BOARD *pBoard, S_MOVELIST *movelist, const int from, const int to);
	static void    mgAddBlackPawnCapMove (C_BOARD *pBoard, S_MOVELIST *movelist, int from, int to, int cap);
	static void    mgAddBlackPawnMove    (C_BOARD *pBoard, S_MOVELIST *movelist, int from, int to);
				  
	static void    mgHandleWhitePawns    (C_BOARD *pBoard, S_MOVELIST *movelist, const bool capturesOnly);
	static void    mgHandleBlackPawns    (C_BOARD *pBoard, S_MOVELIST *movelist, const bool capturesOnly);
	static void    mgHandleWhiteKnights  (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleWhiteKing     (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleWhiteQueens   (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleWhiteRooks    (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleWhiteBishops  (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleWhiteCastle   (C_BOARD *pBoard, S_MOVELIST *movelist);
	static void    mgHandleBlackKnights  (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleBlackKing     (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleBlackQueens   (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleBlackRooks    (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleBlackBishops  (C_BOARD *pBoard, S_MOVELIST *movelist, bool capturesOnly);
	static void    mgHandleBlackCastle   (C_BOARD *pBoard, S_MOVELIST *movelist);
				   
	static void    mgGenerateMoves       (C_BOARD *pBoard, S_MOVELIST *movelist, const bool capturesOnly);

};

