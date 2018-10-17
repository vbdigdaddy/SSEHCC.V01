// init.cpp : main project file.
#include "stdafx.h"
#include "C_BOOKTABLE.h"
#include "C_MOVEGEN.h"
#include <direct.h>

mt19937 gen(314158265);  // seed it (note is 32 bit random number generator)

#define RAND_64 ( ((U64)gen()) | (((U64)gen())<<32) )

	const  int LoopSlidePce       [8] = { wB, wR, wQ, 0, bB, bR, bQ, 0 };
	const  int LoopNonSlidePce    [6] = { wN, wK, 0, bN, bK, 0 };
	const  int LoopSlideIndex     [2] = { 0, 4 };
	const  int LoopNonSlideIndex  [2] = { 0, 3 };
	
	const  int PceDir         [13][8] = {
		{ 0,   0,   0,   0,  0,   0,  0,  0 },
		{ 0,   0,   0,   0,  0,   0,  0,  0 },
		{ -8, -19, -21, -12,  8,  19, 21, 12 },
		{ -9, -11,  11,   9,  0,   0,  0,  0 },
		{ -1, -10, 	 1,  10,  0,   0,  0,  0 },
		{ -1, -10,	 1,  10, -9, -11, 11,  9 },
		{ -1, -10,	 1,  10, -9, -11, 11,  9 },
		{ 0,   0,   0,   0,  0,   0,  0,  0 },
		{ -8, -19, -21, -12,  8,  19, 21, 12 },
		{ -9, -11,  11,   9,  0,   0,  0,  0 },
		{ -1, -10,	 1,  10,  0,   0,  0,  0 },
		{ -1, -10,	 1,  10, -9, -11, 11,  9 },
		{ -1, -10,	 1,  10, -9, -11, 11,  9 }
	};
	
	const int PceDir_Knight      [8] = { -8, -19, -21, -12,  8,  19, 21, 12 };
	const int PceDir_Bishop      [4] = { -9, -11,  11,   9 };
	const int PceDir_Rook        [4] = { -1, -10,   1,  10 };
	const int PceDir_Queen       [8] = { -1, -10,   1,  10, -9, -11, 11,  9 };
	const int PceDir_King        [8] = { -1, -10,   1,  10, -9, -11, 11,  9 };
	  
	const int KnDir              [8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
	const int RkDir              [4] = { -1, -10,	1, 10 };
	const int BiDir              [4] = { -9, -11, 11, 9 };
	const int KiDir              [8] = { -1, -10,	1, 10, -9, -11, 11, 9 };
	 
	const int NumDir             [13] = { 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8 };
	  
	const int VictimScore        [13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };


#pragma region Piece Move Generation Variables
	//#pragma warning( disable: 4996)

#pragma region Bitboard File, Rank, Piece Spans, etc...
	const  U64 BB_FILE_A          = 0x0101010101010101; // (Bits 0+ 8+16+24+32+40+48+56)
	const  U64 BB_FILE_B          = 0x0202020202020202; // (Bits 1+ 9+17+25+33+41+49+57)
	const  U64 BB_FILE_C          = 0x0404040404040404; // (Bits 2+10+18+26+34+42+50+58)
	const  U64 BB_FILE_D          = 0x0808080808080808; // (Bits 3+11+19+27+35+43+51+59)
	const  U64 BB_FILE_E          = 0x1010101010101010; // (Bits 4+12+20+28+36+44+52+60)
	const  U64 BB_FILE_F          = 0x2020202020202020; // (Bits 5+13+21+39+37+45+53+61)
	const  U64 BB_FILE_G          = 0x4040404040404040; // (Bits 6+14+22+30+38+46+54+62)
	const  U64 BB_FILE_H          = 0x8080808080808080; // (Bits 7+15+23+31+39+47+55+63)
						         
	const  U64 BB_FILE_AB         = BB_FILE_A | BB_FILE_B;
	const  U64 BB_FILE_GH         = BB_FILE_G | BB_FILE_H;

	const  U64 BB_RANK_1          = 0x00000000000000FF;
	const  U64 BB_RANK_2          = 0x000000000000FF00;
	const  U64 BB_RANK_3          = 0x0000000000FF0000;
	const  U64 BB_RANK_4          = 0x00000000FF000000;
	const  U64 BB_RANK_5          = 0x000000FF00000000;
	const  U64 BB_RANK_6          = 0x0000FF0000000000;
	const  U64 BB_RANK_7          = 0x00FF000000000000;
	const  U64 BB_RANK_8          = 0xFF00000000000000;

	const  U64 BB_CENTRE          = 0x0000001818000000;
	const  U64 BB_EXTENDED_CENTRE = 0x00003C3C3C3C0000;
	const  U64 BB_KING_SIDE       = 0x0F0F0F0F0F0F0F0F;
	const  U64 BB_QUEEN_SIDE      = 0xF0F0F0F0F0F0F0F0;
	const  U64 BB_KING_SPAN       = 0x0000000000070507;
	const  U64 BB_KNIGHT_SPAN     = 0x0000000A1100110A;
#pragma endregion

#pragma region Bitboard DiagonalMasks8->from top left to bottom right
	const  U64 DiagonalMasks8[15] =
	{
		  0x0000000000000001
		, 0x0000000000000102
		, 0x0000000000010204
		, 0x0000000001020408
		, 0x0000000102040810
		, 0x0000010204081020
		, 0x0001020408102040
		, 0x0102040810204080
		, 0x0204081020408000
		, 0x0408102040800000
		, 0x0810204080000000
		, 0x1020408000000000
		, 0x2040800000000000
		, 0x4080000000000000
		, 0x8000000000000000
	};
#pragma endregion

#pragma region Bitboard AntiDiagonalMasks8->from top right to bottom left
	const  U64 AntiDiagonalMasks8[15] =
	{
		  0x0000000000000080
		, 0x0000000000008040
		, 0x0000000000804020
		, 0x0000000080402010
		, 0x0000008040201008
		, 0x0000804020100804
		, 0x0080402010080402
		, 0x8040201008040201
		, 0x4020100804020100
		, 0x2010080402010000
		, 0x1008040201000000
		, 0x0804020100000000
		, 0x0402010000000000
		, 0x0201000000000000
		, 0x0100000000000000
	};
#pragma endregion

#pragma region Bitboard RankMask8->from rank1 to rank8
	const U64 RankMasks8[8] =
	{
		BB_RANK_1
		, BB_RANK_2
		, BB_RANK_3
		, BB_RANK_4
		, BB_RANK_5
		, BB_RANK_6
		, BB_RANK_7
		, BB_RANK_8
	};
#pragma endregion

#pragma region Bitboard FileMask8->from fileA to FileH
	const U64 FileMasks8[8] =
	{
		BB_FILE_A
		, BB_FILE_B
		, BB_FILE_C
		, BB_FILE_D
		, BB_FILE_E
		, BB_FILE_F
		, BB_FILE_G
		, BB_FILE_H
	};
#pragma endregion




#pragma endregion


int FilesBrd       [BRD_SQ_NUM120];
int RanksBrd       [BRD_SQ_NUM120];
int Sq120ToSq64    [BRD_SQ_NUM120];
int Sq64ToSq120    [BRD_SQ_NUM64 ];

U64 SetMask        [BRD_SQ_NUM64];
U64 ClearMask      [BRD_SQ_NUM64];
U64 BlackPassedMask[BRD_SQ_NUM64];
U64 WhitePassedMask[BRD_SQ_NUM64];
U64 IsolatedMask   [BRD_SQ_NUM64];

U64 PieceKeys      [13][120];
U64 SideKey;
U64 CastleKeys     [16];
U64 FileBBMask     [8];
U64 RankBBMask     [8];

U64 KnightSpans    [BRD_SQ_NUM64];
U64 KingSpans      [BRD_SQ_NUM64];
int MvvLvaScores   [13][13];

S_OPTIONS EngineOptions[1];

U64 BlockerMasks_Rook  [BRD_SQ_NUM64];
U64 BlockerMasks_Bishop[BRD_SQ_NUM64];
U64 MagicIndexs_Rook   [BRD_SQ_NUM64];
U64 MagicIndexs_Bishop [BRD_SQ_NUM64];

void InitEvalMasks     () 
{

	int sq, tsq, r, f;

	for (sq = 0; sq < 8; ++sq)
	{
		FileBBMask[sq] = 0ULL;
		RankBBMask[sq] = 0ULL;
	}

	for (r = RANK_8; r >= RANK_1; r--) 
	{
		for (f = FILE_A; f <= FILE_H; f++) 
		{
			sq = r * 8 + f;
			FileBBMask[f] |= (1ULL << sq);
			RankBBMask[r] |= (1ULL << sq);
		}
	}

	for (U64 &isolatedMask    : IsolatedMask   ) isolatedMask    = 0ULL;
	for (U64 &whitePassedMask : WhitePassedMask) whitePassedMask = 0ULL;
	for (U64 &blackPassedMask : BlackPassedMask) blackPassedMask = 0ULL;

	for (sq = 0; sq < 64; ++sq) 
	{
		tsq = sq + 8;

		while (tsq < 64)
		{
			WhitePassedMask[sq] |= (1ULL << tsq);
			tsq += 8;
		}

		tsq = sq - 8;
		while (tsq >= 0) 
		{
			BlackPassedMask[sq] |= (1ULL << tsq);
			tsq -= 8;
		}

		if (FilesBrd[SQ120(sq)] > FILE_A) 
		{
			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1];

			tsq = sq + 7;
			while (tsq < 64) 
			{
				WhitePassedMask[sq] |= (1ULL << tsq);
				tsq += 8;
			}

			tsq = sq - 9;
			while (tsq >= 0) 
			{
				BlackPassedMask[sq] |= (1ULL << tsq);
				tsq -= 8;
			}
		}

		if (FilesBrd[SQ120(sq)] < FILE_H) 
		{
			IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];

			tsq = sq + 9;
			while (tsq < 64) 
			{
				WhitePassedMask[sq] |= (1ULL << tsq);
				tsq += 8;
			}

			tsq = sq - 7;
			while (tsq >= 0) 
			{
				BlackPassedMask[sq] |= (1ULL << tsq);
				tsq -= 8;
			}
		}
	}
}
void InitFilesRanksBrd () 
{
	int file  = FILE_A;
	int rank  = RANK_1;
	int sq    = A1;

	for (int &filesBrd : FilesBrd) filesBrd = OFFBOARD;
	for (int &rankBrd  : RanksBrd) rankBrd  = OFFBOARD;
	
	for (rank = RANK_1; rank <= RANK_8; ++rank) 
	{
		for (file = FILE_A; file <= FILE_H; ++file) 
		{
			sq = FR2SQ(file, rank);
			FilesBrd[sq] = file;
			RanksBrd[sq] = rank;
		}
	}
}
void InitHashKeys      () 
{
	for (int i = 0; i < MAXPIECETYPE; i++) 
	{
		for (U64 &pieceKeys : PieceKeys[i])
			pieceKeys = RAND_64;
	}

	SideKey = RAND_64;
	for (U64 &castleKeys : CastleKeys)  
		castleKeys = RAND_64;
}
void InitBitMasks      () 
{

	for (U64 &setMask   : SetMask  ) setMask   = 0ULL;
	for (U64 &clearMask : ClearMask) clearMask = 0ULL;

	for (int i = 0; i < 64; i++) 
	{
		SetMask  [i] |= (1ULL << i);
		ClearMask[i]  = ~SetMask[i];
	}
}
void InitSq120To64     () 
{
	int sq64   = 0;

	for (int &sq : Sq120ToSq64) sq =  65;
	for (int &sq : Sq64ToSq120)	sq = 120;

	for (int rank = RANK_1; rank <= RANK_8; rank++) 
	{
		for (int file = FILE_A; file <= FILE_H; file++) 
		{
			int sq = FR2SQ(file, rank);
			ASSERT(SqOnBoard(sq));
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq  ] = sq64;
			sq64++;
		}
	}
}
bool FilePresent(string filepath)
{
	ifstream f(filepath);
	return f.good();
}
void CheckResourceFilesAvailability()
{
	
	bool err = false;
	char * cwd = _getcwd(0,0);
	cout << endl << "Working Directory = " << cwd << endl << endl;
	free(cwd);

	if ( ! FilePresent(BOOKFILENAME       ) ) { cout << "Error: " << BOOKFILENAME        << "Resource File NOT found" << endl; err = true; }
	if ( ! FilePresent(EVALTESTSFILENAME  ) ) { cout << "Error: " << EVALTESTSFILENAME   << "Resource File NOT found" << endl; err = true; }
	if ( ! FilePresent(MOVETESTSFILENAME  ) ) { cout << "Error: " << MOVETESTSFILENAME   << "Resource File NOT found" << endl; err = true; }
	if ( ! FilePresent(MATETESTSFILENAME  ) ) { cout << "Error: " << MATETESTSFILENAME   << "Resource File NOT found" << endl; err = true; }
	if ( ! FilePresent(BMTESTSFILENAME    ) ) { cout << "Error: " << BMTESTSFILENAME     << "Resource File NOT found" << endl; err = true; }
	if ( ! FilePresent(MIRRORTESTSFILENAME) ) { cout << "Error: " << MIRRORTESTSFILENAME << "Resource File NOT found" << endl; err = true; }
}
void InitMagicSquares()
{
	C_BOARD blankBoard;
	blankBoard.ResetBoard();

	for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++)
	{
		BlockerMasks_Rook  [sq64] = C_MOVEGEN::mgBlockermask_rook  (&blankBoard,sq64).value;
		BlockerMasks_Bishop[sq64] = C_MOVEGEN::mgBlockermask_bishop(&blankBoard,sq64).value;
	}

}
void InitMagicIndexs()
{
	mt19937 gen(314158265);  // seed it (note is 32 bit random number generator)
    #define RAND_64 ( ((U64)gen()) | (((U64)gen())<<32) )
	bool done       = false;  // true iff all indexes have been generated with no muggles
	U64  blkmask    = 0;
	U64  magicIndex = 0;	for (int sq64 = 0; sq64 < 64; sq64++)
	{
		MagicIndexs_Rook[sq64] = 0;
		MagicIndexs_Bishop[sq64] = 0;
	}
	while (!done)
	{
		done = true;
		for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++)
		{
			if (MagicIndexs_Rook[sq64] <= 0)
			{
				blkmask = BlockerMasks_Rook[sq64];
				magicIndex = (blkmask*RAND_64) >> (64 - 10); // generate magic index;
				for (int iPos = 0; iPos < 64; iPos++)
				{   // check for muggle
					if (MagicIndexs_Rook[iPos] != magicIndex) continue;
					// found muggle
					MagicIndexs_Rook[iPos] = 0;
					magicIndex = 0;
					done = false;
					break;
				}
				MagicIndexs_Rook[sq64] = magicIndex;
			}
		}

		for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++)
		{
			if (MagicIndexs_Bishop[sq64] <= 0)
			{
				blkmask = BlockerMasks_Rook[sq64];
				magicIndex = (blkmask*RAND_64) >> (64 - 10); // generate magic index;
				for (int iPos = 0; iPos < 64; iPos++)
				{   // check for muggle
					if (MagicIndexs_Bishop[iPos] != magicIndex) continue;
					// found muggle
					MagicIndexs_Bishop[iPos] = 0;
					magicIndex = 0;
					done = false;
					break;
				}
				MagicIndexs_Bishop[sq64] = magicIndex;
			}
		}
	}

}

void InitMoveGen()
{
#pragma region Init MvvLVAScores
	int Attacker;
	int Victim;
	for (Attacker = wP; Attacker <= bK; ++Attacker)
	{
		for (Victim = wP; Victim <= bK; ++Victim)
		{
			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - (VictimScore[Attacker] / 100);
		}
	}
#pragma endregion

#pragma region Init Knight Spans
	for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++)
	{
		U64 knightSpan = (sq64 > 18) ? BB_KNIGHT_SPAN << (sq64 - 18) : BB_KNIGHT_SPAN >> (18 - sq64);

		if (sq64 % 8 < 4)
			knightSpan &= ~BB_FILE_GH;
		else
			knightSpan &= ~BB_FILE_AB;

		KnightSpans[sq64] = knightSpan;
	}
#pragma endregion

#pragma region Init King Spans
	for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++)
	{
		U64 kingSpan = (sq64 > 9) ? BB_KING_SPAN << (sq64 - 9) : BB_KING_SPAN >> (9 - sq64);

		if (sq64 % 8 < 4)
			kingSpan &= ~BB_FILE_GH;
		else
			kingSpan &= ~BB_FILE_AB;

		KingSpans[sq64] = kingSpan;
	}
#pragma endregion

}
void InitializeThreadPool()
{
    int sz = NumberOfProcessors()*2 - 1; 

	for(int n=0; n<sz; n++)                                                                
	   threadPool.create_thread( boost::bind(&boost::asio::io_service::run,&ioService) );  
  
	cout << "ThreadPool Initialized [size=" << sz << "]" << endl;                          
  
}

void InitializeSearchKind(int kind)
{

	searchKind = SEARCHKIND_DEFAULT;

	switch (searchKind)
	{
	default:
	case SEARCHTYPE_THREADS_NONE:     cout << "SEARCH KIND=SEARCHTYPE_THREADS_NONE   (For each move { makemove; AlphaBeta; unmakemove })" << endl;                            break;
	case SEARCHTYPE_THREADS_SIMPLE:   cout << "SEARCH KIND=SEARCHTYPE_THREADS_SIMPLE ('n' threads)" << endl;                                                                  break;
	case SEARCHTYPE_THREADS_YBWC:     cout << "SEARCH KIND=SEARCHTYPE_THREADS_YBWC   (TODO WIP-not working yet: 'n' Threads....Younger Brother Wait Concept (YBWC))" << endl; break;
	}
	cout << endl;
	cout << " Number of Processors = " << NumberOfProcessors()     << endl;
	cout << " Max Thread Pool      = " << NumberOfProcessors()*2-1 << endl;
	cout << " Max Search Threads   = " << maxSearchThreads         << endl;

}

void AllInit           () 
{
	InitSq120To64     ();
	InitBitMasks      ();
	InitHashKeys      ();
	InitFilesRanksBrd ();
	InitEvalMasks     ();
	InitMagicSquares  ();
	InitMagicIndexs   ();
	InitMoveGen       ();

	CheckResourceFilesAvailability();

	maxSearchThreads = MAXTHREADS_DEFAULT;

	InitializeThreadPool(); // note: Thread Pool size = Number of Processors; usable Threads = maxThreads; range (0 <= maxThreads <= #Processors)

	InitializeSearchKind(SEARCHKIND_DEFAULT);
	

	BOOK.InitPolyBook(BOOKFILENAME);
}
