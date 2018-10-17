#include "stdafx.h"
#include "C_BOOKTABLE.h"
#include "C_BOARD.h"
#include "C_MOVEGEN.h"
#include "CS_HASHTABLE.h"
#include "CS_EVALUATE.h"

static int   doNullDepth = 4;
static bool  inputFound  = false;

static void        CheckUp         (SEARCHINFO *info) 
{
	// .. check if time up, or interrupt from GUI
	if( info->timeset && (GetTimeMs() > info->stoptime) || cmdLineAvailable)
	{
		info->stopped = true;
		stopAllThreads = true;
		cout << "timeset limit reached" << endl;
	}

	//ReadInput(info);
}
static void        PickNextMove    (int moveNum, S_MOVELIST *movelist)
{

	S_MOVE tempMoveEntry;
	int    bestScore   = -CHESSINFINITE;
	int    bestNum     = moveNum;
	int    bestCapture = 0;
	for (int i = moveNum; i < movelist->size; i++)
	{
		if ( movelist->moves[i].score > bestScore )
		{
			bestScore = movelist->moves[i].score;
			bestNum   = i;
			bestCapture = (MV_CAPTURED(movelist->moves[i].move) != EMPTY) ? 0 : PieceVal[MV_CAPTURED(movelist->moves[i].move)];
		}
		else
		if ( (movelist->moves[i].score == bestScore) && (MV_CAPTURED(movelist->moves[i].move) != EMPTY) )
		{
			if ( PieceVal[MV_CAPTURED(movelist->moves[i].move)] > bestCapture )
			{
				bestNum     = i;
				bestCapture = PieceVal[MV_CAPTURED(movelist->moves[i].move)];
			}
		}
	}

	ASSERT(moveNum>=0 && moveNum<movelist->size);
	ASSERT(bestNum>=0 && bestNum<movelist->size);
	ASSERT(bestNum>=moveNum);

	tempMoveEntry            = movelist->moves[moveNum];
	movelist->moves[moveNum] = movelist->moves[bestNum];
	movelist->moves[bestNum] = tempMoveEntry;
}
static int         MateInHalfMoves (int bestscore)
{
    if ( abs(bestscore) < ISMATE )
		return 0;

    ASSERT( (CHESSINFINITE - abs(bestscore)) >= 0);

	int mateinPlymoves = CHESSINFINITE - abs(bestscore);

    return ( mateinPlymoves ) ; // return half move count
}

#define            PrintMoveListScores(msg,depth,movelist)                           \
                   {                                                                 \
                   		if (true)                                                    \
                   		{                                                            \
                   			std::cout << msg << " Depth:" << depth << endl;          \
                   			for (int i = 0; i < movelist->size; i++)                 \
                   				if (movelist->moves[i].score != 0)                   \
                   					std::cout << "    Move:" << i << " "             \
						                 << PrMove(movelist->moves[i].move)          \
                   					     << " Score: " << movelist->moves[i].score   \
						                 << endl;                                    \
                   		}                                                            \
                   }

static bool        IsRepetition    (C_BOARD *pBoard) 
{
	for(int index = pBoard->hisPly - pBoard->fiftyMove; index < pBoard->hisPly-1; ++index) 
	{
		ASSERT(index >= 0 && index < MAXGAMEMOVES);
		if(pBoard->posKey == pBoard->history[index].posKey) 
			return true;
	}
	return false;
}
static void        ClearForSearch  (C_BOARD *pBoard, SEARCHINFO *info) 
{
	//for (int i = 0; i < 13; i++) 
	//	for (int &searchHistory : pBoard->searchHistory[i])
    //        searchHistory = 0;
	memset(pBoard->searchHistory, 0, sizeof(int) * MAXPIECETYPE * BRD_SQ_NUM120);
	
	//for (int i = 0; i < 2; i++) 
    //		for (int &searchKillers : pBoard->searchKillers[i]) 
    //        searchKillers = 0;
	memset(pBoard->searchKillers, 0, sizeof(int) * 2* MAX_PLY_DEPTH);

	HASH.htNewWrites = 0;
	HASH.htOverWrite = 0;
	HASH.htHits      = 0;
	HASH.htCuts      = 0;

	pBoard->ply      = 0;

	info->stopped    = 0;
	info->nodes      = 0;
	info->fh         = 0;
	info->fhf        = 0;
}
                  
static void        Quiescence      (C_BOARD *pBoard, SEARCHINFO *pInfo, int alpha, int beta                                   ) 
{

	ASSERT(pBoard->CheckBoard());
	ASSERT(beta>alpha);

	if(( pInfo->nodes & 2047 ) == 0)
		CheckUp(pInfo);

	pInfo->nodes++;

	ASSERT(pBoard->bb64_wKings.BitCount() == 1);
	if (IsRepetition(pBoard) || pBoard->fiftyMove >= 100)
	{
		pInfo->bestScore = 0;
		return;
	}
	ASSERT(pBoard->bb64_wKings.BitCount() == 1);
	int Score = EVAL.EvalPosition(pBoard);
	ASSERT(pBoard->bb64_wKings.BitCount() == 1);

	if (pBoard->ply > MAX_PLY_DEPTH - 1)
	{
		pInfo->bestScore = Score;
		return;
	}
	ASSERT(pBoard->bb64_wKings.BitCount() == 1);
	ASSERT(Score>-CHESSINFINITE && Score<CHESSINFINITE);

	if (Score >= beta)
	{
		pInfo->bestScore = beta;
		return;
	}
	if(Score > alpha) 
		alpha = Score;

	S_MOVELIST movelist[1];
	ASSERT(pBoard->bb64_wKings.BitCount() == 1);
	C_MOVEGEN::mgGenerateMoves(pBoard,movelist,true); // just capital moves
	ASSERT(pBoard->bb64_wKings.BitCount() == 1);

	int LegalMoveCount   = 0;
	Score       = -CHESSINFINITE;

	for(int MoveNum = 0; MoveNum < movelist->size; ++MoveNum) 
	{
		PickNextMove(MoveNum, movelist);

        if ( !pBoard->MakeMove(movelist->moves[MoveNum].move))  
            continue;

		LegalMoveCount++;
        Quiescence(pBoard, pInfo, -beta, -alpha);
        pInfo->bestScore = -pInfo->bestScore;
        Score          =  pInfo->bestScore;
        pBoard->TakeMove();

		if (pInfo->stopped)
		{
			pInfo->bestScore = 0;
			return;
		}

		if (Score > alpha)
		{
			if (Score >= beta) 
			{
				if ( LegalMoveCount == 1 ) 
					pInfo->fhf++;
				pInfo->fh++;
				pInfo->bestScore = beta;
				return;
			}
			alpha = Score;
		}
    }

	pInfo->bestScore = alpha;
	return;
}
static SEARCHINFO *AlphaBeta       (C_BOARD *pBoard, SEARCHINFO *pInfo, int alpha, int beta, DEPTH depthRemaining, bool DoNull) 
{
	ASSERT(pBoard->CheckBoard());
	ASSERT(beta>alpha);
	ASSERT(depthRemaining>=0);

	if (depthRemaining <= 0)
    {
		Quiescence(pBoard, pInfo, alpha, beta);
        return pInfo;
    }

	if ( (pInfo->nodes & 2047) == 0)
		CheckUp(pInfo);

	pInfo->nodes++;

	if ((IsRepetition(pBoard) || pBoard->fiftyMove >= 100) && pBoard->ply)
	{
		pInfo->bestScore = 0;
		return pInfo;
	}

	if (pBoard->ply > MAX_PLY_DEPTH - 1)
	{
		pInfo->bestScore = EVAL.EvalPosition(pBoard);
		return pInfo;
	}

	int KingInCheck = pBoard->SqAttackedKingSq();

	if ( KingInCheck )
		depthRemaining++;

	int Score  = -CHESSINFINITE;
	int PvMove = NOMOVE;

	if ( HASH.ProbeHashEntry(pBoard, &PvMove, &Score, alpha, beta, depthRemaining) )
	{   // found hashtable entry (previous position)
		HASH.htCuts++;
		pInfo->bestScore = Score;
		stopAllThreads   = true;
		return pInfo;
	}

	if ( DoNull && depthRemaining>=doNullDepth && !KingInCheck && pBoard->ply && pBoard->bigPce[pBoard->side] )
	{
		pBoard->MakeNullMove();
		AlphaBeta(pBoard, pInfo,  -beta, -beta + 1, depthRemaining-doNullDepth, false);
		pInfo->bestScore = -pInfo->bestScore;
        Score            =  pInfo->bestScore;
        pBoard->TakeNullMove();

		if (Score >= beta && abs(Score) < ISMATE)
		{
			pInfo->nullCut++;
			pInfo->bestScore = beta;
			stopAllThreads   = true;
			return pInfo;
		}

		if (pInfo->stopped)
		{
			pInfo->bestScore = 0;
			return pInfo;
		}
	}

	S_MOVELIST movelist[1];
	C_MOVEGEN::mgGenerateMoves(pBoard,movelist,false);

	int  LegalMoveCount = 0;
	int  OldAlpha       = alpha;
	int  BestMove       = NOMOVE;
	int  BestScore      = -CHESSINFINITE;
	bool foundPV        = false;
	
	Score = -CHESSINFINITE;

	if( PvMove != NOMOVE) 
	{
		for (int i=0; i<movelist->size; i++) 
		{
			if( movelist->moves[i].move == PvMove) 
			{
				movelist->moves[i].score = PVMOVEFOUND;
				foundPV = true;
				break;
			}
		}
	}

	for(int MoveNum = 0; MoveNum < movelist->size; MoveNum++)	
	{
		#pragma region Walk Branch
		PickNextMove(MoveNum, movelist);
        if ( !pBoard->MakeMove(movelist->moves[MoveNum].move))  
            continue;

		LegalMoveCount++;

        if ( foundPV )
        {
            AlphaBeta(pBoard, pInfo, -alpha - 1, -alpha, depthRemaining-1, true);
            pInfo->bestScore = -pInfo->bestScore;
            Score            =  pInfo->bestScore;
            if ( Score > alpha && Score < beta )
            {
                AlphaBeta(pBoard, pInfo, -beta, -alpha, depthRemaining-1, true);
                pInfo->bestScore = -pInfo->bestScore;
                Score            =  pInfo->bestScore;
            }
        }
        
#ifdef USE_FUTILITY_LIMIT
		bool futility = false;

		if ( (!foundPV) & (depthRemaining==1) )
		{   
			// at frontier node (node before a 'leaf')
			// call static evaluation 
			// add a FUTILITY_MARGIN
			int futility_score = EVAL.EvalPosition(pBoard);
			futility = (BestScore >= futility_score + FUTILITY_LIMIT);  
			if (futility)
			{
				pInfo->bestScore = -pInfo->bestScore;
				Score            = pInfo->bestScore;
			}
		}
	
		if (!futility)
#endif			
		{
			AlphaBeta(pBoard, pInfo, -beta, -alpha, depthRemaining-1,  true);
            pInfo->bestScore = -pInfo->bestScore;
            Score            = pInfo->bestScore;
        }

		pBoard->TakeMove();

		if (pInfo->stopped)
		{
			pInfo->bestScore = 0;
			return pInfo;
		}

		if ( Score > BestScore ) 
		{
			pInfo->bestScore = Score;
			BestScore      = pInfo->bestScore = Score;
			pInfo->bestMove  = movelist->moves[MoveNum].move;
			BestMove       = pInfo->bestMove;
			if (Score > alpha) 
			{
				if (Score >= beta) 
				{
					if ( LegalMoveCount == 1 ) 
						pInfo->fhf++;
					pInfo->fh++;
					if ( !MV_PIECECAPTURE(BestMove) ) 
					{
						pBoard->searchKillers[1][pBoard->ply] = pBoard->searchKillers[0][pBoard->ply];
						pBoard->searchKillers[0][pBoard->ply] = BestMove;
					}
					HASH.StoreHashEntry(pBoard, BestMove, beta, HFBETA, depthRemaining);
					pInfo->bestScore = beta;
					return pInfo;
				}

				alpha = Score;

				if ( !MV_PIECECAPTURE(BestMove) )
					pBoard->searchHistory[pBoard->pieces[MV_FROMSQ(BestMove)]][MV_TOSQ(BestMove)] += depthRemaining;
			}
		}
		#pragma endregion
    }

	if( LegalMoveCount == 0)  
	{
		pInfo->bestScore = ( KingInCheck ) ? -CHESSINFINITE + pBoard->ply : 0;
		return pInfo;
	}

	ASSERT(alpha >= OldAlpha);

	if(alpha != OldAlpha) 
		HASH.StoreHashEntry(pBoard, BestMove, BestScore, HFEXACT, depthRemaining);
	else 
		HASH.StoreHashEntry(pBoard, BestMove, alpha    , HFALPHA, depthRemaining);

	pInfo->bestScore = alpha;
	return pInfo;
}

       void        Search_Position (C_BOARD *pBoard, SEARCHINFO *pInfo) 
{

	CLOCK_START;
    CLOCK_OFF;

	int   bestMove    = NOMOVE;
	int   bestScore   = -CHESSINFINITE;
    bool  bookMove    = false;

	pInfo->matein_plymoves = -1;
	ClearForSearch(pBoard,pInfo);
	
	#pragma region Check for Book Move
	if ( EngineOptions->UseBook ) 
	{
		bestMove = BOOK.GetBookMove(pBoard);
		if (bestMove != NOMOVE)
		{
			cout << "info Found Book Move!" << endl;
            bookMove  = true;
			bestScore = 0;
            pInfo->bestMove = bestMove;
		}
	}
	else
	{
		cout << "info NOT using Book Moves" << endl;
	}
	#pragma endregion

	if (!bookMove) // no book move found
	{
		if ((IsRepetition(pBoard) || pBoard->fiftyMove >= 100) && pBoard->ply)
		{
			pInfo->bestScore = 0;
		}
		else
		if (pBoard->ply > MAX_PLY_DEPTH - 1)
		{
			pInfo->bestScore = EVAL.EvalPosition(pBoard);
		}
		else
		{
			// generate First Level Move List	
			S_MOVELIST firstLvlMovelist[1];
			C_MOVEGEN::mgGenerateMoves(pBoard, firstLvlMovelist, false);
			for (int i = 0; i < firstLvlMovelist->size; i++)
				firstLvlMovelist->moves[i].score = 0;

			// reiteratively walk branches
			for (int iterativeDepth = 1; iterativeDepth <= pInfo->depth; iterativeDepth++) // iterative deepening
			{
				#pragma region Search Tree for Best Move
				doNullDepth = (iterativeDepth < 8) ? 5 : 4;
				stopAllThreads = false;
				//PrintMoveListScores("at Iterative Depth",iterativeDepth,firstLvlMovelist);
				switch ((iterativeDepth < THREADSEARCHDEPTHTHRESHHOLD) ? SEARCHTYPE_THREADS_NONE : searchKind)
				{
					default:
					case SEARCHTYPE_THREADS_NONE:   CLOCK_ON; Search_Thread_None  (pBoard, pInfo, iterativeDepth, firstLvlMovelist); CLOCK_OFF; break;
					case SEARCHTYPE_THREADS_SIMPLE: CLOCK_ON; Search_Thread_Simple(pBoard, pInfo, iterativeDepth, firstLvlMovelist); CLOCK_OFF; break;
					case SEARCHTYPE_THREADS_YBWC:   CLOCK_ON; Search_Thread_YBWC  (pBoard, pInfo, iterativeDepth, firstLvlMovelist); CLOCK_OFF; break;
				}

				bestScore = pInfo->bestScore;

				#pragma region display stats
				cout << "info depth " << setw( 2) << iterativeDepth
					<< " score cp "   << setw(11) << bestScore
					<< " nodes "      << setw(11) << pInfo->nodes
					<< " time "       << setw(11) << GetTimeMs() - pInfo->starttime;

				cout << " pv";
				int pvMoves = pBoard->GetPvLine(pBoard, iterativeDepth);
				for (int pvNum = 0; pvNum < pvMoves; pvNum++)
					cout << " " << PrMove(pBoard->PvArray[pvNum]);
				pInfo->bestMove = pBoard->PvArray[0];
				bestMove = pInfo->bestMove;
				cout << endl;

				#ifdef DEBUG
				cout << "Hits:" << HASH.htHits
					<< " htOverWrite:" << HASH.htOverWrite
					<< " htNewWrites:" << HASH.htNewWrites
					<< " Cut:" << HASH.htCuts
					<< " Ordering " << setprecision(2) << (pInfo->fhf / pInfo->fh) * 100.
					<< " NullCut:" << pInfo->nullCut << endl;
				#endif

				#pragma endregion

				if (abs(bestScore) >= ISMATE) break;
				if (pInfo->stopped) break; // ran out of time or asked to quit
				#pragma endregion
			}
			PrintMoveListScores("BestMove Scores", (int)pInfo->depth, firstLvlMovelist);
		}
	}

	ASSERT(bestMove != NOMOVE);
    ASSERT(bestMove == pInfo->bestMove);

   if ( pInfo->GAME_MODE == CONSOLEMODE )
   {
      U64 totalcount = HASH.htOverWrite + HASH.htNewWrites;
	  cout << "htHits:"       << FormatWithCommas(HASH.htHits 	   )
		   << " htOverWrite:" << FormatWithCommas(HASH.htOverWrite)  << "(" << (HASH.htOverWrite*(U64)100)/max((U64)1,totalcount) << "%)"
		   << " htNewWrites:" << FormatWithCommas(HASH.htNewWrites)  << "(" << (HASH.htNewWrites*(U64)100)/max((U64)1,totalcount) << "%)"
		   << " Cut:"         << FormatWithCommas(HASH.htCuts     )  << "(" << (HASH.htCuts     *(U64)100)/max((U64)1,totalcount) << "%)"
		   << " Ordering "    << setprecision(2) << (pInfo->fhf / pInfo->fh) * 100
		   << " NullCut:"     << pInfo->nullCut
		   << endl;
   }

	if ( abs(bestScore) >= ISMATE ) // mate found
	{
	    #pragma region Display MATE in 'n' moves message
		pInfo->matein_plymoves = MateInHalfMoves(bestScore);
        string s;
        string color;

        if (pInfo->matein_plymoves  % 2 == 0)
			color = (pBoard->side == WHITE) ? "BLACK" : "WHITE";
		else
            color = (pBoard->side == WHITE) ? "WHITE" : "BLACK";

		if (pInfo->matein_plymoves == 0)
		{
			s = "info " + color + " MATE!!";
			cout << s << endl;
		}
        else
        if (pInfo->matein_plymoves % 2 == 0)
        {
			cout << "info ====> there appears to be MATE for " << color << " in " << pInfo->matein_plymoves / 2 << " move(s)" << endl;
        }
        else
        {
			cout << "info ====> there appears to be MATE for " << color << " in " << pInfo->matein_plymoves << " ply move(s)" << endl;
        }
	    #pragma endregion
	}

    #pragma region Cleanup and Return Result                 

    bestMove = pInfo->bestMove; 
    cout << "info-> Best Move: piece=" << cPceChar[pBoard->pieces[MV_FROMSQ(bestMove)]] 
		 << ", fromsq="                << MV_FROMSQ(bestMove)
		 << ", tosq="                  << MV_TOSQ  (bestMove)
		 ;

    if ( MV_PROMOTED(bestMove) != EMPTY )
        cout << " promoted=" << cPceChar[MV_PROMOTED(bestMove)];

    if ( MV_CAPTURED(bestMove) != EMPTY )
        cout << " captured=" << cPceChar[MV_CAPTURED(bestMove)];
    cout << endl;
	cout << "bestmove " << PrMove(bestMove) << endl;

	if( pInfo->GAME_MODE == CONSOLEMODE ) 	
		pBoard->MakeMove(bestMove);

	//pInfo->bestMove  = bestMove;
	//pInfo->bestScore = bestScore;
   
	#pragma endregion

	CLOCK_ON;
	CLOCK_STOP;
    return;
}


#pragma region THREAD ROUTINES and VARIABLES

int searchActiveTasks = 0;
struct THREADMAP_SEARCH
{
      boost::thread     thread;
      SEARCHINFO*       tInfo; 
	  int               MoveNum;
	  bool              threadCompleted;
};    
vector<THREADMAP_SEARCH*>  threadmaps;

static void        ABThread             (THREADMAP_SEARCH *tm, C_BOARD *tBoard, SEARCHINFO *tInfo, int alpha, int beta, DEPTH depthRemaining, bool DoNull)
{
	AlphaBeta (tBoard, tInfo, alpha, beta, depthRemaining, DoNull);
	THREAD_LOCK_PROTECT(tm->threadCompleted = true; --searchActiveTasks; );
}

#define PROCESSALLCOMPLETEDTASKS                                                                                        \
{																														\
	for (auto tm : threadmaps)																							\
	{																													\
		if ( tm == NULL           ) continue; /* not a legal move or already handled */  								\
		if ( !tm->threadCompleted ) continue;                 								    						\
		Score = -tm->tInfo->bestScore;	                                                                                \
        firstLvlMovelist->moves[tm->MoveNum].score = Score;                                                             \
		if (Score > BestScore)																							\
		{	pInfo->bestScore = Score;																					\
			BestScore        = Score;																					\
			pInfo->bestMove  = firstLvlMovelist->moves[tm->MoveNum].move;												\
			BestMove         = pInfo->bestMove;																			\
			if (Score > alpha)																							\
			{	if (Score >= beta)																						\
				{	if (LegalMoveCount == 1) pInfo->fhf++;																\
					pInfo->fh++;																						\
					if (!MV_PIECECAPTURE(BestMove))																		\
					{																									\
						pBoard->searchKillers[1][pBoard->ply] = pBoard->searchKillers[0][pBoard->ply];					\
						pBoard->searchKillers[0][pBoard->ply] = BestMove;												\
					}																									\
					HASH.StoreHashEntry(pBoard, BestMove, beta, HFBETA, iterativeDepth);								\
					pInfo->bestScore = beta;																			\
					THREAD_WAITFOR(searchActiveTasks == 0);                                                             \
					threadmaps.clear();                                                                                 \
					return pInfo;  																						\
				}																										\
				alpha = Score;																							\
				if (!MV_PIECECAPTURE(BestMove))																			\
					pBoard->searchHistory[pBoard->pieces[MV_FROMSQ(BestMove)]][MV_TOSQ(BestMove)] += iterativeDepth;	\
			}																											\
		}																												\
        pInfo->nodes += tm->tInfo->nodes;																				\
		threadmaps[tm->MoveNum] = NULL;                                                                                 \
	}                                                                                                                   \
}


static SEARCHINFO *Search_Thread_YBWC   (C_BOARD *pBoard, SEARCHINFO *pInfo, DEPTH iterativeDepth, S_MOVELIST *firstLvlMovelist)
{
	int  alpha          = -CHESSINFINITE;
	int  beta           = CHESSINFINITE;
	bool DoNull         = true;
	int  Score          = -CHESSINFINITE;
	int  PvMove         = NOMOVE;
	int  LegalMoveCount = 0;
	int  OldAlpha       = alpha;
	int  BestMove       = NOMOVE;
	int  BestScore      = -CHESSINFINITE;
	bool foundPV        = false;
	int  KingInCheck    = pBoard->SqAttackedKingSq();

	#pragma region setup
	if (KingInCheck)
		iterativeDepth++;

	if ( HASH.ProbeHashEntry(pBoard, &PvMove, &Score, alpha, beta, iterativeDepth) )
	{
		// found hashtable entry (previous position) at required depth
		HASH.htCuts++;
		pInfo->bestScore = Score;
		pInfo->bestMove  = PvMove;
		stopAllThreads   = true;
		return pInfo;
	}

	if (PvMove != NOMOVE)
	{
		for (int i = 0; i < firstLvlMovelist->size; i++)
		{
			if ( firstLvlMovelist->moves[i].move == PvMove )
			{
				firstLvlMovelist->moves[i].score = PVMOVEFOUND;
				foundPV = true;
				break;
			}
		}
	}
	#pragma endregion

	ASSERT(searchActiveTasks == 0);
	int startedThreadCount = 0;

	for (int MoveNum = 0; MoveNum < firstLvlMovelist->size; MoveNum++)
	{

		#pragma region Determine next move
		PickNextMove(MoveNum, firstLvlMovelist);
		if ( !pBoard->MakeMove(firstLvlMovelist->moves[MoveNum].move) )
		{
			// not a legal move
			threadmaps.push_back(NULL);
			continue;
		}
		#pragma endregion
		LegalMoveCount++;
		#pragma region Walk Branch
		#define oldestBrother   (LegalMoveCount <= 1) 

		THREADMAP_SEARCH  *tm    = new THREADMAP_SEARCH();

		tm->tInfo                = new SEARCHINFO();;
		tm->tInfo->pThreadBoard  = NULL;
		tm->MoveNum              = MoveNum;
		threadmaps.push_back(tm);

		if (!stopAllThreads)
		{
			if (iterativeDepth < THREADSEARCHDEPTHTHRESHHOLD)
			{   // perform this branch directly (no thread)
				AlphaBeta(pBoard, tm->tInfo, -beta, -alpha, iterativeDepth - 1, true);
				tm->tInfo->bestScore = -tm->tInfo->bestScore;
				tm->threadCompleted = true;
			}
			else
			{
				// Start a Thread on this branch
				tm->threadCompleted = false;
				tm->tInfo->pThreadBoard = pBoard->DeepClone();
				THREAD_START2(boost::bind(ABThread, tm, tm->tInfo->pThreadBoard, tm->tInfo, -beta, -alpha, iterativeDepth - 1, true), searchActiveTasks);
				startedThreadCount++;
				//if (oldestBrother) THREAD_WAITFOR(searchActiveTasks <= 0); // wait for oldestBrother
			}
		}

		pBoard->TakeMove();
		THREAD_WAITFOR(searchActiveTasks < maxSearchThreads);
		PROCESSALLCOMPLETEDTASKS;
		
	    #pragma endregion
	}

	// wait for all threads to finish
	cout << "Started Thread Count = " << startedThreadCount << endl;
	THREAD_WAITFOR(searchActiveTasks == 0);
	PROCESSALLCOMPLETEDTASKS;
	threadmaps.clear();

	#pragma region Finish up

	if (LegalMoveCount == 0)
	{
		pInfo->bestScore = (KingInCheck) ? -CHESSINFINITE + pBoard->ply : 0;
		return pInfo;
	}

	ASSERT(alpha >= OldAlpha);

	if (alpha != OldAlpha)
		HASH.StoreHashEntry(pBoard, BestMove, BestScore, HFEXACT, iterativeDepth);
	else
		HASH.StoreHashEntry(pBoard, BestMove, alpha, HFALPHA, iterativeDepth);

	pInfo->bestScore = alpha;

	#pragma endregion

	return pInfo;
}
static SEARCHINFO *Search_Thread_Simple (C_BOARD *pBoard, SEARCHINFO *pInfo, DEPTH iterativeDepth, S_MOVELIST *firstLvlMovelist)
{
	int  alpha          = -CHESSINFINITE;
	int  beta           =  CHESSINFINITE;
	bool DoNull         = true;
	int  Score          = -CHESSINFINITE;
	int  PvMove         = NOMOVE;
	int  LegalMoveCount = 0;
	int  OldAlpha       = alpha;
	int  BestMove       = NOMOVE;
	int  BestScore      = -CHESSINFINITE;
	bool foundPV        = false;
	int  KingInCheck    = pBoard->SqAttackedKingSq();

	#pragma region setup
	if ( KingInCheck ) 
		iterativeDepth++;

	if (HASH.ProbeHashEntry(pBoard, &PvMove, &Score, alpha, beta, iterativeDepth))
	{   
		// found hashtable entry (previous position)
		HASH.htCuts++;
		pInfo->bestScore = Score;
		pInfo->bestMove  = PvMove;
		stopAllThreads = true;
		return pInfo;
	}

	if (PvMove != NOMOVE)
	{
		for (int i = 0; i < firstLvlMovelist->size; i++)
		{
			if (firstLvlMovelist->moves[i].move == PvMove)
			{
				firstLvlMovelist->moves[i].score = PVMOVEFOUND;
				foundPV = true;
				break;
			}
		}
	}

	#pragma endregion

	ASSERT(searchActiveTasks == 0);

	for (int MoveNum = 0; MoveNum < firstLvlMovelist->size; MoveNum++)
	{
		#pragma region Walk Branch and Start Threads
		PickNextMove(MoveNum, firstLvlMovelist);
		if (!pBoard->MakeMove(firstLvlMovelist->moves[MoveNum].move))
		{
			// not a legal move
			threadmaps.push_back(NULL);
			continue;
		}

		LegalMoveCount++;

		THREADMAP_SEARCH  *tm    = new THREADMAP_SEARCH();
		SEARCHINFO        *tInfo = new SEARCHINFO();

		tm->tInfo                = tInfo;
		tInfo->pThreadBoard      = NULL;
		tm->MoveNum              = MoveNum;

		if ( iterativeDepth < THREADSEARCHDEPTHTHRESHHOLD )
		{	// perform directly (no thread)
			AlphaBeta(pBoard, tInfo, -beta, -alpha, iterativeDepth - 1, true);
			tInfo->bestScore = -tInfo->bestScore;
			tm->threadCompleted = true;
		}
		else
		{   // start a thread
			tInfo->pThreadBoard = pBoard->DeepClone();
			tm->threadCompleted = false;
			THREAD_START2(boost::bind(ABThread, tm, tInfo->pThreadBoard, tInfo, -beta, -alpha, iterativeDepth - 1, true), searchActiveTasks);
		}

		pBoard->TakeMove();
		threadmaps.push_back(tm);
 
		THREAD_WAITFOR(searchActiveTasks<maxSearchThreads);
		PROCESSALLCOMPLETEDTASKS;
		#pragma endregion
	}

	// wait for all threads to finish
	THREAD_WAITFOR(searchActiveTasks == 0);
	PROCESSALLCOMPLETEDTASKS;
	threadmaps.clear();

    #pragma region Finish up

	if (LegalMoveCount == 0)
	{
		pInfo->bestScore = (KingInCheck) ? -CHESSINFINITE + pBoard->ply : 0;
		return pInfo;
	}

	ASSERT(alpha >= OldAlpha);

	if (alpha != OldAlpha)
		HASH.StoreHashEntry(pBoard, BestMove, BestScore, HFEXACT, iterativeDepth);
	else
		HASH.StoreHashEntry(pBoard, BestMove, alpha, HFALPHA, iterativeDepth);

	pInfo->bestScore = alpha;
    
	#pragma endregion
	
	return pInfo;
}
static SEARCHINFO *Search_Thread_None   (C_BOARD *pBoard, SEARCHINFO *pInfo, DEPTH iterativeDepth, S_MOVELIST *firstLvlMovelist)
{
	int  alpha          = -CHESSINFINITE;
	int  beta           = CHESSINFINITE;
	bool DoNull         = true;
	int  Score          = -CHESSINFINITE;
	int  PvMove         = NOMOVE;
	int  LegalMoveCount = 0;
	int  OldAlpha       = alpha;
	int  BestMove       = NOMOVE;
	int  BestScore      = -CHESSINFINITE;
	bool foundPV        = false;
	int  KingInCheck    = pBoard->SqAttackedKingSq();

	#pragma region setup
	if ( KingInCheck )
		iterativeDepth++;

	if (HASH.ProbeHashEntry(pBoard, &PvMove, &Score, alpha, beta, iterativeDepth))
	{   
		// found hashtable entry (previous position)
		HASH.htCuts++;
		pInfo->bestScore = Score;
		return pInfo;
	}

	if (PvMove != NOMOVE)
	{
		for (int i = 0; i< firstLvlMovelist->size; i++)
		{
			if (firstLvlMovelist->moves[i].move == PvMove)
			{
				firstLvlMovelist->moves[i].score = PVMOVEFOUND;
				foundPV = true;
				break;
			}
		}
	}
	#pragma endregion

	for (int MoveNum = 0; MoveNum < firstLvlMovelist->size; MoveNum++)
	{
		#pragma region Walk Branch
		PickNextMove(MoveNum, firstLvlMovelist);
		if (!pBoard->MakeMove(firstLvlMovelist->moves[MoveNum].move))
			continue;

		LegalMoveCount++;

		if (foundPV)
		{
			AlphaBeta(pBoard, pInfo, -alpha - 1, -alpha, iterativeDepth - 1, true);
			pInfo->bestScore = -pInfo->bestScore;
			Score = pInfo->bestScore;
			if (Score > alpha && Score < beta)
			{
				AlphaBeta(pBoard, pInfo, -beta, -alpha, iterativeDepth - 1, true);
				pInfo->bestScore = -pInfo->bestScore;
				Score = pInfo->bestScore;
			}
		}
		else
		{
			AlphaBeta(pBoard, pInfo, -beta, -alpha, iterativeDepth - 1, true);
			pInfo->bestScore = -pInfo->bestScore;
			Score = pInfo->bestScore;
		}
		pBoard->TakeMove();
		firstLvlMovelist->moves[MoveNum].score = Score;

		if (pInfo->stopped)
		{
			pInfo->bestScore = 0;
			return pInfo;
		}

		if (Score > BestScore)
		{
			pInfo->bestScore = Score;
			BestScore = pInfo->bestScore = Score;
			pInfo->bestMove = firstLvlMovelist->moves[MoveNum].move;
			BestMove = pInfo->bestMove;
			if (Score > alpha)
			{
				if (Score >= beta)
				{
					if (LegalMoveCount == 1)
						pInfo->fhf++;
					pInfo->fh++;
					if (!MV_PIECECAPTURE(BestMove))
					{
						pBoard->searchKillers[1][pBoard->ply] = pBoard->searchKillers[0][pBoard->ply];
						pBoard->searchKillers[0][pBoard->ply] = BestMove;
					}
					HASH.StoreHashEntry(pBoard, BestMove, beta, HFBETA, iterativeDepth);
					pInfo->bestScore = beta;
					return pInfo;
				}

				alpha = Score;

				if (!MV_PIECECAPTURE(BestMove))
					pBoard->searchHistory[pBoard->pieces[MV_FROMSQ(BestMove)]][MV_TOSQ(BestMove)] += iterativeDepth;
			}
		}
	#pragma endregion
	}

	#pragma region Finish up

	if (LegalMoveCount == 0)
	{
		pInfo->bestScore = (KingInCheck) ? -CHESSINFINITE + pBoard->ply : 0;
		return pInfo;
	}

	ASSERT(alpha >= OldAlpha);

	if (alpha != OldAlpha)
		HASH.StoreHashEntry(pBoard, BestMove, BestScore, HFEXACT, iterativeDepth);
	else
		HASH.StoreHashEntry(pBoard, BestMove, alpha, HFALPHA, iterativeDepth);

	pInfo->bestScore = alpha;
	
	#pragma endregion
	
	return pInfo;
}

#pragma endregion


