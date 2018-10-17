// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "stdlib.h"
#include <tchar.h>
#include "string"
//#include "ATLComTime.h"
#include <iomanip>
#include "iostream"
#include "fstream"
#include "sstream"
#include "vector"
#include <algorithm>
#include <exception>
#include <random>
#include "string"
#include <mutex>

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/move/move.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;

//#define DEBUG
//#define DEBUGMOVE

//#define USE_FUTILITY_LIMIT
#define FUTILITY_LIMIT 3

#pragma region Timer Defines and Variables

    #define    CLOCK_CLEAR  { THECLOCK  = 0;  THECLOCK_INUSE = FALSE;   }  
    #define    CLOCK_START  { ASSERT(!THECLOCK_INUSE); THECLOCK  = -(S64)GetTimeMs();  THECLOCK_INUSE = TRUE;   }  // start using clock; start timing interval 
    #define    CLOCK_ON     { ASSERT( THECLOCK_INUSE); THECLOCK -= GetTimeMs();                                 }  // start timing interval
    #define    CLOCK_OFF    { ASSERT( THECLOCK_INUSE); THECLOCK += GetTimeMs();                                 }  // add   timing interval
    #define    CLOCK_STOP   { ASSERT( THECLOCK_INUSE); THECLOCK += GetTimeMs();        THECLOCK_INUSE = FALSE;  }  // add   timing interval; stop using clock;

#pragma endregion

	enum E_SEARCHTYPE { SEARCHTYPE_THREADS_NONE                    // NO THREADS: for each move { makemove; AlphaBeta; unmakemove }
			          , SEARCHTYPE_THREADS_SIMPLE                  // 'n' threads 
		                                                           //    - TODO WIP
		                                                           //    - it works but does not always best results 
		              , SEARCHTYPE_THREADS_YBWC                    // Younger Brother Wait Concept (YBWC)
		                                                           //    - TODO WIP
		                                                           //    - does NOT WORK yet
		                                                           //    - returning wrong move level?
				      };

#pragma region BOOST THEAD POOL IMPLEMENTATION

	constexpr auto SEARCHKIND_DEFAULT          = SEARCHTYPE_THREADS_YBWC;
	constexpr auto MAXTHREADS_DEFAULT          = 5;
	constexpr auto THREADSEARCHDEPTHTHRESHHOLD = 9;   // threads only are used if depth >= THREADSEARCHDEPTHTHRESHHOLD (ply moves);

	extern bool								 haltOnError;
	extern bool                              stopAllThreads;
	extern int                               searchKind;
	extern int                               maxSearchThreads;

    extern boost::asio::io_service           ioService; 
    extern boost::thread_group               threadPool;
	extern boost::mutex                      thread_mutex;

#define THREAD_START(task)                  ioService.post(task)
#define THREAD_START2(task,taskcount)       { THREAD_LOCK_PROTECT(++taskcount;) THREAD_START(task); }
#define THREAD_WAITFOR(cond)                LINGER_WHILE(!(cond))

#define MICROSEC                                1000 // in nanosecs
#define MILLISEC                             1000000 // in nanosecs
#define LINGER_MAX                          10000000 // 10 milli-seconds in nanosecs 
#define LINGER_WHILE(cond)                  {  int waitns=1*MICROSEC;                       \
									  	       while(cond)                                  \
									  	       {  waitns = min(LINGER_MAX,waitns++);        \
									  	          THREAD_SLEEP_NANOSECS(waitns);            \
									  	       }                                            \
									        }
#define THREAD_SLEEP_NANOSECS(nanosecs)     boost::this_thread::sleep_for(boost::chrono::nanoseconds (nanosecs )) 
#define THREAD_SLEEP_MICROSECS(microsecs)   boost::this_thread::sleep_for(boost::chrono::microseconds(microsecs)) 
#define THREAD_SLEEP_MILLISECS(millisecs)   boost::this_thread::sleep_for(boost::chrono::milliseconds(millisecs)) 
#define THREAD_SLEEP_SECS(secs)             boost::this_thread::sleep_for(boost::chrono::seconds     (secs     )) 
#define THREAD_LOCK                         thread_mutex.lock()
#define THREAD_UNLOCK                       thread_mutex.unlock()
#define THREAD_LOCK_PROTECT(stmts)          {  THREAD_LOCK; stmts; THREAD_UNLOCK; } 
									        
#define NumberOfProcessors                  []() 											\
	                                        {	SYSTEM_INFO sysinfo; 						\
									        	GetSystemInfo(&sysinfo); 					\
									        	return (int)sysinfo.dwNumberOfProcessors;  	\
	                                        }

#pragma endregion

class C_BOARD;

#pragma warning( disable: 4996)

// Singalton Classes (Shorten Names)
#define EVAL ( CS_EVALUATE  ::getInstance() )
#define BOOK ( C_BOOKTABLE ::getInstance() )
#define HASH ( CS_HASHTABLE ::getInstance() )

#define MYPROJECTBASE       "C:/Users/Jim/Documents/My Projects"
#define BOOKFILENAME        (MYPROJECTBASE "/Chess BookBases/Maverick.bin")

#define EPDTESTS(name)      (MYPROJECTBASE "/Chess Resources/EPD Tests/" name)
#define EPDTESTS2(name)     (MYPROJECTBASE "/Chess Resources/EPD Tests/epd_testsuites/" name)

#define EVALTESTSFILENAME   EPDTESTS("evaltests.epd")  // mirror tests
#define MOVETESTSFILENAME   EPDTESTS2("perftsuite.epd")
#define MATETESTSFILENAME   EPDTESTS("matein.epd")
#define BMTESTSFILENAME     EPDTESTS("ToMoveAndWin.epd")
#define MIRRORTESTSFILENAME EPDTESTS("evaltests.epd")

#define pf std::printf
#ifndef DEBUG
#define ASSERT(n)
#define ASSERT2(n,msg)
#else
#define ASSERT2(n,msg)                                          \
		if(!(n))                                                \
		{                                                       \
		   pf("\n\n%s - Failed [%s]\n",#n,msg);                 \
		   pf("   On %s \n"       ,__DATE__);                   \
		   pf("   At %s \n"       ,__TIME__);                   \
		   pf("   In File %s \n"  ,__FILE__);                   \
		   pf("   At Line %d \n"  ,__LINE__);                   \
		   pf("\n\n--> Hits 'any char key' to end program..."); \
		   if (haltOnError) getchar();                          \
		   abort();                                             \
		} 
#define ASSERT(n) ASSERT2((n),"")
#endif

#define WRONG_PLY_MOVES(plyMoves,expectedMoves) 	( abs(plyMoves-(expectedMoves)*2) > 1 )

#define FORCE_INLINE  __forceinline

typedef boost::uint_t<256> U256;
typedef boost::uint_t<128> U128;
typedef unsigned __int64   U64;
typedef unsigned __int32   U32;
typedef          __int64   S64;
typedef          __int32   S32;

#define PRINT_ID_NAME          {                                                   \
								   std::string s = "";                             \
								   s += "id name ssehcc.v01.";                     \
								   time_t        rawtime;						   \
								   struct tm *   timeinfo;						   \
								   char buffer[80];        						   \
								   time(&rawtime);								   \
								   timeinfo = localtime(&rawtime);				   \
								   strftime(buffer, 80, "%Y.%m%d.%H%M", timeinfo); \
								   s += std::string(buffer);             		   \
								   cout << s.c_str() << endl;    				   \
							   }


#define BRD_SQ_NUM120          120
#define BRD_SQ_NUM64           64
#define BRD_SQ_LOWEST          21           // lowest  on board sq #
#define BRD_SQ_HIGHEST         98           // highest on board sq # 
#define MAXGAMEMOVES           2048		    // max ply moves (1/2 moves)
#define MAX_PLY_DEPTH          64
#define MAXGENMOVES            218          // absolute # of moves which can be generated at end specific board position
#define START_FEN              "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
                               
#define CHESSINFINITE          300000 
#define PVMOVEFOUND            2000000  
#define ISMATE                 ( CHESSINFINITE - MAX_PLY_DEPTH )

enum E_PIECETYPE  { EMPTY , wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK, MAXPIECETYPE };
enum E_FILE       { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };
enum E_RANK       { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };
enum E_COLOR      { WHITE, BLACK, BOTH };
enum E_GAMEMODE   { UCIMODE, CONSOLEMODE };
enum E_HF         { HFNONE, HFALPHA, HFBETA, HFEXACT };
enum E_CASTLEKIND   // White King Castling, White Queen Castling, ...
{
	  WK_SIDECASTLING = 1
	, WQ_SIDECASTLING = 2
	, BK_SIDECASTLING = 4
	, BQ_SIDECASTLING = 8 
};

typedef __int8 FLAGS;
typedef __int8 DEPTH;

#pragma region SQ numbers(G.A1, A2, ...)  120 sq versions
enum E_SQNAME120 
      {
	    A1 = 21, B1 = 22, C1 = 23, D1 = 24, E1 = 25, F1 = 26, G1 = 27, H1 = 28 
	  , A2 = 31, B2 = 32, C2 = 33, D2 = 34, E2 = 35, F2 = 36, G2 = 37, H2 = 38 
	  , A3 = 41, B3 = 42, C3 = 43, D3 = 44, E3 = 45, F3 = 46, G3 = 47, H3 = 48 
	  , A4 = 51, B4 = 52, C4 = 53, D4 = 54, E4 = 55, F4 = 56, G4 = 57, H4 = 58 
	  , A5 = 61, B5 = 62, C5 = 63, D5 = 64, E5 = 65, F5 = 66, G5 = 67, H5 = 68 
	  , A6 = 71, B6 = 72, C6 = 73, D6 = 74, E6 = 75, F6 = 76, G6 = 77, H6 = 78 
	  , A7 = 81, B7 = 82, C7 = 83, D7 = 84, E7 = 85, F7 = 86, G7 = 87, H7 = 88 
	  , A8 = 91, B8 = 92, C8 = 93, D8 = 94, E8 = 95, F8 = 96, G8 = 97, H8 = 98 
	  , OFFBOARD =  99
	  , NO_EP_SQ = 100
	  , NO_SQ    = 101
     };
#pragma endregion

#pragma region SQ numbers(G.AA1, G, AA2, ...) 64 sq version
enum E_SQNAME64
      {
	    SQ64_A1 =  0, SQ64_B1 =  1, SQ64_C1 =  2, SQ64_D1 =  3, SQ64_E1 =  4, SQ64_F1 =  5, SQ64_G1 =  6, SQ64_H1 =  7
      , SQ64_A2 =  8, SQ64_B2 =  9, SQ64_C2 = 10, SQ64_D2 = 11, SQ64_E2 = 12, SQ64_F2 = 13, SQ64_G2 = 14, SQ64_H2 = 15
      , SQ64_A3 = 16, SQ64_B3 = 17, SQ64_C3 = 18, SQ64_D3 = 19, SQ64_E3 = 20, SQ64_F3 = 21, SQ64_G3 = 22, SQ64_H3 = 23
      , SQ64_A4 = 24, SQ64_B4 = 25, SQ64_C4 = 26, SQ64_D4 = 27, SQ64_E4 = 28, SQ64_F4 = 29, SQ64_G4 = 30, SQ64_H4 = 31
      , SQ64_A5 = 32, SQ64_B5 = 33, SQ64_C5 = 34, SQ64_D5 = 35, SQ64_E5 = 36, SQ64_F5 = 37, SQ64_G5 = 38, SQ64_H5 = 39
      , SQ64_A6 = 40, SQ64_B6 = 41, SQ64_C6 = 42, SQ64_D6 = 43, SQ64_E6 = 44, SQ64_F6 = 45, SQ64_G6 = 46, SQ64_H6 = 47
      , SQ64_A7 = 48, SQ64_B7 = 49, SQ64_C7 = 50, SQ64_D7 = 51, SQ64_E7 = 52, SQ64_F7 = 53, SQ64_G7 = 54, SQ64_H7 = 55
      , SQ64_A8 = 56, SQ64_B8 = 57, SQ64_C8 = 58, SQ64_D8 = 59, SQ64_E8 = 60, SQ64_F8 = 61, SQ64_G8 = 62, SQ64_H8 = 63
	};
#pragma endregion

class S_MOVE                             
{
public:
	int move;
	int score;
} ;

class S_MOVELIST  
{
public:
	int    size; // current size
	S_MOVE moves[MAXGENMOVES];

	void   Clear(           )  { size          = 0;    }
	void   Add  (S_MOVE move)  { moves[size++] = move; }
};
class S_HASHENTRY    
{
public:
	U64   posKey;   // hash key
	int   move;
	int   score;
	DEPTH depthRemaining;
	FLAGS flags;
};
class S_UNDO                             
{
public:
	int move;
	int castlePerm;
	int SQenPas120;
	int fiftyMove;
	U64 posKey;
};
class S_OPTIONS        
{
public:
	bool UseBook;
};
class SEARCHINFO 
{
   public:
	   U64   starttime;
	   S64   stoptime;
	   DEPTH depth;
	   bool  timeset;
	   int   movestogo;
       C_BOARD *pThreadBoard;
	   U64   nodes;
		  
	   bool  quit;
	   int   stopped;

	   float fh;
	   float fhf;
	   int   nullCut;

	   int   GAME_MODE;
	   bool  POST_THINKING;

	   int   bestScore;
	   int   bestMove;
	   int   matein_plymoves;
};


// GAME MOVE Layout 
//   0000 0000 0000 0000 0000 0111 1111 -> From                    0x7F
//   0000 0000 0000 0011 1111 1000 0000 -> To       >     > 7,     0x7F
//   0000 0000 0011 1100 0000 0000 0000 -> Captured       >> 14,   0xF
//   0000 0000 0100 0000 0000 0000 0000 -> EP                      0x40000
//   0000 0000 1000 0000 0000 0000 0000 -> Pawn Start              0x80000
//   0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20,   0xF
//   0001 0000 0000 0000 0000 0000 0000 -> Castle                  0x1000000

#define MV_FROMSQ(move)        (  (move)      & 0x7F )
#define MV_TOSQ(move)          ( ((move)>> 7) & 0x7F )
#define MV_CAPTURED(move)      ( ((move)>>14) & 0xF  )
#define MV_PROMOTED(move)      ( ((move)>>20) & 0xF  )

#define MV_ENPASSANT(move)     (  (move) & MVFLAG_EnPassant     )
#define MV_PAWNSTART(move)     (  (move) & MVFLAG_PawnStart     )
#define MV_CASTLE(move)        (  (move) & MVFLAG_Castle        )
#define MV_PIECECAPTURE(move)  (  (move) & MVFLAG_PieceCapture  )
#define MV_PROMOTEDPIECE(move) (  (move) & MVFLAG_PromotedPiece )

#define MVFLAG_EnPassant     0x40000             // move flag: En Passant
#define MVFLAG_PawnStart     0x80000             // move flag: Pawn Start
#define MVFLAG_Castle        0x1000000           // move flag: Castle
#define MVFLAG_PieceCapture  0x7C000             // move flag: Piece Capture
#define MVFLAG_PromotedPiece 0xF00000            // move flag: Promoted Piece

#define NOMOVE    0


/* MACROS */

#define SQONBOARD(sq)  ( FilesBrd[sq] != OFFBOARD )
#define FR2SQ(f,r)     ( (21 + (f) ) + ( (r) * 10 ) )
#define DIST(sq1,sq2)  ( abs(RanksBrd[sq1]-RanksBrd[sq2]) + abs(FilesBrd[sq1]-FilesBrd[sq2]) )
#define SQ64(sq120)    (Sq120ToSq64[(sq120)])
#define SQ120(sq64)    (Sq64ToSq120[(sq64 )])
#define CNT(b)         BitCount(b)
#define CLRBIT(bb,sq)  ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq)  ((bb) |= SetMask[(sq)])

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])

/* GLOBALS */

extern S64    THECLOCK;
extern bool   THECLOCK_INUSE;
extern bool   cmdLineAvailable;      // input line has been read
extern string cmdLine;

extern const  int PceDir_Knight[8];
extern const  int PceDir_Bishop[4];
extern const  int PceDir_Rook[4];
extern const  int PceDir_Queen[8];
extern const  int PceDir_King[8];
extern const  int KnDir[8];
extern const  int RkDir[4];
extern const  int BiDir[4];
extern const  int KiDir[8];
extern const  int NumDir[13];
extern const  int VictimScore[13];
extern const  U64 DiagonalMasks8[15];
extern const  U64 AntiDiagonalMasks8[15];
extern const  U64 RankMasks8[8];
extern const  U64 FileMasks8[8];

extern const  U64 BB_FILE_A;
extern const  U64 BB_FILE_B;
extern const  U64 BB_FILE_C;
extern const  U64 BB_FILE_D;
extern const  U64 BB_FILE_E;
extern const  U64 BB_FILE_F;
extern const  U64 BB_FILE_G;
extern const  U64 BB_FILE_H;
extern const  U64 BB_FILE_AB;
extern const  U64 BB_FILE_GH;
extern const  U64 BB_RANK_1;
extern const  U64 BB_RANK_2;
extern const  U64 BB_RANK_3;
extern const  U64 BB_RANK_4;
extern const  U64 BB_RANK_5;
extern const  U64 BB_RANK_6;
extern const  U64 BB_RANK_7;
extern const  U64 BB_RANK_8;
extern const  U64 BB_CENTRE;
extern const  U64 BB_EXTENDED_CENTRE;
extern const  U64 BB_KING_SIDE;
extern const  U64 BB_QUEEN_SIDE;
extern const  U64 BB_KING_SPAN;
extern const  U64 BB_KNIGHT_SPAN;

extern int  Sq120ToSq64[BRD_SQ_NUM120];
extern int  Sq64ToSq120[BRD_SQ_NUM64];
extern U64  SetMask    [BRD_SQ_NUM64];
extern U64  ClearMask  [BRD_SQ_NUM64];
extern U64  PieceKeys  [MAXPIECETYPE][BRD_SQ_NUM120];
extern U64  SideKey;
extern U64  CastleKeys [16];

extern U64  highestmovecount;

extern const char   cPceChar    [];
extern const char   cSideChar   [];
extern const char   cRankChar   [];
extern const char   cFileChar   [];

extern const string sPceChar    [];
extern const string sSideChar   [];
extern const string sRankChar   [];
extern const string sFileChar   [];

extern const bool PieceBig   [MAXPIECETYPE];
extern const bool PieceMaj   [MAXPIECETYPE];
extern const bool PieceMin   [MAXPIECETYPE];
extern const int  PieceVal   [MAXPIECETYPE];
extern const int  PieceCol   [MAXPIECETYPE];
extern const bool PiecePawn  [MAXPIECETYPE];

extern int  FilesBrd[BRD_SQ_NUM120];
extern int  RanksBrd[BRD_SQ_NUM120];

extern const bool PieceKnight      [13];  // is a knight big
extern const bool PieceKing        [13];
extern const bool PieceRookQueen   [13];
extern const bool PieceBishopQueen [13];
extern const bool PieceSlides      [13];

extern const int  Mirror64    [BRD_SQ_NUM64];
		    
extern U64  FileBBMask        [8];
extern U64  RankBBMask        [8];

extern U64  KnightSpans       [BRD_SQ_NUM64];
extern U64  KingSpans         [BRD_SQ_NUM64];
extern int  MvvLvaScores      [13][13];
						      
extern U64  BlackPassedMask   [BRD_SQ_NUM64];
extern U64  WhitePassedMask   [BRD_SQ_NUM64];
extern U64  IsolatedMask      [BRD_SQ_NUM64];

extern U64 BlockerMasks_Rook  [BRD_SQ_NUM64];
extern U64 BlockerMasks_Bishop[BRD_SQ_NUM64];
extern U64 MagicIndexs_Rook   [BRD_SQ_NUM64];
extern U64 MagicIndexs_Bishop [BRD_SQ_NUM64];

extern S_OPTIONS EngineOptions[1];

extern string token;
extern string skan(string& line);


template <typename T> string FormatWithCommas(T value)
{
	std::stringstream ss;
	ss.imbue(std::locale(""));
	ss << std::fixed << value;
	return ss.str();
}

#define SKANINT(i) { try                { i = stoi(skan(cmdLine));  } 						       \
					 catch(exception e) { cout << "Error: scanning 'integer' parameter" << endl;   \
                                          cout << "       line=" << cmdLine << endl;               \
                                          return;                                                  \
                                        }                                                          \
                   }


/* FUNCTIONS */

// search.cpp
extern void        Search_Position         (C_BOARD *pBoard, SEARCHINFO *pInfo);
extern SEARCHINFO *Search_Thread_YBWC      (C_BOARD *pBoard, SEARCHINFO *pInfo, DEPTH depthRemaining, S_MOVELIST *firstLvlMovelist);
extern SEARCHINFO *Search_Thread_Simple    (C_BOARD *pBoard, SEARCHINFO *pInfo, DEPTH depthRemaining, S_MOVELIST *firstLvlMovelist);
extern SEARCHINFO *Search_Thread_None      (C_BOARD *pBoard, SEARCHINFO *pInfo, DEPTH iterativeDepth, S_MOVELIST *firstLvlMovelist);

// init.cpp
extern void AllInit();
extern void InitializeSearchKind(int kind);
extern void InitializeThreadPool();

// io.cpp
extern char *PrMove              (int move);
extern char *PrSq                (int sq);
extern int   ParseMove           (std::string sMove, C_BOARD *pBoard);


//validate.cpp
extern int   SqOnBoard           (int sq);
extern int   SideValid           (int side);
extern int   FileRankValid       (int fr);
extern int   PieceValidEmpty     (int pce);
extern int   PieceValid          (int pce);
extern int   SqIs120             (int sq);
extern int   PceValidEmptyOffbrd (int pce);

// perft.cpp				     
extern long  PerftTest           (DEPTH depth, C_BOARD *pBoard);
extern bool  PerftTestAll        ();
extern void  PerftMirrorTest     ();
extern bool  PerftTestEval       ();
extern bool  PerftTestMove       (int maxMoveCount);
extern bool  PerftTestMate       (DEPTH maxDepth  ,bool clearHash);
extern bool  PerftTestBestMove   (std::string parm,bool clearHash);
extern bool  PerftTestUtil       ();
extern bool  PerftTestMagicSQ    ();
extern bool  PerftTestSearch     ();

// misc.cpp
extern S64   GetTimeMs           ();
extern void  PUNT                (std::string msg);
extern std::string Trim          (std::string s);
extern void  PrintBB             (U64 bb);
extern void  CMDLineReaderThread ();
extern void  GetCMDLine          ();

const int BitTable[64] =
{
	63, 30,  3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29,  2, 51, 21, 43,
	45, 10, 18, 47,  1, 54,  9, 57,  0, 35, 62, 31, 40,  4, 49,  5, 52,	26, 60,  6, 23, 44,
	46, 27, 56, 16,  7, 39, 48, 24, 59, 14, 12, 55, 38, 28, 58, 20, 37, 17, 36,  8
};

const int Mod37BitPosition[37] = // map a bit value mod 37 to its position
{
	32,  0,  1, 26,  2, 23, 27,  0,  3, 16
	, 24, 30, 28, 11,  0, 13,  4,  7, 17,  0
	, 25, 22, 31, 15, 29, 10, 12,  6,  0, 21
	, 14,  9,  5, 20,  8, 19, 18
};

const U64 BitReverseTable[256]
{
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,  //   0 -   7
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,  //   8 -  15
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,  //  16 -  23
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,  //  24 -  31
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,  //  32 -  39
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,  //  40 -  47
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,  //  48 -  55
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,  //  56 -  63
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,  //  64 -  71
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,  //  72 -  79
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,  //  80 -  87
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,  //  88 -  95
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,  //  96 - 103
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,  // 104 - 111
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,  // 112 - 119
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,  // 120 - 127
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,  // 128 - 135
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,  // 136 - 143
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,  // 144 - 151
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,  // 152 - 159
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,  // 160 - 167
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,  // 168 - 175
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,  // 176 - 183
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,  // 184 - 191
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,  // 192 - 199
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,  // 200 - 207
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,  // 208 - 215
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,  // 216 - 223
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,  // 224 - 231
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,  // 232 - 239
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,  // 240 - 247
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff   // 248 - 255
};

