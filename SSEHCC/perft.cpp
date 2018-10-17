// perft.c
#include "stdafx.h"
#include "C_BOOKTABLE.h"
#include "C_BB_64.h"
#include "C_BOARD.h"
#include "C_MOVEGEN.h"
#include "CS_HASHTABLE.h"
#include "CS_EVALUATE.h"

using namespace std;

long leafNodes;

struct S_MOVETEST
{
	int     lineNo;
	string  fen;
	DEPTH   depth;
	U64     moveCount;

	bool operator<(S_MOVETEST& m1)
	{
		return moveCount < m1.moveCount;
	}
};

struct S_BESTMOVETEST
{
    int          lineNo;
    string       fen;
    int          maxDepth;
    string       sBestMove;
    string       comment;
    long long    oldSolvedTim;
    long long    newSolvedTim;
    string       sFoundBestMove;
	bool         good;
};

vector<S_MOVETEST> moveTests;
S_MOVETEST         moveTest;

static void PrintStats (U64 counts[], U64 times[], int maxdepth )
{
	U64 totCount = 0;
	U64 totTime  = 0;
	for (int i = 1; i <= min(MAX_PLY_DEPTH,maxdepth); i++)
	{
		if (counts[i] > 0)
		{
			cout << "TestMate Stats:"
				 << "  Mate In="    << setw(2) << i 
				 << "  Count="      << setw(3) << counts[i] 
				 << "  Total Time=" << setw(9) << FormatWithCommas(times[i]) 
				 << "  Avg Time="   << setw(9) << FormatWithCommas(times[i] / counts[i]) 
				 << endl;
			totCount += counts[i];
			totTime  += times[i];
		}
	}

	if (totCount > 0)
	{
		cout << "________________________________________________________________________________" << endl;
		cout << "TestMate Stats Totals:     "
			 << "  Count="      << setw(3) << totCount
			 << "  Total Time=" << setw(9) << FormatWithCommas(totTime)
			 << "  Avg Time="   << setw(9) << FormatWithCommas(totTime / totCount)
			 << endl;
	}
}

static void tbu          (U64 bb, int expectedL0, int expectedT0, int expectedOnes)
{
	C_BB_64 bb64(bb);
	int L0      = bb64.BitLeading0();
	int T0      = bb64.BitTrailing0();
	int numBits = bb64.BitCount();

	ASSERT(expectedL0   == L0     );
	ASSERT(expectedT0   == T0     );
	ASSERT(expectedOnes == numBits);
}
static void testReverse  (U64 bb, U64 expectedResult)
{
	ASSERT(C_BB_64(bb).BitReverse() == expectedResult);
}
static void testPopBit   (int bitNo, U64 bb)
{
	C_BB_64 t_bb64(bb);
	C_BB_64 bb64(bb);

	int   bn = bb64.BitPop();
	t_bb64.ClrBit(bitNo);

	ASSERT(bitNo == bn);
	ASSERT(bb64  == t_bb64);
}

static void testDiagMoves(U64 occupiedSQs, int sq64, C_BB_64 bb64_expectedResult)
{
	C_BOARD *pBoard        = new C_BOARD();
	C_BB_64 bb64_theResult = C_MOVEGEN::mgDAndAntiDMoves(occupiedSQs,sq64);

	if (bb64_expectedResult != bb64_theResult)
		cout << "expected result " << setw(16) << bb64_expectedResult.value << endl
	         << "the Result      " << setw(16) << bb64_theResult.value      << endl;

	ASSERT(bb64_expectedResult == bb64_theResult);
}
static void testHVMoves  (C_BB_64 bb64_occupiedSQs, int sq64, C_BB_64 bb64_expectedResult)
{
	C_BOARD *pBoard        = new C_BOARD();
	C_BB_64 bb64_theResult = C_MOVEGEN::mgHAndVMoves(pBoard->bb64_OccupiedSQs,sq64);

	if (bb64_expectedResult != bb64_theResult)
		cout << "expected result    " << setw(16) << bb64_expectedResult.value << endl
		     << "the Result         " << setw(16) << bb64_theResult.value << endl;

	ASSERT(bb64_expectedResult == bb64_theResult);
}
static bool TestEval     (int lineNo,string fen, int expectedResult)
{
    C_BOARD *pBoard = new C_BOARD();
	pBoard->ParseFen(fen);    
	int rst = EVAL.EvalPosition(pBoard);

	if (rst == expectedResult)
		return true;
	
    cout << lineNo << ":" << " ERROR [Expected:" << expectedResult << "  Evalated:" << rst << endl << endl;
	pBoard->PrintBoard();
    return false;

}

void Perft             (DEPTH depth, C_BOARD *pBoard) 
{

    ASSERT(pBoard->CheckBoard());  

	if(depth == 0) 
	{
        leafNodes++;
        return;
    }	

	S_MOVELIST movelist[1];
	C_MOVEGEN::mgGenerateMoves(pBoard,movelist,false);
      
	for(int MoveNum = 0; MoveNum < movelist->size; ++MoveNum) 
	{	
		#ifdef DEBUGMOVE
		string s = "------------------------------------------------------------------------------------>";
		s = s.substr(s.length() - depth * 3) + PrMove(movelist[MoveNum].move);
		cout << s << endl;
		#endif

        if ( !pBoard->MakeMove(movelist->moves[MoveNum].move))  
            continue;

        Perft(depth - 1, pBoard);
        pBoard->TakeMove();
    }

    return;
}
long PerftTest         (DEPTH depth, C_BOARD *pBoard)
{

    ASSERT(pBoard->CheckBoard());

	leafNodes = 0;
	S_MOVELIST movelist[1];
	C_MOVEGEN::mgGenerateMoves(pBoard,movelist,false);

	for(int MoveNum = 0; MoveNum < movelist->size; ++MoveNum) 
	{
        int move = movelist->moves[MoveNum].move;

		#ifdef DEBUGMOVE
		string s = "------------------------------------------------------------------------------------>";
		s = s.substr(s.length() - (10-depth) * 3) + PrMove(movelist[MoveNum].move);
		pBoard->PrintBoard();
		cout << s >> endl;
		#endif        
		
		ASSERT(pBoard->CheckBoard());

		if ( !pBoard->MakeMove(move))
            continue;

        long cumnodes = leafNodes;
        Perft(depth - 1, pBoard);
        pBoard->TakeMove();   

		ASSERT(pBoard->CheckBoard());
        long oldnodes = leafNodes - cumnodes;
	}

    return leafNodes;
}
bool PerftTestAll      (                          )
{
    bool noerrors = true;

    HASH.ClearHashTable();
    noerrors &= PerftTestMove(100000);                  // 100,000,000 moves                 
    noerrors &= PerftTestMate(MAX_PLY_DEPTH,false);     // all mate tests      
    noerrors &= PerftTestBestMove(" ",false);           // just solved ones

    if ( noerrors )
       cout << "All tests finished correctly" << endl;
    else
       cout << "Some tests finished incorrectly" << endl;

    return noerrors;
}
bool PerftTestEval     (                          )
{
	ifstream fin;
	string   line;
	string   fen;
	string   comment;
	int      score;
	int      lineNo = 0;
    bool     noerrors = true;

	fin.open(EVALTESTSFILENAME,ios::in);

	while (getline(fin, line))
	{ // process line
		lineNo++;
		cout << lineNo << ":" << line << endl;
		if (line.length()>=2 && line.substr(0,2) == "//")
			continue;

		size_t sz;
		sz = line.find(';');  fen   = line.substr(0, sz);       line = line.substr(sz+1);
		sz = line.find(';');  score = stoi(line.substr(0, sz)); line = line.substr(sz+1);
		comment   = Trim(line);
		noerrors &= TestEval(lineNo,fen,score);
	}

	cout << "Test Eval Finished" << endl;
    return noerrors;
}
bool PerftTestMove     (int maxMoveCount          )
{
	ifstream fin;
	string   line;
	string   token;
	int      lineNo = 0;
	int      d;
	DEPTH    maxDepth = 6; // max depth entry in data file

	maxMoveCount *= 1000;

	#pragma region read file into memory 
	fin.open(MOVETESTSFILENAME);

	while (getline(fin, line))
	{ // process line
		moveTest.lineNo = ++lineNo;
		size_t sz = line.find(';');

		moveTest.fen = Trim(line.substr(0, sz));
       
		for (d = 1; d <= maxDepth; d++)
		{
			moveTest.depth = d;
			line = line.substr(sz + 4); // skip over ; Dx
			sz    = line.find(';'); 
			token = line.substr(0, sz);
			moveTest.moveCount = stoll(token);
			moveTests.push_back(moveTest);
		}
	}
	#pragma endregion

	// sort by Move Count (small to large) See: MoveTest Stuct for compare procedure
	std::sort(moveTests.begin(), moveTests.end());

	#pragma region Process Tests to requested max depth
	U64  totalTime  = 0;
	U64  totalMoves = 0;
	long testNo     = 0;

	for (int i = 0; i < (int)moveTests.size(); i++)
	{
		testNo++;
		moveTest = moveTests.at(i);
        C_BOARD *pBoard = new C_BOARD();
		if (pBoard->ParseFen(moveTest.fen.c_str()) < 0)
			return false; // ParseFen had error
		CLOCK_START;
		U64 theMoveCount = PerftTest(moveTest.depth, pBoard);
		CLOCK_STOP;
		totalTime  += THECLOCK;
		totalMoves += theMoveCount;
		if (totalMoves > maxMoveCount)
			break;
		if (THECLOCK <= 0) THECLOCK = 1;
		U64 movespersec = theMoveCount*1000 / THECLOCK;
		cout << "Test#:"       << setw(4) << testNo 
			 << " Line#:"      << setw(4) << moveTest.lineNo 
			 << " Depth:"      << setw(2) << (int)moveTest.depth 
			 << " ET:"         << setw(6) << THECLOCK 
			 << " Move Count:" << setw(9) << theMoveCount 
			 << " Moves/Sec:"  << setw(9) << movespersec
			 << " Fen:"        << moveTest.fen << endl;

		if (theMoveCount != moveTest.moveCount)
		{
			pBoard->PrintBoard();
			cout << "FAILED [expected " << moveTest.moveCount << " generated " << theMoveCount << endl;
			if (haltOnError) getchar();
            return false; // error found
		}
	}
	#pragma endregion

	cout << "===================================================================" << endl;
	cout << "Total Moves       " << setw(10) << totalMoves << endl;
	cout << "Total ET(secs)    " << setw(10) << totalTime / 1000 << endl;
	U64 tim = totalTime / 1000;
	if (tim < 1)tim = 1;
	cout << "Overall Moves/sec " << setw(10) << totalMoves/ tim << endl;
	cout << "Perft Move Tests Completed" << endl << endl;
    return true;
}
bool PerftTestMate     (DEPTH maxMateInMoves,bool clearHash)
{
	cout << "Performing Perft Mate Tests" << endl;

    ifstream fin;
	string   line;
	int      errors_nomate    = 0;
    int      errors_wrongmate = 0;
    int      lineNo           = 0;
	U64      totTime          = 0;
    int      maxDepth         = 99;
	U64      mateMoveCounts[MAX_PLY_DEPTH+1];
	U64      mateMoveTimes [MAX_PLY_DEPTH+1];
	string   fen;
	string   comment;
	string   sExpectedBestmove;
	string   sFoundBestmove;
	int      expected_matein_moves = 0;
	
    C_BOARD       *pBoard = new C_BOARD();
    SEARCHINFO  *pInfo  = new SEARCHINFO();

	for (int n = 0; n <= MAX_PLY_DEPTH; n++)
	{
		mateMoveCounts[n] = 0;
		mateMoveTimes [n] = 0;
	}
	fin.open(MATETESTSFILENAME);

    if ( clearHash )
	   HASH.ClearHashTable();

    while (getline(fin,line))
	{
		lineNo++;
		fen             = "";
		expected_matein_moves    = 0;
	
		#pragma region Read and Parse line
		if ( line.compare("STOP") == 0 )
		{ 
		   cout << "'STOP' read at line " << lineNo << " will not process further entries." << endl;
		   break;
		}
		
		if ( (line.length() >= 2) && (line.substr(0,2) == "//") )
		{ 
		   // comment line or title
		   if ( (line.length() >= 4) && (line.substr(0,3) == "//+") )
		   { // Title line
			   string s = "========================";
		       cout << s << "    " << Trim(line.substr(3)) << endl;
		   }
		   continue; 
		}
		
		#pragma region strip comment if present
		comment = "";
		int n = (int)line.find("//");
		if (n > 0) 
		{
		   comment = line.substr(n+2);
		   line    = line.substr(0,n);
		}
        #pragma endregion

		// Parse line ( <fen>; <max search depth>;  <mate in 'n'>  [; bestmove] )
		n = (int)line.find(';');
		if ( n == string::npos )
			PUNT("Error: Missing <Mate in 'n' in test file");
		
		fen                    = line.substr(0,n);
		line                   = line.substr(n + 1);

		n = (int)line.find(';');
		if (n == string::npos)
		{
			sExpectedBestmove = "";
			expected_matein_moves = stoi(line);
		}		
		else
		{ // handle optional [; bestmove]
			sExpectedBestmove     = Trim(line.substr(n + 1));
			expected_matein_moves = stoi(line.substr(0, n - 1));
		}

		maxDepth               = expected_matein_moves*2 + 3; 
		#pragma endregion
		
		if (expected_matein_moves > maxMateInMoves)
		   continue; // this position too deep for request (skip it)

		#pragma region pre-move setup
		cout << endl;
		cout << ((string)" ").append(80,'=') << endl;
		cout << "Test for Mate in: " << setw(2) << expected_matein_moves << " Full Moves;  Max Search Depth:" << setw(2) << maxDepth << " PLY Moves" << endl;
		cout << "FEN:" << fen << " [Line# " << lineNo << "]" << endl;
		if ( comment.length() > 0 ) 
			cout << "Comment:" <<  comment << endl;
		cout << ((string)" ").append(80,'=') << endl;           
		if (pBoard->ParseFen(fen.c_str()) < 0)
			continue; // ParseFen had an error
		
		pInfo->starttime       = GetTimeMs();
		pInfo->depth           = maxDepth;
		pInfo->timeset         = false;
		pInfo->quit            = false;
        pInfo->matein_plymoves = -1;
        pInfo->GAME_MODE       = CONSOLEMODE;

		pInfo->quit = false;
		ASSERT(pBoard->bb64_wKings.BitCount() == 1);
		ASSERT(pBoard->CheckBoard());


		pBoard->PrintBoard("Before Move");
		cout << ((string)" ").append(80,'=') << endl;
		#pragma endregion

		Search_Position(pBoard, pInfo);
		totTime += THECLOCK;

		#pragma region post-move processing
		mateMoveTimes [expected_matein_moves] += THECLOCK;
		mateMoveCounts[expected_matein_moves]++;
		sFoundBestmove = PrMove(pInfo->bestMove);
		if (sExpectedBestmove.length()>0 && sExpectedBestmove.compare(sFoundBestmove) != 0)
		{
			Beep(523, 500);
			cout << "info " + ((string)"<").append(10, '<') + ((string)" NO MATE or BEST MATE not found ").append(80, '>') << endl;
			cout << "Bestmove Found: " << sFoundBestmove << "   Bestmove Expected:" << sExpectedBestmove << endl;
			errors_nomate++;
			if (haltOnError) getchar();
		}
		else
		if (pInfo->matein_plymoves < 0)
		{ 
		   Beep(523,500);
           cout << "info "+((string)"<").append(10,'<') + ((string)" no mate found ").append(80,'>') << endl;
		   errors_nomate++;
		   if (haltOnError) getchar();
		}
		else
		{
           if ( expected_matein_moves == 63 ) 
			   cout << "ENDGAME Mate in " << pInfo->matein_plymoves << " PLY Moves" << endl;
           else
           {
		      cout << "Mate in " << pInfo->matein_plymoves << " PLY Moves"<< endl;
		      if ( WRONG_PLY_MOVES(pInfo->matein_plymoves,expected_matein_moves ) )
		      { 
				  cout << "info " + ((string)"<").append(10, '<') + ((string)" wrong mate found ").append(80, '>') << endl;
				  cout << "Mate in plymoves:" << pInfo->matein_plymoves << "  expected:" << expected_matein_moves * 2 << endl;
				  errors_wrongmate++;
				  if (haltOnError) getchar();
		      }
           }
		}
        #pragma endregion
    }

	cout << endl << endl << "=============================== SUMMARY ========================================" << endl << endl;
    PrintStats(mateMoveCounts, mateMoveTimes, maxMateInMoves);
    cout << endl << "Total time for mate tests: " << totTime / 1000 << " seconds" << endl << endl;
	if (errors_wrongmate + errors_nomate == 0)
	{
		cout << "info <<<<<<<<< ALL MATES FOUND Correctly >>>>>>>>>>" << endl;
		return true; // no errors found
	}
	
    // SOME ERRORS FOUND
    Beep(523,500);
    if (errors_nomate != 0)
       cout << "info <<<<<<<<< " << errors_nomate    << " MATE Errors found >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    if (errors_wrongmate != 0)
       cout << "info <<<<<<<<< " << errors_wrongmate << " WRONG MATE Errors found >>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
 
	cout << "Perft Mate Tests Completed" << endl << endl;
	
	return false; // errors found

}
bool PerftTestBestMove (string parm         ,bool clearHash)
{
	cout << "Performing Perft Best Move Tests" << endl;

	bool                   onlyBKTests;
	bool                   onlyPOSTests;
    bool                   getUnSolved; 
    bool                   getSolved;
	vector<S_BESTMOVETEST> bmTests;

    ifstream fin;
	string   line             = " ";
	int      errs             = 0;
    int      lineNo           = 0;
	int      testNo           = 0;
	
    int      maxDepth         = 99;
	string   fen;
	string   comment;
	int      matein_moves     = 0;

	// default solved only
	getSolved    = true;
	getUnSolved  = false;
	onlyBKTests  = parm.find("bktest" ) != string::npos;
	onlyPOSTests = parm.find("postest") != string::npos;
	
	if (parm.length() > 0)
	{
		if (parm.find("all") != string::npos)
		{
			getSolved   = true;
			getUnSolved = true;
		}
		else
		if (parm.find("unsolved")  != string::npos)
		{
			getSolved   = false;
			getUnSolved = true;
		}
	}

	if (getSolved   ) cout << "Get Solved    " << endl;
	if (getUnSolved ) cout << "Get UnSolved  " << endl;
	if (onlyBKTests ) cout << "BK Tests Only " << endl;
	if (onlyPOSTests) cout << "POS Tests Only" << endl;

	fin.open(BMTESTSFILENAME);
 
    C_BOARD       *pBoard = new C_BOARD();
    SEARCHINFO  *pInfo  = new SEARCHINFO();

    if ( clearHash )
	    HASH.ClearHashTable();

    while ( getline(fin,line) )
    {
		lineNo++;

		string s;

		#pragma region Read and Parse line

		if ( Trim(line).length() == 0 )
			continue; // empty line

		if ( (line.length() >= 4) && (line.substr(0,4) == "STOP") )  
		{
			cout << "'STOP' read at line " << lineNo << " will not process further entries." << endl << endl;
			break;
		} 
            
		if ((line.length() >= 2) && (line.substr(0, 2) == "//"))
		{ // comment line or title
			if ((line.length() >= 3) && (line.substr(0, 3) == "//+"))
			{ // Title line -> print it
				cout << endl << endl;
				string s = ((string)" ").append(80, '=');
				cout << s << endl;
				cout << s.c_str() << "   " << line.substr(3);
				cout << s << endl;
				cout << endl << endl;
			}
			continue;
		}

		S_BESTMOVETEST bmtest;
		bmtest.lineNo = lineNo;

		#pragma region strip trailing comment if present
		int n = (int)line.find("//");
		if ( n != string::npos)
		{
			bmtest.comment = line.substr(n + 2);
			line = line.substr(0, n);
		}
		else
		{
			bmtest.comment = " ";
		}
		#pragma endregion

		#pragma region parse fen
		n = (int)line.find(';');
		if ( n == string::npos )
			throw std::exception("Missing '<fen>;' in test file");
		bmtest.fen = Trim(line.substr(0, n));
		s = line.substr(n + 1);
		#pragma endregion

		#pragma region maxDepth
		n = (int)s.find(';');
		if (n <= 0)
			throw std::exception("Missing 'maxDepth;' in test file");
		bmtest.maxDepth = stol(s.substr(0, n));
		s = s.substr(n + 1);
		#pragma endregion

		#pragma region parse bestmove
		n = (int)s.find(';');
		if (n <= 0)
			throw std::exception("Missing 'bestmove;' in test file");
		bmtest.sBestMove = Trim(s.substr(0, n));
		s = s.substr(n + 1);
		#pragma endregion

		#pragma region parse solveTime
		n = (int)s.find(';');
		if ( n == string::npos )
			throw std::exception("Missing 'solveTim;' in test file");
		bmtest.oldSolvedTim = stol(s.substr(0, n));
		s = line.substr(n + 1);
		#pragma endregion
		#pragma endregion

		bool Solved = bmtest.comment.find("NOT SOLVED") == string::npos;
		if ( (!Solved) && (!getUnSolved) )
			continue;
		if ( Solved && (!getSolved) )
			continue;
	    if ( onlyBKTests && bmtest.comment.find("BK.") == string::npos )
			continue;
	    if ( onlyPOSTests && bmtest.comment.find("POS-") == string::npos )
			continue;
		#pragma region test setup

		string sEquals = ((string)"=").append(80,'=');
		cout << endl;
		cout << sEquals << endl;
		cout << "Test for Best Move [" << bmtest.sBestMove << "];  Max Search Depth:" << setw(2) << bmtest.maxDepth << endl;
		cout << "FEN:" << bmtest.fen << " [Line# " << bmtest.lineNo << "; Test# " << testNo << "]" << endl;
		if (bmtest.comment.length() > 0) 
			cout << "Comment:" << bmtest.comment << endl;
		cout << sEquals << endl;
		if (pBoard->ParseFen(bmtest.fen.c_str()) < 0)
			continue; // ParseFen had an error
		testNo++;
		pBoard->PrintBoard();
		cout << sEquals << endl;  

		#pragma endregion

		pBoard->ParseFen(bmtest.fen.c_str());
		pInfo->depth     = bmtest.maxDepth;
		pInfo->starttime = GetTimeMs();
		pInfo->timeset   = false;
		pInfo->quit      = false;
		pInfo->bestMove  = 0;
		pInfo->bestScore = 0;
		pInfo->matein_plymoves = -1;
		pInfo->GAME_MODE = CONSOLEMODE;		
		
		if (clearHash)
			HASH.ClearHashTable();
		ASSERT(pBoard->CheckBoard());

		Search_Position(pBoard, pInfo);

		#pragma region test cleanup

		bmtest.sFoundBestMove = PrMove(pInfo->bestMove);
		bmtest.newSolvedTim   = THECLOCK;
		bmTests.push_back(bmtest);

		cout << "LineNo#" << setw(3) << lineNo << "  ET=" << setw(7) << bmtest.newSolvedTim << " Milli-seconds (Old Time=" << bmtest.oldSolvedTim << ")" << endl;
		bmtest.good = bmtest.sBestMove.compare(bmtest.sFoundBestMove) == 0;
		if ( ! bmtest.good )
		{
			errs++;
			Beep(523,500);
			cout << "info <<<<<<<<<<<<<<<<<<<<<<<<<<<< best move NOT found >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
		}
		else
		{
			cout << "info Bestmove " << bmtest.sBestMove << " found correctly" << endl;
		}
		#pragma endregion
    }

    #pragma region Print Summary

    cout << "================================ SUMMARY ========================================" << endl;
    U64 newTotTime     =  0;
    U64 oldTotTime     =  0;
	U64 POS_CountTotal =  0;
	U64 POS_GoodTotal  =  0;
	U64 BK_CountTotal  = 24;
	U64 BK_GoodTotal   =  0;

    for(size_t n=0; n<bmTests.size();n++)
    {  
		S_BESTMOVETEST bmt = bmTests[n];
		string sGoodBad = (bmt.sBestMove.compare(bmt.sFoundBestMove) == 0) ? "  GOOD " : ">>BAD<<";
		S64    diffTim = bmt.newSolvedTim - bmt.oldSolvedTim;
		newTotTime += bmt.newSolvedTim;
		oldTotTime += bmt.oldSolvedTim;
		if ( bmt.comment.find("BK.") != string::npos )
		{   // BL.xx test
			if ( bmt.good )
				BK_GoodTotal++;
		}
		if ( bmt.comment.find("POS-")  != string::npos )
		{   // POS-xxx test
			POS_CountTotal++;
			if ( bmt.good )
				POS_GoodTotal++;
		}

		cout << "Test#"     << setw(3) << n
			<< " Line#"     << setw(3) << bmt.lineNo         << " " << sGoodBad
			<< " bm:"       << setw(5) << bmt.sFoundBestMove << " (" << setw(4) << bmt.sBestMove << ")"
			<< " Old Time:" << setw(7) << bmt.oldSolvedTim
			<< " New Time:" << setw(7) << bmt.newSolvedTim
			<< " Diff:"     << setw(7) << diffTim            << " " << bmt.comment << endl;
	}

    cout << "===============================================================================================================================" << endl;
	cout << "Total Times:  new:" << FormatWithCommas(newTotTime) << " old:" << FormatWithCommas(oldTotTime) << ", diff:" << FormatWithCommas((S64)oldTotTime - (S64)newTotTime) << " ms" << endl;

	if (errs > 0)
		cout << "Total of " << errs << " errors (no solution) found (" << (testNo - errs) << "/" << testNo << " correct)" << endl; 
    else
		cout << "ALL Solutions Found - For selected tests" << endl;

	cout << "BK  Tests " << BK_GoodTotal  << " of " << BK_CountTotal  << " found." << endl;   
	cout << "POS Tests " << POS_GoodTotal << " of " << POS_CountTotal << " found." << endl;   
    #pragma endregion

    return (errs==0); 

}
bool PerftTestUtil     (                         )
{
	#pragma region test all "one bit set" ulong words
	U64 n = 1;
	for (int i = 1; i < 64; i++)
	{
		tbu(n, 64 - i, i - 1, 1);
		n *= 2L;
	}
	#pragma endregion

	#pragma region test specific   L0, T0, ONES
    cout << "Testing BitLeading0(), BitTrailing0(), BitCount() functions" << endl;
	tbu(0x0000000000000000, 64, 64, 0);
	tbu(0x0000000000000003, 62, 0, 2);
	tbu(0x0000000000000005, 61, 0, 2);
	tbu(0xF000000000000000, 0, 60, 4);
	tbu(0x0000000100000000, 31, 32, 1);
	tbu(0x0000001010000000, 27, 28, 2);
	tbu(0x0000010001000000, 23, 24, 2);
	tbu(0x0000100000100000, 19, 20, 2);
	tbu(0x0001000000010000, 15, 16, 2);
	tbu(0x0010000000001000, 11, 12, 2);
	tbu(0x0100000000000100, 7, 8, 2);
	tbu(0x1000000000000010, 3, 4, 2);
	tbu(0x0001010101010100, 15, 8, 6);
	tbu(0x0002020202020200, 14, 9, 6);
	tbu(0x0003030303030300, 14, 8, 12);
	tbu(0x0004040404040400, 13, 10, 6);
	tbu(0x0005050505050500, 13, 8, 12);
	tbu(0x0006060606060600, 13, 9, 12);
	tbu(0x0007070707070700, 13, 8, 18);
	tbu(0x0008080808080800, 12, 11, 6);
	tbu(0x0009090909090900, 12, 8, 12);
	tbu(0x000A0A0A0A0A0A00, 12, 9, 12);
	tbu(0x000B0B0B0B0B0B00, 12, 8, 18);
	tbu(0x000C0C0C0C0C0C00, 12, 10, 12);
	tbu(0x000D0D0D0D0D0D00, 12, 8, 18);
	tbu(0x000E0E0E0E0E0E00, 12, 9, 18);
	tbu(0x8000000FF00000F0, 0, 4, 13);
	tbu(0x0800000FF0000F00, 4, 8, 13);
	tbu(0x0080000FF000F000, 8, 12, 13);
	tbu(0x0008000FF00F0000, 12, 16, 13);
	tbu(0x0000800FF0F00000, 16, 20, 13);
	tbu(0x0000080FFF000000, 20, 24, 13);
	tbu(0x0000008FF0000000, 24, 28, 9);
	tbu(0x0000001FF0000000, 27, 28, 9);
	tbu(0x000000FFF8000000, 24, 27, 13);
	tbu(0x00000F0FF0800000, 20, 23, 13);
	tbu(0x0000F00FF0080000, 16, 19, 13);
	tbu(0x000F000FF0008000, 12, 15, 13);
	tbu(0x00F0000FF0000800, 8, 11, 13);
	tbu(0x0F00000FF0000080, 4, 7, 13);
	tbu(0xF000000FF0000008, 0, 3, 13);

	tbu(0x0000000FF00000F0, 28, 4, 12);
	tbu(0x0000000FF0000F00, 28, 8, 12);
	tbu(0x0000000FF000F000, 28, 12, 12);
	tbu(0x0000000FF00F0000, 28, 16, 12);
	tbu(0x0000000FF0F00000, 28, 20, 12);
	tbu(0x0000000FFF000000, 28, 24, 12);
	tbu(0x0000000FF0000000, 28, 28, 8);
	tbu(0x0000000FF0000000, 28, 28, 8);
	tbu(0x000000FFF0000000, 24, 28, 12);
	tbu(0x00000F0FF0000000, 20, 28, 12);
	tbu(0x0000F00FF0000000, 16, 28, 12);
	tbu(0x000F000FF0000000, 12, 28, 12);
	tbu(0x00F0000FF0000000, 8, 28, 12);
	tbu(0x0F00000FF0000000, 4, 28, 12);
	tbu(0xF000000FF0000000, 0, 28, 12);

	tbu(0x00000000000000F0, 56, 4, 4);
	tbu(0x0000000000000F00, 52, 8, 4);
	tbu(0x000000000000F000, 48, 12, 4);
	tbu(0x00000000000F0000, 44, 16, 4);
	tbu(0x0000000000F00000, 40, 20, 4);
	tbu(0x000000000F000000, 36, 24, 4);
	tbu(0x00000000F0000000, 32, 28, 4);
	tbu(0x0000000F00000000, 28, 32, 4);
	tbu(0x000000F000000000, 24, 36, 4);
	tbu(0x00000F0000000000, 20, 40, 4);
	tbu(0x0000F00000000000, 16, 44, 4);
	tbu(0x000F000000000000, 12, 48, 4);
	tbu(0x00F0000000000000, 8, 52, 4);
	tbu(0x0F00000000000000, 4, 56, 4);
	tbu(0xF000000000000000, 0, 60, 4);

	tbu(0x000000000000000F, 60, 0, 4);
	tbu(0x00000000000000FF, 56, 0, 8);
	tbu(0x0000000000000FFF, 52, 0, 12);
	tbu(0x000000000000FFFF, 48, 0, 16);
	tbu(0x00000000000FFFFF, 44, 0, 20);
	tbu(0x0000000000FFFFFF, 40, 0, 24);
	tbu(0x000000000FFFFFFF, 36, 0, 28);
	tbu(0x00000000FFFFFFFF, 32, 0, 32);
	tbu(0x0000000FFFFFFFFF, 28, 0, 36);
	tbu(0x000000FFFFFFFFFF, 24, 0, 40);
	tbu(0x00000FFFFFFFFFFF, 20, 0, 44);
	tbu(0x0000FFFFFFFFFFFF, 16, 0, 48);
	tbu(0x000FFFFFFFFFFFFF, 12, 0, 52);
	tbu(0x00FFFFFFFFFFFFFF, 8, 0, 56);
	tbu(0x0FFFFFFFFFFFFFFF, 4, 0, 60);
	tbu(0xFFFFFFFFFFFFFFFF, 0, 0, 64);
	  
	  
	tbu(0x0FFFFFFFFFFFFFF0, 4, 4, 56);
	tbu(0x1FFFFFFFFFFFFFF1, 3, 0, 58);
	tbu(0x2FFFFFFFFFFFFFF2, 2, 1, 58);
	tbu(0x4FFFFFFFFFFFFFF4, 1, 2, 58);
	tbu(0x8FFFFFFFFFFFFFF8, 0, 3, 58);
	  
	tbu(0x00FFFFFFFFFFFF00, 8, 8, 48);
	tbu(0x01FFFFFFFFFFFF10, 7, 4, 50);
	tbu(0x02FFFFFFFFFFFF20, 6, 5, 50);
	tbu(0x04FFFFFFFFFFFF40, 5, 6, 50);
	tbu(0x08FFFFFFFFFFFF80, 4, 7, 50);
	  
	tbu(0x000FFFFFFFFFF000, 12, 12, 40);
	tbu(0x001FFFFFFFFFF100, 11, 8, 42);
	tbu(0x002FFFFFFFFFF200, 10, 9, 42);
	tbu(0x004FFFFFFFFFF400, 9, 10, 42);
	tbu(0x008FFFFFFFFFF800, 8, 11, 42);
	  
	tbu(0x0000FFFFFFFF0000, 16, 16, 32);
	tbu(0x0001FFFFFFFF1000, 15, 12, 34);
	tbu(0x0002FFFFFFFF2000, 14, 13, 34);
	tbu(0x0004FFFFFFFF4000, 13, 14, 34);
	tbu(0x0008FFFFFFFF8000, 12, 15, 34);
	  
	tbu(0x00000FFFFFF00000, 20, 20, 24);
	tbu(0x00001FFFFFF10000, 19, 16, 26);
	tbu(0x00002FFFFFF20000, 18, 17, 26);
	tbu(0x00004FFFFFF40000, 17, 18, 26);
	tbu(0x00008FFFFFF80000, 16, 19, 26);
	  
	tbu(0x000000FFFF000000, 24, 24, 16);
	tbu(0x000001FFFF100000, 23, 20, 18);
	tbu(0x000002FFFF200000, 22, 21, 18);
	tbu(0x000004FFFF400000, 21, 22, 18);
	tbu(0x000008FFFF800000, 20, 23, 18);
	  
	tbu(0x0000000FF0000000, 28, 28, 8);
	tbu(0x0000001FF1000000, 27, 24, 10);
	tbu(0x0000002FF2000000, 26, 25, 10);
	tbu(0x0000004FF4000000, 25, 26, 10);
	tbu(0x0000008FF8000000, 24, 27, 10);
	  
	tbu(0x0000000000000000, 64, 64, 0);
	tbu(0x0000000110000000, 31, 28, 2);
	tbu(0x0000000220000000, 30, 29, 2);
	tbu(0x0000000440000000, 29, 30, 2);
	tbu(0x0000000880000000, 28, 31, 2);
	#pragma endregion

	#pragma region test BitReverse()
    cout << "Testing BitReverse() Function" << endl;
	testReverse(0x0000000000000000, 0x0000000000000000);
	testReverse(0xf000000000000000, 0x000000000000000f);
	testReverse(0x0f00000000000000, 0x00000000000000f0);
	testReverse(0x00f0000000000000, 0x0000000000000f00);
	testReverse(0x000f000000000000, 0x000000000000f000);
	testReverse(0x0000f00000000000, 0x00000000000f0000);
	testReverse(0x00000f0000000000, 0x0000000000f00000);
	testReverse(0x000000f000000000, 0x000000000f000000);
	testReverse(0x0000000f00000000, 0x00000000f0000000);
	testReverse(0x00000000f0000000, 0x0000000f00000000);
	testReverse(0x000000000f000000, 0x000000f000000000);
	testReverse(0x0000000000f00000, 0x00000f0000000000);
	testReverse(0x00000000000f0000, 0x0000f00000000000);
	testReverse(0x000000000000f000, 0x000f000000000000);
	testReverse(0x0000000000000f00, 0x00f0000000000000);
	testReverse(0x00000000000000f0, 0x0f00000000000000);
	testReverse(0x000000000000000f, 0xf000000000000000);
	testReverse(0x8888888888888888, 0x1111111111111111);
	testReverse(0x0fffffffffffffff, 0xfffffffffffffff0);
	testReverse(0xf0ffffffffffffff, 0xffffffffffffff0f);
	testReverse(0xff0fffffffffffff, 0xfffffffffffff0ff);
	testReverse(0xfff0ffffffffffff, 0xffffffffffff0fff);
	testReverse(0xffffffffffffffff, 0xffffffffffffffff);
	#pragma endregion

	#pragma region test popbit
    cout << "Testing BitPop() Function" << endl;
	testPopBit(0, 0x0000000000000001);
	testPopBit(1, 0x0000000000000002);
	testPopBit(2, 0x0000000000000004);
	testPopBit(3, 0x0000000000000008);
	testPopBit(4, 0x0000000000000010);
	testPopBit(5, 0x0000000000000020);
	testPopBit(6, 0x0000000000000040);
	testPopBit(7, 0x0000000000000080);
	#pragma endregion

	#pragma region test HandVMoves Function
    cout << "Testing HandVMoves() Function" << endl;
	testHVMoves(0x0000000000000001, 0, 0x01010101010101fe);
	testHVMoves(0xffffffffffffff01, 0, 0x00000000000001fe);
	testHVMoves(0x0101010101010101, 0, 0x00000000000001fe);
	#pragma endregion

	#pragma region test DAndAntiDMoves Function
    cout << "Testing DAndAntiDMoves() Function" << endl;
	// TODO - routines dont work yet
	//testDiagMoves(0x0000000000000000, 2, 0x01010101010101fe);
	//testDiagMoves(0xffffffffffffff01, 0, 0x00000000000001fe);
	//testDiagMoves(0x0101010101010101, 0, 0x00000000000001fe);
	#pragma endregion

    cout << "Utility tests completed" << endl << endl;
	return true;
}
bool PerftTestMagicSQ()
{

#pragma region test magic square routines
	#pragma region print blockermasks
	C_BOARD b;

    #pragma endregion
	
	cout << endl << "Rook Magic Numbers" << endl;
	for (int sq64 = 0; sq64 < 64; sq64++) 
		cout << "sq=" << setw(2) << sq64 << "  Magic=" << setw(4) << MagicIndexs_Rook[sq64] << endl;

	cout << endl << "Bishop Magic Numbers" << endl;
	for (int sq64 = 0; sq64 < 64; sq64++)
		cout << "sq=" << setw(2) << sq64 << "  Magic=" << setw(4) << MagicIndexs_Bishop[sq64] << endl;

	return true;
#pragma endregion

}
bool PerftTestSearch   (                         )
{
    C_BOARD       *pBoard = new C_BOARD();
    SEARCHINFO  *pInfo  = new SEARCHINFO();
	HASH.ClearHashTable();
	
	pBoard->ParseFen("8/3KP1k1/8/8/8/8/8/8 w - - 0 1 ");
	
	#ifdef DEBUG
	pInfo->depth     = 5;
	#else
	pInfo->depth     = 21;
	#endif
	
	pInfo->starttime       = GetTimeMs();
	pInfo->timeset         = false;
	pInfo->quit            = false;
	pInfo->bestMove        = 0;
	pInfo->bestScore       = 0;
    pInfo->matein_plymoves =-1;
	pInfo->GAME_MODE       = CONSOLEMODE;

	EngineOptions->UseBook = false;
	Search_Position(pBoard, pInfo);
	EngineOptions->UseBook = true;

	pBoard->MakeMove(pInfo->bestMove);

	cout << endl;
	cout << "info Bestmove  " << PrMove(pInfo->bestMove) << endl;
	cout << "info Bestscore " << pInfo->bestScore        << endl;
	cout << "info at depth  " << pInfo->depth            << endl;

	return true;
}
