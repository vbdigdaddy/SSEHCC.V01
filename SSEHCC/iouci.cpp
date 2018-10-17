// uci.c

#include "stdafx.h"
#include "C_BOARD.h"
#include "CS_HASHTABLE.h"
 
#pragma warning( disable: 4996)

#define INPUTBUFFER 400 * 6


void ParseGo(string line, SEARCHINFO *pInfo, C_BOARD *pBoard) 
{
	//UCI *go
	//
	//	start calculating on the current position
	//	There are a number of commands that can follow this command, all will be sent in the same string.
	//	If one command is not send its value should be interpreted as it would not influence the search. 
	//
	//	* searchmoves <move1> .... <movei>
	//	       restrict search to this moves only
	//	* ponder
	//	       start searching in pondering move.
	//	       Do not exit the search in ponder mode, even if it's mate!
	//	       This means that the last move sent in in the position string is the ponder move.
	//	       The engine can do what it wants to do, but after a "ponderhit" command
	//	       it should execute the suggested move to ponder on.
	//	* wtime <x>
	//	       white has x msec left on the clock
	//	* btime <x>
	//	       black has x msec left on the clock
	//	* winc <x>
	//	       white increment per move in mseconds if x > 0
	//	* binc <x>
	//	       black increment per move in mseconds if x > 0
	//	* movestogo <x>
	//	       there are x moves to the next time control,
	//	             this will only be sent if x > 0,
	//	             if you don't get this and get the wtime and btime it's sudden death
	//	* depth <x>
	//		  search x plies only.
	//	* nodes <x>
	//  		search x nodes only,
	//	* mate <x>
	//	    	search for a mate in x moves
	//	* movetime <x>
	//		    search exactly x mseconds
	//	* infinite
	//		    search until the "stop" command.Do not exit the search without being told so in this mode!



	DEPTH depth      = -1;
	int   movestogo  = 30;
	int   movetime   = -1;
	int   time       = -1;
	int   inc        =  0;
	pInfo->timeset   = false;

	while ( line.size() > 0 )
	{
		skan(line);
		if ( token.compare("infinite" ) == 0                        ) {	; }
		if ( token.compare("binc"     ) == 0  && pBoard->side == BLACK ) { inc       = stoi(skan(line)); continue; }
		if ( token.compare("winc"     ) == 0  && pBoard->side == WHITE ) { inc       = stoi(skan(line));	continue; }
		if ( token.compare("wtime"    ) == 0  && pBoard->side == WHITE ) { time      = stoi(skan(line));	continue; }
		if ( token.compare("btime"    ) == 0  && pBoard->side == BLACK ) { time      = stoi(skan(line));	continue; }
		if ( token.compare("movestogo") == 0                        ) { movestogo = stoi(skan(line));	continue; }
		if ( token.compare("movetime" ) == 0                        ) { movetime  = stoi(skan(line));	continue; }
		if ( token.compare("depth"    ) == 0                        ) { depth     = stoi(skan(line));	continue; }
		break; // should not get here
	}

	if(movetime != -1) 
	{
		time = movetime;
		movestogo = 1;
	}

	pInfo->starttime = GetTimeMs();
	pInfo->depth = depth;

	if(time != -1) 
	{
		pInfo->timeset = true;
		time /= movestogo;
		time -= 50;
		pInfo->stoptime = pInfo->starttime + time + inc;
	}

	if(depth == -1) 
		pInfo->depth = MAX_PLY_DEPTH;

	// cout "time:" << time << " start:" << info->starttime << " stop:" << info->stoptime << "depth:" << info->depth << " timeset:" << info->timeset << endl;
	// PrintBoard(pos); 
	Search_Position(pBoard, pInfo);

}

void ParsePosition(string line, C_BOARD *pBoard) 
{
	//
	// * position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
	//
    //   set up the position described in fenstring on the internal board and
	//	 play the moves on the internal chess pBoard->
	//	 if the game was played  from the start postion the string "startpos" will be sent
	//	 Note : no "new" command is needed.

	string cmd = skan(line);

    if (cmd.compare("startpos") == 0 )
	{
        pBoard->ParseFen(START_FEN);
    } 
	else 
	{
        if (cmd.compare("fen") != 0 )
		{
            pBoard->ParseFen(START_FEN);
        } 
		else 
		{
			string fen = skan(line);
            pBoard->ParseFen(fen);
        }
    }

	if (skan(line) == "moves")
	{
		skan(line);
        while( token.size() > 0 ) 
		{
              int move = ParseMove(token,pBoard);
			  if(move == NOMOVE) 
				  break;
			  pBoard->MakeMove(move);
              pBoard->ply=0;
			  skan(line);
        }
    }
	//pBoard->PrintBoard();
}

void Uci_Loop(C_BOARD *pBoard, SEARCHINFO *pInfo)
{

	pInfo->GAME_MODE = UCIMODE;

                
	PRINT_ID_NAME;
    cout << "id author Petersen" << endl;
	cout << "option name Hash type spin default " << CS_HASHTABLE::MIN_HASHTABLE_SIZE << " min " << CS_HASHTABLE::MIN_HASHTABLE_SIZE << " max " << CS_HASHTABLE::DEFAULT_HASHTABLE_SIZE << endl;

	cout << "option name Book type check default true" << endl;
    cout << "uciok" << endl;

	while (true) 
	{

		if (pInfo->quit)
			break;
        
		GetCMDLine();
		string cmd = skan(cmdLine);

		if ( cmd.compare("isready"   ) == 0 ) { cout << "readyok" << endl;                     continue; }
		if ( cmd.compare("position"  ) == 0 ) { ParsePosition(cmdLine, pBoard);                continue; }
		if ( cmd.compare("ucinewgame") == 0 ) { ParsePosition("position startpos\n", pBoard);  continue; } 
		if ( cmd.compare("go"        ) == 0 ) { ParseGo(cmdLine, pInfo, pBoard);               continue; }
		if ( cmd.compare("quit"      ) == 0 ) { pInfo->quit = true;                            break;    }

		if ( cmd.compare("uci"       ) == 0 ) { PRINT_ID_NAME;
		                                        cout << "id author Semajris" << endl;
		                                        cout << "uciok" << endl;
												continue;
											  } 

		if ( cmd.compare("setoption name Book value ") == 0 )
		{			
			EngineOptions->UseBook = (cmdLine.compare("true") == 0);
			continue;
		}

    }
    return;
}
