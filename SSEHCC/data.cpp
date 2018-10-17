// data.c
#include "stdafx.h"

bool   haltOnError      = false;
bool   stopAllThreads   = false;
int    searchKind       = 0;
int    maxSearchThreads = 0;
U64    highestmovecount = 0;
S64    THECLOCK         = 0;
bool   THECLOCK_INUSE   = false;
bool   cmdLineAvailable = false;
string cmdLine;

const char   cPceChar []  = { '.','P','N','B','R','Q','K','p','n','b','r','q','k' };
const char   cSideChar[]  = { 'w','b','-' };
const char   cRankChar[]  = { '1','2','3','4','5','6','7','8' };
const char   cFileChar[]  = { 'a','b','c','d','e','f','g','h' };

const string sPceChar [] = { ".","P","N","B","R","Q","K","p","n","b","r","q","k" };
const string sSideChar[] = { "w","b","-" };
const string sRankChar[] = { "1","2","3","4","5","6","7","8" };
const string sFileChar[] = { "a","b","c","d","e","f","g","h" };
    
//                                                     wP     wN     wB     wR     wQ     wK     bP     bN     bB     bR     bQ     bK  
const bool PieceBig        [MAXPIECETYPE] = { false, false,  true,  true,  true,  true,  true, false,  true,  true,  true,  true,  true };
const bool PieceMaj        [MAXPIECETYPE] = { false, false, false, false,  true,  true,  true, false, false, false,  true,  true,  true };
const bool PieceMin        [MAXPIECETYPE] = { false, false,  true,  true, false, false, false, false,  true,  true, false, false, false };
const bool PiecePawn       [MAXPIECETYPE] = { false,  true, false, false, false, false, false,  true, false, false, false, false, false };
const bool PieceKnight     [MAXPIECETYPE] = { false, false,  true, false, false, false, false, false,  true, false, false, false, false };
const bool PieceKing       [MAXPIECETYPE] = { false, false, false, false, false, false,  true, false, false, false, false, false,  true };
const bool PieceRookQueen  [MAXPIECETYPE] = { false, false, false, false,  true,  true, false, false, false, false,  true,  true, false };
const bool PieceBishopQueen[MAXPIECETYPE] = { false, false, false,  true, false,  true, false, false, false,  true, false,  true, false };
const bool PieceSlides     [MAXPIECETYPE] = { false, false, false,  true,  true,  true, false, false, false,  true,  true,  true, false };

//                                                     wP     wN     wB     wR     wQ     wK     bP     bN     bB     bR     bQ     bK  
const int  PieceVal        [MAXPIECETYPE] = {     0,   100,   320,   330,   500,   900, 50000,   100,   320,   330,   500,   900, 50000 };
const int  PieceCol        [MAXPIECETYPE] = {  BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK };

const int Mirror64[64] = 
{
	56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
	48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
	40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
	32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
	24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
	16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
	 8	,	 9	,	10	,	11	,	12	,	13	,	14	,	15	,
	 0	,	 1	,	 2	,	 3	,	 4	,	 5	,	 6	,	 7
};
