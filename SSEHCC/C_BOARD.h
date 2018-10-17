#pragma once
#include "stdafx.h"
#include "C_BB_64.h"

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
#define SQOFFBOARD(sq120)   (FilesBrd[(sq120)]==OFFBOARD)


class C_BOARD
{

public: 

   	       C_BOARD                ();  // constructor
    	   ~C_BOARD               ();  // destructor

	#pragma region PUBLIC FUNCTIONS
		      
    int      KINGSQ64             ();
    int      KINGSQ120            ();
    int      OTHERKINGSQ64        ();
    int      OTHERKINGSQ120       ();
		     
	bool     MakeMove             (const int move);
    void     TakeMove             ();
	void     ClearPiece           (int sq120);
	void     AddPiece             (int sq120, int pce);
	void     MovePiece            (int fromsq120, int tosq120);
	void     MakeNullMove         ();
    void     TakeNullMove         ();
    bool     MoveExists           (const int move);
 	string   GetMoveDetail        (int move);
	int      GetPvLine            (C_BOARD *pBoard,DEPTH depth );
	void     PrintMoveList        (S_MOVELIST *movelist);
    bool     MoveListOk           (const S_MOVELIST *movelist);
		     
	void     PrintMoveHistory     (int n);
    int      SqAttackedKingSq     ();	
	void     UpdateListsMaterial  ();				        
	U64      GeneratePosKey       ();
	void     MirrorBoard          ();
	void     ResetBoard           ();
	bool     CheckBoard           ();
	int      ParseFen             (string fen);				        
	string   GetPieceDetail       (int sq);
	void     PrintBoard           ();
	void     PrintBoard           (string msg);
	C_BOARD *DeepClone            ();

	#pragma endregion

	#pragma region PUBLIC VARIABLES

	#pragma region BITBOARDS

	C_BB_64 bb64_OccupiedSQs;                    // Occupied Squares
	C_BB_64 bb64_EmptySQs;                       // Empty Squares
	C_BB_64 bb64_wPiecesSQs;                     // White Piece Squares
	C_BB_64 bb64_bPiecesSQs;                     // Black Piece Squares
	 							        
	C_BB_64 bb64_wPawns         , bb64_bPawns        ;	// bitboard for pawns   
	C_BB_64 bb64_wKnights       , bb64_bKnights      ;   // bitboard for rooks   
	C_BB_64 bb64_wBishops       , bb64_bBishops      ;   // bitboard for knights 
	C_BB_64 bb64_wRooks         , bb64_bRooks        ;   // bitboard for bishops 
	C_BB_64 bb64_wQueens        , bb64_bQueens       ;   // bitboard for queens  
	C_BB_64 bb64_wKings         , bb64_bKings        ;   // bitboard for kings   
						      
	C_BB_64 bb64_wPawnAttks     , bb64_bPawnAttks    ;   // bitboard for pawn   attacks
	C_BB_64 bb64_wKnightAttks   , bb64_bKnightAttks  ;   // bitboard for rook   attacks   
	C_BB_64 bb64_wBishopAttks   , bb64_bBishopAttks  ;   // bitboard for knight attacks 
	C_BB_64 bb64_wRookAttks     , bb64_bRookAttks    ;   // bitboard for bishop attacks 
	C_BB_64 bb64_wQueenAttks    , bb64_bQueenAttks   ;   // bitboard for queen  attacks  
	C_BB_64 bb64_wKingAttks     , bb64_bKingAttks    ;   // bitboard for king   attacks   
						      
	C_BB_64 bb64_wPawnProts     , bb64_bPawnProts    ;   // bitboard for pawn   Protects
	C_BB_64 bb64_wKnightProts   , bb64_bKnightProts  ;   // bitboard for rook   Protects   
	C_BB_64 bb64_wBishopProts   , bb64_bBishopProts  ;   // bitboard for knight Protects 
	C_BB_64 bb64_wRookProts     , bb64_bRookProts    ;   // bitboard for bishop Protects 
	C_BB_64 bb64_wQueenProts    , bb64_bQueenProts   ;   // bitboard for queen  Protects  
	C_BB_64 bb64_wKingProts     , bb64_bKingProts    ;   // bitboard for king   Protects   

	C_BB_64 bb64_wPiecesAttksSQs, bb64_bPiecesAttksSQs;
	C_BB_64 bb64_wPiecesProtsSQs, bb64_bPiecesProtsSQs;

	int wTotalAttks    , bTotalAttks;
	int wTotalProts    , bTotalProts;

#pragma endregion						        
											                             
	int              side;					        // current side to move
	int              ply;                           // 1/2 move count in move list
	int              hisPly;   	                    // 1/2 move count in history[] moves array
	int              SQenPas120;                    // enPassant SQ (sq to be taken by enpassant)
    U64              posKey;                        // position hash key
	int              fiftyMove;				        
	S_UNDO           history   [MAXGAMEMOVES];      // history of info need to undo move(s)
	int              material  [BOTH];
	int              pieces    [BRD_SQ_NUM120];	    // the board
	int              bigPce    [BOTH];		  	    // # all but pawns	
	int              PvArray   [MAX_PLY_DEPTH];

	int              searchHistory [MAXPIECETYPE][BRD_SQ_NUM120];
	int              searchKillers [2][MAX_PLY_DEPTH];

	int              castlePerm;					// Castle Permisions (See WK_SIDECASTLING for example) Bit Map

	int              majPce[BOTH];			        // # Rooks, Queens
	int              minPce[BOTH];                  // # Knights, Bishops

	#pragma endregion


private: 
	#pragma region PRIVATE VARIABLES

	const int  CastlePerm    [120] =  {
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
								    	15, 15, 15, 15, 15, 15, 15, 15, 15, 15
								      };


#pragma endregion

};

