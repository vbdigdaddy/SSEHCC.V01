// ioconsole.c
#include "stdafx.h"
#include "C_BOARD.h"
#include "C_MOVEGEN.h"
#include "CS_EVALUATE.h"
#include "CS_HASHTABLE.h"
 
#pragma warning( disable: 4996)

int  ThreeFoldRep(C_BOARD *pBoard) 
{

	ASSERT(pBoard->CheckBoard());

	int i = 0, r = 0;
	for (i = 0; i < pBoard->hisPly; ++i)	
	{
	    if (pBoard->history[i].posKey == pBoard->posKey) 
		{
		    r++;
		}
	}
	return r;
}
int  DrawMaterial(C_BOARD *pBoard) 
{
	ASSERT(pBoard->CheckBoard());

	// Do we have any Pawns(then not a draw)
	if ( (pBoard->bb64_wPawns | pBoard->bb64_bPawns) != 0 )
		return false;

	// Do we have an Queens or Rooks(then not a draw)
	if ( (pBoard->bb64_wQueens | pBoard->bb64_bQueens | pBoard->bb64_wRooks | pBoard->bb64_bRooks) != 0 )
		return false;

	// do we have 2 or more bishops
	if ( pBoard->bb64_wBishops.BitCount() > 1 || pBoard->bb64_bBishops.BitCount() > 1 )
		return false;

	// do we have 2 or more knights
	if ( pBoard->bb64_wKnights.BitCount() > 1 || pBoard->bb64_bKnights.BitCount() > 1 )
        return false;

	// do we have 2 or more knights & bishops
	if ( pBoard->bb64_wKnights.BitCount() && pBoard->bb64_wBishops.BitCount() )
		return false;
	if ( pBoard->bb64_bKnights.BitCount() && pBoard->bb64_bBishops.BitCount() )
		return false;
	
	// we have a draw (not enough material present on board for anyone to win
	return true;
}
bool checkresult (C_BOARD *pBoard) 
{
	ASSERT(pBoard->CheckBoard());

    if (pBoard->fiftyMove > 100) 
	{
       cout << "1/2-1/2 {fifty move rule (claimed by SSEHCC)}" << endl; 
	   return true;
    }

    if (ThreeFoldRep(pBoard) >= 2) 
	{
       cout << "1/2-1/2 {3-fold repetition (claimed by SSEHCC)}" << endl;
	   return true;
    }

	if ( DrawMaterial(pBoard) ) 
	{
       cout << "1/2-1/2 {insufficient material (claimed by SSEHCC)}" << endl; 
	   return true;
    }

	S_MOVELIST movelist[1];
	C_MOVEGEN::mgGenerateMoves(pBoard,movelist,false);

	int found = 0;
	for(int MoveNum = 0; MoveNum < movelist->size; ++MoveNum) 
	{
        if ( !pBoard->MakeMove(movelist->moves[MoveNum].move)) 
            continue;
        found++;
		pBoard->TakeMove();
		break;
    }

	if (found == 0)
	{ // no moves found (checkmate or statemate)
		int KingInCheck = pBoard->SqAttackedKingSq();
		if (KingInCheck)
		{
			if (pBoard->side == WHITE)
			{
				cout << "0-1 {black mates (claimed by SSEHCC)}" << endl;
				return true;
			}
			else
			{
				cout << "0-1 {white mates (claimed by SSEHCC)}" << endl;
				return true;
			}
		}
		else
		{
			cout << "\n1/2-1/2 {stalemate (claimed by SSEHCC)}" << endl;
			return true;
		}
	}

	return false;
}

void PrintOptions() 
{
	cout << "feature ping=1 setboard=1 colors=0 usermove=1 memory=1" << endl;
	cout << "feature done=1" << endl;
}

void HandleCmdHelp  ()
{
	cout << "Commands:"                                                                                                                << endl 
	     << "quit       - quit game"                                                                                                   << endl 
	     << "force      - computer will not think"                                                                                     << endl 
	     << "print      - show board"                                                                                                  << endl 
	     << "post       - show thinking"                                                                                               << endl 
	     << "nopost     - do not show thinking"                                                                                        << endl 
	     << "new        - start new game"                                                                                              << endl 
	     << "go         - set computer thinking"                                                                                       << endl 
	     << "depth x    - set depth to x"                                                                                              << endl 
	     << "time x     - set thinking time to x seconds (depth still applies if set)"                                                 << endl 
	     << "view       - show current depth and movetime settings"                                                                    << endl 
	     << "setboard x - set position to fen x"                                                                                       << endl 
	                                                                                                                                   << endl 
	     << "perft testall                        - testmove 50000; testmate ; testbestmove"                                           << endl 
	     << "perft testeval                       - test evaluation of simple board positions"                                         << endl 
	     << "perft testmove     [total move count]- test moves counts  (ooo)"                                                          << endl 
	     << "perft testmate     [depth] [noclear] - test mates         (default depth 5; noclear => dont clear hash table)"            << endl 
	     << "perft testbestmove [parm ] [noclear] - test for best move (parm: [bktest] [solved||unsolved|all||<empty>]  empty=>solved" << endl 
	     << "perft testutil                       - test bit fuctions"                                                                 << endl 
	     << "perft testmagicsq                    - test magic square functions"                                                       << endl 
	     << "perft testsearch                     - test basic search tree evaluation"                                                 << endl 
         << "perft highestcount                   - print out highest generated move count (for a board position)"                     << endl 
	                                                                                                                                   << endl 
	     << "** note ** - to reset time and depth, set to 0"                                                                           << endl 
	     << "enter moves using b7b8q notation"                                                                                         << endl 
		                                                                                                                               << endl;
}


void HandleCmdPerft ()
{
	std::string ptest     = skan(cmdLine);
    bool        clearHash = cmdLine.find("noclear") == string::npos;
    int         d = 0;

	if ( !ptest.compare("mirror"      ) )  {                 PerftMirrorTest();                                    }    else
	if ( !ptest.compare("highestcount") )  {                 cout << "highest count=" << highestmovecount << endl; }    else
	if ( !ptest.compare("testall"     ) )  {                 PerftTestAll();                                       }    else
	if ( !ptest.compare("testeval"    ) )  {                 PerftTestEval();                                      }    else
	if ( !ptest.compare("testmove"    ) )  { SKANINT(d);     PerftTestMove(d);                                     }    else
	if ( !ptest.compare("testmate"    ) )  { SKANINT(d);     PerftTestMate(d,clearHash);                           }    else
	if ( !ptest.compare("testbestmove") )  {                 PerftTestBestMove(cmdLine,clearHash);                 }    else
	if ( !ptest.compare("testutil"    ) )  {                 PerftTestUtil();                                      }    else
	if ( !ptest.compare("testmagicsq" ) )  {                 PerftTestMagicSQ();                                   }    else
	if ( !ptest.compare("testsearch"  ) )  {                 PerftTestSearch();                                    }    else

	cout << "Unknown Perft test ->" << ptest << endl;
}
void HandleCmdEval  (C_BOARD *pBoard)
{
	pBoard->PrintBoard();
	cout << "Eval:" << EVAL.EvalPosition(pBoard) << endl;
	pBoard->MirrorBoard();
	pBoard->PrintBoard();
	cout << "Eval:" << EVAL.EvalPosition(pBoard) << endl;
}
void HandleCmdView  (DEPTH depth, int movetime)
{
	if (depth == MAX_PLY_DEPTH) 
		cout << "depth not set " << endl;
	else 
		cout << "depth " << depth << endl;

	if (movetime != 0) 
		cout << "movetime " << movetime/1000 << " seconds" << endl;
	else 
		cout << "movetime not set" << endl;
}
void HandleCmdNew   (C_BOARD *pBoard,int& engineSide)
{
	HASH.ClearHashTable();
	engineSide = BLACK;
	pBoard->ParseFen(START_FEN);
}


void Console_Loop   (C_BOARD *pBoard, SEARCHINFO *pInfo) 
{

	PRINT_ID_NAME;

	
	cout << "Type help for commands" << endl << endl;

	pInfo->GAME_MODE     = CONSOLEMODE;
	pInfo->POST_THINKING = true;

	DEPTH  depth      = MAX_PLY_DEPTH;
    int    movetime   = 9000;
	int    engineSide = BOTH;
	int    move       = NOMOVE;

	std::string cmd;

	engineSide = BLACK;
	pBoard->ParseFen(START_FEN);

	while(true) 
	{
		if(pBoard->side == engineSide && checkresult(pBoard) == false) 
		{
			pInfo->starttime = GetTimeMs();
			pInfo->depth     = depth;

			if(movetime != 0) 
			{
				pInfo->timeset = true;
				pInfo->stoptime = pInfo->starttime + movetime;
			}

			Search_Position(pBoard, pInfo);
		}

		cout << endl << "SSEHCC > ";

		GetCMDLine();
		haltOnError = cmdLine.compare("halt");
		cmd = skan(cmdLine);

		if ( !cmd.compare("help")     ) { HandleCmdHelp ();                                              continue; }
		if ( !cmd.compare("perft")    ) { HandleCmdPerft();                                              continue; }
		if ( !cmd.compare("eval")     ) { HandleCmdEval(pBoard);                                         continue; }
		if ( !cmd.compare("view")     ) { HandleCmdView(depth,movetime);                                 continue; }
																			         		     
		if ( !cmd.compare("depth")    ) { depth=stoi(skan(cmdLine)); if(depth==0) depth=MAX_PLY_DEPTH;   continue; }
 		if ( !cmd.compare("time")     ) { movetime=stoi(skan(cmdLine)); movetime *= 1000;                continue; }
		if ( !cmd.compare("go")       ) { engineSide = pBoard->side; 		                    	     continue; }
		if ( !cmd.compare("setboard") ) { engineSide = BOTH; pBoard->ParseFen(cmdLine);                  continue; }
		if ( !cmd.compare("print")    ) { pBoard->PrintBoard(); 		                                 continue; }
			 																				     	     
		if ( !cmd.compare("force")    ) { engineSide = BOTH;	          		                         continue; }
		if ( !cmd.compare("post")     ) { pInfo->POST_THINKING = true; 	 	                             continue; }
		if ( !cmd.compare("nopost")   ) { pInfo->POST_THINKING = false;                                  continue; }
			 																				     	     
		if ( !cmd.compare("new")      ) { HandleCmdNew(pBoard,engineSide);                               continue; }
		if ( !cmd.compare("quit")     ) { pInfo->quit = true;                                            break;    }

		try
		{
			move = ParseMove(token, pBoard);
		}
		catch (exception e)
		{
			move = NOMOVE;
		}
		if(move == NOMOVE)
		{
			cout << "Command unknown:'" << token << endl;
			continue;
		}
		pBoard->MakeMove(move);
		pBoard->ply=0;
    }

    return;
}
