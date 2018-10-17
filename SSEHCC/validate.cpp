// validate.c
#include "stdafx.h"
#include "C_BOARD.h"
#include "CS_EVALUATE.h"
 
#pragma warning( disable: 4996)

int SqIs120            (int sq120) { return (sq120>=0 && sq120<120);                   }
int PceValidEmptyOffbrd(int pce  ) { return (PieceValidEmpty(pce) || pce == OFFBOARD); }
int SqOnBoard          (int sq120) { return (SQOFFBOARD(sq120) ? 0 : 1);               }
int SideValid          (int side ) { return (side == WHITE || side == BLACK) ? 1 : 0;  }
int FileRankValid      (int fr   ) { return (fr >= 0 && fr <= 7) ? 1 : 0;              }
int PieceValidEmpty    (int pce  ) { return (pce >= EMPTY && pce <= bK) ? 1 : 0;       }
int PieceValid         (int pce  ) { return (pce >= wP && pce <= bK) ? 1 : 0;          }

void PerftMirrorTest()
{
	int      ev1       = 0; 
	int      ev2       = 0;
	int      positions = 0; 
	int      lineNo    = 0;
	string   line;
	string   fen;
	ifstream fin(EVALTESTSFILENAME, ios::in);
	C_BOARD *pBoard =  new C_BOARD();

	while (getline(fin, line))
	{ // process line
		lineNo++;
		line = Trim(line);
		cout << lineNo << ":" << line << endl;
		if (line.length() >= 2 && line.substr(0, 2) == "//")
			continue;

		size_t sz;
		sz = line.find(';');  
		fen = (sz>0) ? line.substr(0, sz) : line;     
		if (pBoard->ParseFen(line) < 0)
			continue; // ParseFen had an error
		positions++;
		ev1 = EVAL.EvalPosition(pBoard);
		pBoard->MirrorBoard();
		ev2 = EVAL.EvalPosition(pBoard);

		if (ev1 != ev2) 
		{
			cout << endl << endl << "Mirror Fail: EV1=" << ev1 << "  EV2=" << ev2 << endl;
			pBoard->ParseFen(fen);
			pBoard->PrintBoard();
			pBoard->MirrorBoard();
			pBoard->PrintBoard();
			cout << endl << endl << "Mirror Fail: EV1=" << ev1 << "  EV2=" << ev2 << endl;
			if (haltOnError) getchar();
			//return;
		}

		if ((positions % 1000) == 0) 
			cout << "position " << positions << endl;

	}
	
}

