#pragma once
#include "stdafx.h"
#include "C_BOARD.h"
#include "CS_HASHTABLE.h"
#include "C_MOVEGEN.h"
#include <thread>

#define ASSERTBOARD(n) { if (!(n)) { PrintBoard(); ASSERT(n); } }

	C_BOARD::C_BOARD              ()
	{ 	
	}
	C_BOARD::~C_BOARD             ()
	{
	}

#pragma region PUBLIC FUNCTIONS

    int          C_BOARD::KINGSQ64             ()
    { 
         return (side == WHITE) ? bb64_wKings.BitGet() : bb64_bKings.BitGet();
    }
    int          C_BOARD::KINGSQ120            ()
    {
         return SQ120(KINGSQ64());
    }
    int          C_BOARD::OTHERKINGSQ64        ()
    {
         return (side != WHITE) ? bb64_wKings.BitGet() : bb64_bKings.BitGet();
    }
    int          C_BOARD::OTHERKINGSQ120       ()
    {
         return SQ120(OTHERKINGSQ64());
    }

	bool         C_BOARD::MakeMove             (const int move                                  ) 
	{

		ASSERT(CheckBoard());

		int from = MV_FROMSQ(move);
		int to   = MV_TOSQ(move);

		ASSERT(SqOnBoard(from));
		ASSERT(SqOnBoard(to));
		ASSERT(SideValid(side));

	    #ifdef DEBUG
		if (!PieceValid(pieces[from]))
		{   
			PrintBoard();
			cout << "Current MakeMove in progress: " << GetMoveDetail(move)<< endl << endl;
			ASSERT2(PieceValid(pieces[from]), "PIECE INVALID");
		}
	    #endif
		ASSERT(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		ASSERT(ply    >= 0 && ply    < MAX_PLY_DEPTH);

		history[hisPly].posKey = posKey;

		if ( MV_ENPASSANT(move) ) 
		{
			if (side == WHITE) 
				ClearPiece(to - 10);
			else 
				ClearPiece(to + 10);
		}
		else 
		if ( MV_CASTLE(move) ) 
		{
			switch (to) 
			{
			case C1:  MovePiece(A1, D1); break;
			case C8:  MovePiece(A8, D8); break;
			case G1:  MovePiece(H1, F1); break;
			case G8:  MovePiece(H8, F8); break;
			default:  ASSERT(false);     break;
			}
		}

		if (SQenPas120 != NO_EP_SQ) HASH_EP(this,SQenPas120);
		HASH_CA(this);

		history[hisPly].move       = move;
		history[hisPly].fiftyMove  = fiftyMove;
		history[hisPly].SQenPas120    = SQenPas120;
		history[hisPly].castlePerm = castlePerm;

		castlePerm &= CastlePerm[from];
		castlePerm &= CastlePerm[to];
		SQenPas120 = NO_EP_SQ;

		HASH_CA(this);

		int captured = MV_CAPTURED(move);
		fiftyMove++;

		if (captured != EMPTY) 
		{
			ASSERT(PieceValid(captured));
			ClearPiece(to);
			fiftyMove = 0;
		}

		hisPly++;
		ply++;

		ASSERT(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		ASSERT(ply    >= 0 && ply    < MAX_PLY_DEPTH);

		if (PiecePawn[pieces[from]]) 
		{
			fiftyMove = 0;
			if ( MV_PAWNSTART(move) ) 
			{
				if (side == WHITE) 
				{
					SQenPas120 = from + 10;
					ASSERT(RanksBrd[SQenPas120] == RANK_3);
				}
				else 
				{
					SQenPas120 = from - 10;
					ASSERT(RanksBrd[SQenPas120] == RANK_6);
				}
				HASH_EP(this,SQenPas120);
			}
		}

		MovePiece(from, to);

		if ( MV_PROMOTED(move) != EMPTY )
		{
			int prPce = MV_PROMOTED(move);
			ASSERT(PieceValid(prPce) && !PiecePawn[prPce]);
			ClearPiece(to);
			AddPiece  (to, prPce);
		}

		side ^= 1;
		HASH_SIDE(this);

		ASSERT(CheckBoard());

		if (C_MOVEGEN::mgSqAttacked(this, OTHERKINGSQ120(), side))
		{
			TakeMove();	
			ASSERT(CheckBoard());
			return false;
		}

		return true;

	}
    void         C_BOARD::TakeMove             (                                                ) 
	{
		ASSERT(CheckBoard());

		hisPly--;
		ply--;

		ASSERT(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		ASSERT(ply    >= 0 && ply    < MAX_PLY_DEPTH);

		int move = history[hisPly].move;
		int from = MV_FROMSQ(move);
		int to   = MV_TOSQ(move);

		ASSERT(SqOnBoard(from));
		ASSERT(SqOnBoard(to));

		if (SQenPas120 != NO_EP_SQ) HASH_EP(this,SQenPas120);
		HASH_CA(this);

		castlePerm = history[hisPly].castlePerm;
		fiftyMove  = history[hisPly].fiftyMove;
		SQenPas120    = history[hisPly].SQenPas120;

		if (SQenPas120 != NO_EP_SQ) HASH_EP(this,SQenPas120);
		HASH_CA(this);

		side ^= 1;
		HASH_SIDE(this);

		if (MVFLAG_EnPassant & move) 
		{
			if (side == WHITE) 
				AddPiece(to - 10, bP);
			else 
				AddPiece(to + 10, wP);
		}
		else 
		if (MVFLAG_Castle & move) 
		{
			switch (to) 
			{
				case C1: MovePiece(D1, A1); break;
				case C8: MovePiece(D8, A8); break;
				case G1: MovePiece(F1, H1); break;
				case G8: MovePiece(F8, H8); break;
				default: ASSERT(false);     break;
			}
		}

		MovePiece(to, from);

		int captured = MV_CAPTURED(move);
		if ( captured != EMPTY )  
		{
			ASSERT(PieceValid(captured));
			AddPiece(to, captured);
		}

		if ( MV_PROMOTED(move) != EMPTY ) 
		{
			ASSERT(PieceValid(MV_PROMOTED(move)) && !PiecePawn[MV_PROMOTED(move)]);
			ClearPiece(from);
			AddPiece  (from, (PieceCol[MV_PROMOTED(move)] == WHITE ? wP : bP));
		}

		ASSERT(CheckBoard());

	}
	void         C_BOARD::ClearPiece           (const int sq120                                 )
	{

		ASSERT(SqOnBoard(sq120));
		ASSERT(CheckBoard());

		int pce = this->pieces[sq120];

		ASSERT(PieceValid(pce));

		int col = PieceCol[pce];
		int index = 0;

		ASSERT(SideValid(col));


		HASH_PCE(this, pce, sq120);

		this->pieces[sq120] = EMPTY;
		this->material[col] -= PieceVal[pce];

		if (PieceBig[pce])
		{
			this->bigPce[col]--;
			if (PieceMaj[pce])
				this->majPce[col]--;
			else
				this->minPce[col]--;
		}

		int sq64 = SQ64(sq120);

		switch (pce)
		{
		case wP: this->bb64_wPawns  .ClrBit(sq64);      break;
		case wR: this->bb64_wRooks  .ClrBit(sq64);      break;
		case wN: this->bb64_wKnights.ClrBit(sq64);      break;
		case wB: this->bb64_wBishops.ClrBit(sq64);      break;
		case wQ: this->bb64_wQueens .ClrBit(sq64);      break;
		case wK: this->bb64_wKings  .ClrBit(sq64);      break;
		case bP: this->bb64_bPawns  .ClrBit(sq64);      break;
		case bR: this->bb64_bRooks  .ClrBit(sq64);      break;
		case bN: this->bb64_bKnights.ClrBit(sq64);      break;
		case bB: this->bb64_bBishops.ClrBit(sq64);      break;
		case bQ: this->bb64_bQueens .ClrBit(sq64);      break;
		case bK: this->bb64_bKings  .ClrBit(sq64);      break;
		}

	}
	void         C_BOARD::AddPiece             (const int sq120, const int pce                  )
	{

		ASSERT(PieceValid(pce));
		ASSERT(SqOnBoard(sq120));

		int col = PieceCol[pce];
		ASSERT(SideValid(col));


		HASH_PCE(this, pce, sq120);

		this->pieces[sq120] = pce;

		if (PieceBig[pce])
		{
			this->bigPce[col]++;
			if (PieceMaj[pce])
				this->majPce[col]++;
			else
				this->minPce[col]++;
		}

		int sq64 = SQ64(sq120);

		switch (pce)
		{
		case wP: this->bb64_wPawns  .SetBit(sq64);  break;
		case wR: this->bb64_wRooks  .SetBit(sq64);  break;
		case wN: this->bb64_wKnights.SetBit(sq64);  break;
		case wB: this->bb64_wBishops.SetBit(sq64);  break;
		case wQ: this->bb64_wQueens .SetBit(sq64);  break;
		case wK: this->bb64_wKings  .SetBit(sq64);  break;
		case bP: this->bb64_bPawns  .SetBit(sq64);  break;
		case bR: this->bb64_bRooks  .SetBit(sq64);  break;
		case bN: this->bb64_bKnights.SetBit(sq64);  break;
		case bB: this->bb64_bBishops.SetBit(sq64);  break;
		case bQ: this->bb64_bQueens .SetBit(sq64);  break;
		case bK: this->bb64_bKings  .SetBit(sq64);  break;
		}

		this->material[col] += PieceVal[pce];

	}
	void         C_BOARD::MovePiece            (const int fromsq120, const int tosq120          )
	{

		ASSERT(SqOnBoard(fromsq120));
		ASSERT(SqOnBoard(tosq120));

		int index = 0;
		int pce = this->pieces[fromsq120];
		int col = PieceCol[pce];
		int fromsq64 = SQ64(fromsq120);
		int tosq64 = SQ64(tosq120);

		ASSERT(SideValid(col));
		ASSERT(PieceValid(pce));


		HASH_PCE(this, pce, fromsq120);
		this->pieces[fromsq120] = EMPTY;

		HASH_PCE(this, pce, tosq120);
		this->pieces[tosq120] = pce;

		switch (pce)
		{             // clear from64 bit;  set to64 bit
		case wP: this->bb64_wPawns  .ClrBit(fromsq64);   this->bb64_wPawns  .SetBit(tosq64);  break;
		case wR: this->bb64_wRooks  .ClrBit(fromsq64);   this->bb64_wRooks  .SetBit(tosq64);  break;
		case wN: this->bb64_wKnights.ClrBit(fromsq64);   this->bb64_wKnights.SetBit(tosq64);  break;
		case wB: this->bb64_wBishops.ClrBit(fromsq64);   this->bb64_wBishops.SetBit(tosq64);  break;
		case wQ: this->bb64_wQueens .ClrBit(fromsq64);   this->bb64_wQueens .SetBit(tosq64);  break;
		case wK: this->bb64_wKings  .ClrBit(fromsq64);   this->bb64_wKings  .SetBit(tosq64);  break;
		case bP: this->bb64_bPawns  .ClrBit(fromsq64);   this->bb64_bPawns  .SetBit(tosq64);  break;
		case bR: this->bb64_bRooks  .ClrBit(fromsq64);   this->bb64_bRooks  .SetBit(tosq64);  break;
		case bN: this->bb64_bKnights.ClrBit(fromsq64);   this->bb64_bKnights.SetBit(tosq64);  break;
		case bB: this->bb64_bBishops.ClrBit(fromsq64);   this->bb64_bBishops.SetBit(tosq64);  break;
		case bQ: this->bb64_bQueens .ClrBit(fromsq64);   this->bb64_bQueens .SetBit(tosq64);  break;
		case bK: this->bb64_bKings  .ClrBit(fromsq64);   this->bb64_bKings  .SetBit(tosq64);  break;
		}

	}
	void         C_BOARD::MakeNullMove         (                                                )
	{

		ASSERT(CheckBoard());
		ASSERT( !SqAttackedKingSq() );
 
		ply++;
		history[hisPly].posKey = posKey;

		if (SQenPas120 != NO_EP_SQ) 
			HASH_EP(this,SQenPas120);

		history[hisPly].move       = NOMOVE;
		history[hisPly].fiftyMove  = fiftyMove;
		history[hisPly].SQenPas120    = SQenPas120;
		history[hisPly].castlePerm = castlePerm;
		SQenPas120 = NO_EP_SQ;

		side ^= 1;
		hisPly++;
		HASH_SIDE(this);

		ASSERT(CheckBoard());
		ASSERT(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		ASSERT(ply    >= 0 && ply < MAX_PLY_DEPTH);

		return;
	} 
    void         C_BOARD::TakeNullMove         (                                                ) 
	{
		ASSERT(CheckBoard());

		hisPly--;
		ply--;

		if (SQenPas120 != NO_EP_SQ) 
			HASH_EP(this,SQenPas120);

		castlePerm = history[hisPly].castlePerm;
		fiftyMove  = history[hisPly].fiftyMove;
		SQenPas120    = history[hisPly].SQenPas120;

		if (SQenPas120 != NO_EP_SQ) 
			HASH_EP(this,SQenPas120);
		side ^= 1;
		HASH_SIDE(this);

		ASSERT(CheckBoard());
		ASSERT(hisPly >= 0 && hisPly < MAXGAMEMOVES);
		ASSERT(ply    >= 0 && ply < MAX_PLY_DEPTH);
	}
    int          C_BOARD::SqAttackedKingSq     (                                                )
	{
		return C_MOVEGEN::mgSqAttacked(this ,KINGSQ120(), side ^ 1);
	}			
	void         C_BOARD::UpdateListsMaterial  (                                                ) 
	{
		int piece, colour;

		for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++)
		{
			piece = pieces[SQ120(sq64)];

			ASSERT(PceValidEmptyOffbrd(piece));
			if (piece != OFFBOARD && piece != EMPTY) 
			{
				colour = PieceCol[piece];
				//to do ASSERT(SideValid(colour));

				if ( PieceBig[piece] ) bigPce[colour]++;
				if ( PieceMin[piece] ) minPce[colour]++;
				if ( PieceMaj[piece] ) majPce[colour]++;

				material[colour] += PieceVal[piece];

				switch (piece)
				{
					#pragma region handle piece type

					case wP: bb64_wPawns  .SetBit(sq64);  break;
					case wN: bb64_wKnights.SetBit(sq64);  break;
					case wB: bb64_wBishops.SetBit(sq64);  break;
					case wR: bb64_wRooks  .SetBit(sq64);  break;
					case wQ: bb64_wQueens .SetBit(sq64);  break;
					case wK: bb64_wKings  .SetBit(sq64);  break;

					case bP: bb64_bPawns  .SetBit(sq64);  break;
					case bN: bb64_bKnights.SetBit(sq64);  break;
					case bB: bb64_bBishops.SetBit(sq64);  break;
					case bR: bb64_bRooks  .SetBit(sq64);  break;
					case bQ: bb64_bQueens .SetBit(sq64);  break;
					case bK: bb64_bKings  .SetBit(sq64);  break;

					default: ASSERT(false);               break;

					#pragma endregion
				}
			}
		}
	}						        
	void         C_BOARD::ResetBoard           (                                                ) 
	{
		// set whole board of OFFBOARD
		//for (int &piece : pieces )
		//	piece = OFFBOARD; 
		uninitialized_fill_n(pieces, size(pieces), OFFBOARD);

		// set 'board' to empty
		for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++) 
			pieces[SQ120(sq64)] = EMPTY;

		for (int i = 0; i < BOTH; i++)
		{
			bigPce  [i] = 0;
			majPce  [i] = 0;
			minPce  [i] = 0;
			material[i] = 0;
		}

		bb64_wPawns   = 0; bb64_wPawnProts   = 0;  bb64_wPawnAttks   = 0; 
		bb64_wRooks   = 0; bb64_wKnightProts = 0;  bb64_wKnightAttks = 0; 
		bb64_wKnights = 0; bb64_wBishopProts = 0;  bb64_wBishopAttks = 0; 
		bb64_wBishops = 0; bb64_wRookProts   = 0;  bb64_wRookAttks   = 0; 
		bb64_wQueens  = 0; bb64_wQueenProts  = 0;  bb64_wQueenAttks  = 0; 
		bb64_wKings   = 0; bb64_wKingProts   = 0;  bb64_wKingAttks   = 0; 
									   
		bb64_bPawns   = 0; bb64_bPawnProts   = 0;  bb64_bPawnAttks   = 0;
		bb64_bRooks   = 0; bb64_bKnightProts = 0;  bb64_bKnightAttks = 0;
		bb64_bKnights = 0; bb64_bBishopProts = 0;  bb64_bBishopAttks = 0;
		bb64_bBishops = 0; bb64_bRookProts   = 0;  bb64_bRookAttks   = 0;
		bb64_bQueens  = 0; bb64_bQueenProts  = 0;  bb64_bQueenAttks  = 0;
		bb64_bKings   = 0; bb64_bKingProts	 = 0;  bb64_bKingAttks   = 0;
						   
		wTotalAttks   = 0; wTotalProts = 0;
		bTotalAttks   = 0; bTotalProts = 0;

		side          = BOTH;
		SQenPas120    = NO_EP_SQ;
		fiftyMove     = 0;
		ply           = 0;
		hisPly        = 0;
		castlePerm    = 0;
		posKey        = 0;

	}
	void         C_BOARD::MirrorBoard          (                                                ) 
	{

		int tempPiecesArray[BRD_SQ_NUM64];
		int tempSide       = side ^ 1;
		int SwapPiece[13]  = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
		int tempCastlePerm = 0;
		int tempEnPas      = NO_EP_SQ;

		int tp;

		if (castlePerm & WK_SIDECASTLING) tempCastlePerm |= BK_SIDECASTLING;
		if (castlePerm & WQ_SIDECASTLING) tempCastlePerm |= BQ_SIDECASTLING;

		if (castlePerm & BK_SIDECASTLING) tempCastlePerm |= WK_SIDECASTLING;
		if (castlePerm & BQ_SIDECASTLING) tempCastlePerm |= WQ_SIDECASTLING;

		if (SQenPas120 != NO_EP_SQ) 
			tempEnPas = SQ120(Mirror64[SQ64(SQenPas120)]);

		for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++) 
			tempPiecesArray[sq64] = pieces[SQ120(Mirror64[sq64])];

		ResetBoard();

		for (int sq64 = 0; sq64 < BRD_SQ_NUM64; sq64++) 
		{
			tp = SwapPiece[tempPiecesArray[sq64]];
			pieces[SQ120(sq64)] = tp;
		}

		side       = tempSide;
		castlePerm = tempCastlePerm;
		SQenPas120 = tempEnPas;
		posKey     = GeneratePosKey();

		UpdateListsMaterial();

		CheckBoard();
	}
	U64          C_BOARD::GeneratePosKey       (                                                ) 
	{
		U64 finalKey = 0;
		int piece    = EMPTY;

		// pieces
		for (int sq120=BRD_SQ_LOWEST; sq120 <= BRD_SQ_HIGHEST; sq120++)
		{
			piece = pieces[sq120];
			if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) 
			{
				ASSERT(piece >= wP && piece <= bK);
				finalKey ^= PieceKeys[piece][sq120];
			}
		}

		if (side == WHITE) 
			finalKey ^= SideKey;

		if (SQenPas120 != NO_EP_SQ) 
		{
			ASSERT(SQenPas120 >= 0 && SQenPas120<BRD_SQ_NUM120);
			ASSERT(SqOnBoard(SQenPas120));
			ASSERT(RanksBrd[SQenPas120] == RANK_3 || RanksBrd[SQenPas120] == RANK_6);
			finalKey ^= PieceKeys[EMPTY][SQenPas120];
		}

		ASSERT(castlePerm >= 0 && castlePerm <= 15);

		finalKey ^= CastleKeys[castlePerm];

		return finalKey;
	}
    bool         C_BOARD::MoveExists           (const int move                                  )                                      
	{
		S_MOVELIST movelist[1];
		C_MOVEGEN::mgGenerateMoves(this,movelist,false);

		for (int i=0; i<movelist->size; i++) 
		{
			if (!MakeMove(movelist->moves[i].move))
				continue;

			TakeMove();

			if (movelist->moves[i].move == move)
				return true;
		}

		return false;
	}
	int          C_BOARD::GetPvLine            (C_BOARD *pBoard, const DEPTH depth                ) 
	{
		ASSERT(depth < MAX_PLY_DEPTH && depth >= 1);

		int move  = HASH.ProbePvMove(posKey);
		int count = 0;
	
		while(move != NOMOVE && count < depth) 
		{
			ASSERT(count < MAX_PLY_DEPTH);	
			if ( !MoveExists(move) )
				break;
			MakeMove(move);
			PvArray[count++] = move;
			move = HASH.ProbePvMove(posKey);	
		}
		while(ply > 0) 
			TakeMove();
	
		return count;
	
	}
	 	        
	bool         C_BOARD::CheckBoard           (                                                ) 
	{
		int   t_bigPce  [2] = { 0, 0 };
		int   t_majPce  [2] = { 0, 0 };
		int   t_minPce  [2] = { 0, 0 };
		int   t_material[2] = { 0, 0 };

		int   sq64, t_piece, sq120, colour;

		C_BB_64 t_wPawns   = C_BB_64(bb64_wPawns  );        C_BB_64 t_bPawns   = C_BB_64(bb64_bPawns  );
		C_BB_64 t_wRooks   = C_BB_64(bb64_wRooks  );        C_BB_64 t_bRooks   = C_BB_64(bb64_bRooks  );
		C_BB_64 t_wKnights = C_BB_64(bb64_wKnights);        C_BB_64 t_bKnights = C_BB_64(bb64_bKnights);
		C_BB_64 t_wBishops = C_BB_64(bb64_wBishops);        C_BB_64 t_bBishops = C_BB_64(bb64_bBishops);
		C_BB_64 t_wQueens  = C_BB_64(bb64_wQueens );        C_BB_64 t_bQueens  = C_BB_64(bb64_bQueens );
		C_BB_64 t64_wKings = C_BB_64(bb64_wKings);          C_BB_64 t_bKings   = C_BB_64(bb64_bKings  );

		// check piece count and other counters
		for (sq64 = 0; sq64 < BRD_SQ_NUM64; ++sq64) 
		{
			sq120   = SQ120(sq64);
			t_piece = pieces[sq120];
			colour  = PieceCol[t_piece];
			if ( PieceBig[t_piece] ) t_bigPce[colour]++;
			if ( PieceMin[t_piece] ) t_minPce[colour]++;
			if ( PieceMaj[t_piece] ) t_majPce[colour]++;

			t_material[colour] += PieceVal[t_piece];
		}

#pragma region  check bitboards squares
#ifndef DEBUG
#define testBBvsPiece64(bb,piece) 
#else
#define testBBvsPiece64(bb64,piece)                                         \
				while (bb64 != 0) 		 						                \
				{	sq64 =bb64.BitPop();   		     	   			            \
					int sq120 = SQ120(sq64);		 						    \
					if (pieces[sq120] != piece)                                 \
					{                                                           \
						cout << "sq120          =" << sq120 << " sq64=" << sq64 << endl;             \
						cout << "pieces[sq120]  =" << cPceChar[pieces[sq120]] << "[" << pieces[sq120] << "]" << endl;  \
						cout << "Expected pieces=" << cPceChar[piece]         << "[" << pieces[piece] << "]" << endl;  \
						ASSERT(pieces[sq120] == piece);                         \
					}                                                           \
				}		
    #endif

		testBBvsPiece64(t_wPawns  , wP);        testBBvsPiece64(t_bPawns  , bP); 
		testBBvsPiece64(t_wRooks  , wR);        testBBvsPiece64(t_bRooks  , bR); 

		testBBvsPiece64(t_wKnights, wN);        testBBvsPiece64(t_bKnights, bN); 
		testBBvsPiece64(t_wBishops, wB);        testBBvsPiece64(t_bBishops, bB); 
		testBBvsPiece64(t_wQueens , wQ);        testBBvsPiece64(t_bQueens , bQ); 
		testBBvsPiece64(t64_wKings, wK);    	testBBvsPiece64(t_bKings  , bK);
 						  
		#pragma endregion

		ASSERT(bb64_wKings.BitCount() == 1);
		ASSERT(t_material[WHITE] == material[WHITE] && t_material[BLACK] == material[BLACK]);
		ASSERT(t_minPce  [WHITE] == minPce  [WHITE] && t_minPce  [BLACK] == minPce  [BLACK]);
		ASSERT(t_majPce  [WHITE] == majPce  [WHITE] && t_majPce  [BLACK] == majPce  [BLACK]);
		ASSERT(t_bigPce  [WHITE] == bigPce  [WHITE] && t_bigPce  [BLACK] == bigPce  [BLACK]);

		ASSERT(side == WHITE || side == BLACK);
		ASSERT(GeneratePosKey() == posKey);

		if (SQenPas120 == NO_EP_SQ)
			;
		else
		if (RanksBrd[SQenPas120] == RANK_6 && side == WHITE)
			;
		else
		if (RanksBrd[SQenPas120] == RANK_3 && side == BLACK)
			;
		else
			cout << "enpas sq64=" << SQ64(SQenPas120) << " side=" << side << " rank=" << RanksBrd[SQenPas120] << endl;
		ASSERT(  (SQenPas120 == NO_EP_SQ )
			  || (RanksBrd[SQenPas120] == RANK_6 && side == WHITE)
			  || (RanksBrd[SQenPas120] == RANK_3 && side == BLACK)
			  );

		ASSERT(castlePerm >= 0 && castlePerm <= 15);

		return true;

	}
    bool         C_BOARD::MoveListOk           (const S_MOVELIST  *movelist                     ) 
			{
				int from = 0;
				int to   = 0;

				for (int i=0; i<movelist->size; i++) 
				{
					to   = MV_TOSQ  (movelist->moves[i].move);
					from = MV_FROMSQ(movelist->moves[i].move);

					if (!SqOnBoard(to) || !SqOnBoard(from)) 
						return false;

					if (!PieceValid(pieces[from])) 
					{
						PrintBoard();
						return false;
					}
				}

				return true;
			}
	int          C_BOARD::ParseFen             (string fen                                      ) {

				ASSERT(fen.size() > 0);

				int  rank = RANK_8;
				int  file = FILE_A;
				int  piece = 0;
				int  count = 0;
				int  i     = 0;
				int  n     = 0;
				int  sq64  = 0;
				int  sq120 = 0;
				char c;

				ResetBoard();
				c = fen.at(n);
	
				try
				{
					while ((rank >= RANK_1) && c) 
					{
						count = 1;
						switch (c) 
						{
							case 'p': piece = bP; break;
							case 'r': piece = bR; break;
							case 'n': piece = bN; break;
							case 'b': piece = bB; break;
							case 'k': piece = bK; break;
							case 'q': piece = bQ; break;
							case 'P': piece = wP; break;
							case 'R': piece = wR; break;
							case 'N': piece = wN; break;
							case 'B': piece = wB; break;
							case 'K': piece = wK; break;
							case 'Q': piece = wQ; break;

							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
							case '8':
								piece = EMPTY;
								count = c - '0';
								break;

							case '/':
							case ' ':
								rank--;
								file = FILE_A;
								c = fen.at(++n);
								continue;

							default:
									cout << "FEN error" << endl;
								return -1;
						}

						for (i = 0; i < count; i++) 
						{
							sq64 = rank * 8 + file;
							sq120 = SQ120(sq64);
							if (piece != EMPTY) 
							{
								pieces[sq120] = piece;
							}
							file++;
						}
						c = fen.at(++n);
					}

					ASSERT(c == 'w' || c == 'b');

					side = (c == 'w') ? WHITE : BLACK;
					n += 2;

					for (i = 0; i < 4; i++) 
					{ 
						c = fen.at(n);
						if (c == ' ') 
							break;
						switch (c) 
						{
							case 'K': castlePerm |= WK_SIDECASTLING; break;
							case 'Q': castlePerm |= WQ_SIDECASTLING; break;
							case 'k': castlePerm |= BK_SIDECASTLING; break;
							case 'q': castlePerm |= BQ_SIDECASTLING; break;
							default:                           break;
						}
						n++;
					}
	
					c = fen.at(++n);

					ASSERT(castlePerm >= 0 && castlePerm <= 15);

					if (c != '-') 
					{
						file = fen[n  ] - 'a';
						rank = fen[n+1] - '1';

						ASSERT(file >= FILE_A && file <= FILE_H);
						ASSERT(rank >= RANK_1 && rank <= RANK_8);

						SQenPas120 = FR2SQ(file, rank);
					}
				}
				catch (exception e)
				{
					cout << "ERROR: Fen scanning error" << endl;
					return -1;
				}

				posKey = GeneratePosKey();
				UpdateListsMaterial();

				return 0;
			}						        
	string       C_BOARD::GetMoveDetail        (int move                                        )
			{
				int    fromSQ      = MV_FROMSQ(move);
				int    toSQ        = MV_TOSQ(move);
				int    captured    = MV_CAPTURED(move);
				int    promoted    = MV_PROMOTED(move);
				string s = "";
				int    tosqPiece   = pieces[toSQ];
				int    fromsqPiece = pieces[fromSQ];

				s += PrMove(move);
				s += " To Piece:"   + GetPieceDetail(tosqPiece);
				s += " From Piece:" + GetPieceDetail(fromsqPiece);

				if ( MV_ENPASSANT    (move) ) { s += " En Passant"; }
				if ( MV_PAWNSTART    (move) ) { s += " Pawn Start"; }
				if ( MV_CASTLE       (move) ) { s += " Castle"; }
				if ( MV_PIECECAPTURE (move) ) { s += " Piece Capture ->" + sPceChar[captured]; }
				if ( MV_PROMOTEDPIECE(move) ) { s += " Promoted Piece->" + sPceChar[promoted]; }

	
				s += " fromsq64=" + std::to_string( SQ64(fromSQ) );
				s += " tosq64="   + std::to_string( SQ64(toSQ  ) );

				return s;
			}
	string       C_BOARD::GetPieceDetail       (int sq120                                       )
			{
				string sPiece = sPceChar[pieces[sq120]];

				if (sPiece.compare(".") == 0) 
					return "EMPTY"; 

				if (PieceCol[pieces[sq120]] == WHITE)
					return sPiece; 

				return "(" + sPiece + ")";

			}
	void         C_BOARD::PrintMoveList        (S_MOVELIST  *movelist                           )
			{
				string s;
 
				cout << "======== Move List ========" << endl;
				for (int movenum = 0; movenum < movelist->size; movenum++)
				{
					int    move = movelist->moves[movenum].move;
					cout << "move #" << setw(3) << movenum << " " << PrMove(move) << endl;
				}
				cout << "===== END Move List =======" << endl;
			}
	void         C_BOARD::PrintMoveHistory     (int n                                           )
			{
				string s;
				cout << "======== Move History ========" << endl;
				for (int i = 0; i < n; i++)
				{
					int moveNo = hisPly - i;
					if (moveNo <= 0)
						break;
					int    move = history[i].move;
					string s   = GetMoveDetail(move);
					cout << "move #" << setw(3) << moveNo << " " << s << endl; 
				}
				cout << "===== END Move History =======" << endl;
			}
	void         C_BOARD::PrintBoard           (                                                )
		{
				PrintBoard("");
		}
	void         C_BOARD::PrintBoard           (string msg                                      )
		{
				int sq, file, rank, piece;
				
				// Update current side attack and protect counts for printing for current position
				S_MOVELIST movelist[1];
				C_MOVEGEN::mgGenerateMoves(this, movelist, false);
 				cout << "======== PRINT BOARD ================ [" << msg << "]" << endl;
				cout << endl << "Game Board:" << endl;
				cout << "  ";
				for (file = FILE_A; file <= FILE_H; file++) 
					cout << " " << setw(3) << (char)('a' + file);
				cout << endl;
				cout << "   _________________________________ " << endl;
				cout << "  |                                 |" << endl;
				for (rank = RANK_8; rank >= RANK_1; rank--)
				{
					cout << setw(1) << (rank + 1) << " |";
					for (file = FILE_A; file <= FILE_H; file++) 
					{
						sq = FR2SQ(file, rank);
						piece = pieces[sq];
						cout << " ";
						cout << setw(1) << (char)(PieceCol[piece] == BLACK ? '(' : '  ');
						cout << setw(1) << sPceChar[piece];
						cout << setw(1) << (char)(PieceCol[piece] == BLACK ? ')' : ' ');
					}
					cout << " | " << setw(1) << (rank + 1) << "   ";
					switch ( rank )
					{
						case RANK_8: cout << "  " << "side:    " << cSideChar[side]  << endl;   
							         cout << "  |                                 |     "
										  <<"  " << "enPas SQ:" << SQenPas120       << endl;
							         break;
						case RANK_7: cout << "  " << "castle:  " << (char)(castlePerm & WK_SIDECASTLING ? 'K' : '-') << (char)(castlePerm & WQ_SIDECASTLING ? 'Q' : '-')
		                             				             << (char)(castlePerm & BK_SIDECASTLING ? 'k' : '-') << (char)(castlePerm & BQ_SIDECASTLING ? 'q' : '-'); 
									 cout << endl;
							         cout << "  |                                 |       ";
									 if (!EngineOptions->UseBook) cout << "NOT ";
									 cout << "Using Book";
									 cout << endl;
								     break;
						case RANK_6: cout << endl;
							         cout << "  |                                 |     ";
									 if (side == WHITE) cout << "  " << "White Attk SQs:" << setw(2) << this->bb64_wPiecesAttksSQs.BitCount() 
										                     << "   White Prots SQs : "   << setw(2) << this->bb64_wPiecesProtsSQs.BitCount()
										                     << "   Total White Attks:"   << setw(2) << this->wTotalAttks;
									 else               cout << "  " << "Black Attk SQs:" << setw(2) << this->bb64_bPiecesAttksSQs.BitCount() 
										                     << "   Black Prots SQs : "   << setw(2) << this->bb64_bPiecesProtsSQs.BitCount()
									                         << "   Total Black Prots:"   << setw(2) << this->bTotalProts;
									 cout << endl;
									 break;
						case RANK_5: cout << endl;
									 cout << "  |                                 |     ";
									 if (side == WHITE) cout << "  " << "wRook   Attks:" << setw(2) << this->bb64_wQueenAttks.BitCount() << "   wRook   Prots : " << setw(2) << this->bb64_wRookProts.BitCount();
									 else               cout << "  " << "bRook   Attks:" << setw(2) << this->bb64_bQueenAttks.BitCount() << "   bRook   Prots : " << setw(2) << this->bb64_bRookProts.BitCount(); 
									 cout << endl;
									 break;
						case RANK_4: if (side == WHITE) cout << "  " << "wQueen  Attks:" << setw(2) << this->bb64_wQueenAttks.BitCount() << "   wQueen  Prots : " << setw(2) << this->bb64_wQueenProts.BitCount();
									 else               cout << "  " << "bQueen  Attks:" << setw(2) << this->bb64_bQueenAttks.BitCount() << "   bQueen  Prots : " << setw(2) << this->bb64_bQueenProts.BitCount();
									 cout << endl;
									 cout << "  |                                 |     ";
									 if (side == WHITE) cout << "  " << "wPawn   Attks:" << setw(2) << this->bb64_wPawnAttks.BitCount()  << "   wPawn   Prots : " << setw(2) << this->bb64_wPawnProts.BitCount();
									 else               cout << "  " << "bPawn   Attks:" << setw(2) << this->bb64_bPawnAttks.BitCount()  << "   bPawn   Prots : " << setw(2) << this->bb64_bPawnProts.BitCount();
									 cout << endl;
									 break;
						case RANK_3: if (side == WHITE) cout << "  " << "wKnight Attks:" << setw(2) << this->bb64_wKnightAttks.BitCount() << "   wKnight Prots : " << setw(2) << this->bb64_wKnightProts.BitCount();
									 else               cout << "  " << "bKnight Attks:" << setw(2) << this->bb64_bKnightAttks.BitCount() << "   bKnight Prots : " << setw(2) << this->bb64_bKnightProts.BitCount();
									 cout << endl;
									 cout << "  |                                 |     ";
									 if (side == WHITE) cout << "  " << "wBishop Attks:" << setw(2) << this->bb64_wQueenAttks.BitCount() << "   wBishop Prots : " << setw(2) << this->bb64_wBishopProts.BitCount();
									 else               cout << "  " << "bBishop Attks:" << setw(2) << this->bb64_bQueenAttks.BitCount() << "   bBishop Prots : " << setw(2) << this->bb64_bBishopProts.BitCount();
									 cout << endl;
									 break;
						case RANK_2: cout << endl;
									 cout << "  |                                 |     ";
									 cout << endl;
									 break;
						case RANK_1: cout << endl;
									 cout << "  |                                 |     ";
									 cout << endl;
									 break;
					}
					//cout << endl;
					//if (rank != RANK_1)
					//{
					//	cout << "  |                                 |     ";
					//	switch(rank)
					//	{
					//	    case RANK_8: cout << "  " << "enPas SQ:" << SQenPas120;  break;
					//		case RANK_7: cout << "  ";	 break;
					//		case RANK_6: cout << "  ";	 break;
					//		case RANK_5: cout << "  ";	 break;
					//		case RANK_4: cout << "  ";	 break;
					//		case RANK_3: cout << "  ";	 break;
					//		case RANK_2: cout << "  ";	 break;
					//	}
					//cout << endl;
					//}
				}
                
				cout << "  |_________________________________|   " << endl;
				cout << endl;
				cout << "  ";
				for (file = FILE_A; file <= FILE_H; file++) 
					cout << " " << setw(3) << (char) ('a' + file);
				cout << endl;

				cout << "======== END PRINT BOARD ================" << endl << endl;
			}
	C_BOARD*       C_BOARD::DeepClone            (                                                )
	{
		ASSERT(CheckBoard());

		C_BOARD *pBoard = new C_BOARD();

		pBoard->bb64_wPawns       = this->bb64_wPawns      ;   pBoard->bb64_bPawns      = this->bb64_bPawns      ;
		pBoard->bb64_wRooks       = this->bb64_wRooks      ;   pBoard->bb64_bRooks      = this->bb64_bRooks      ;
		pBoard->bb64_wKnights     = this->bb64_wKnights    ;   pBoard->bb64_bKnights    = this->bb64_bKnights    ;
		pBoard->bb64_wBishops     = this->bb64_wBishops    ;   pBoard->bb64_bBishops    = this->bb64_bBishops    ;
		pBoard->bb64_wQueens      = this->bb64_wQueens     ;   pBoard->bb64_bQueens     = this->bb64_bQueens     ;
		pBoard->bb64_wKings       = this->bb64_wKings      ;   pBoard->bb64_bKings      = this->bb64_bKings      ;
		 
		pBoard->bb64_wPawnAttks   = this->bb64_wPawnAttks  ;  pBoard->bb64_wPawnProts   = this->bb64_wPawnProts  ;
		pBoard->bb64_wKnightAttks = this->bb64_wKnightAttks;  pBoard->bb64_wKnightProts = this->bb64_wKnightProts;
		pBoard->bb64_wBishopAttks = this->bb64_wBishopAttks;  pBoard->bb64_wBishopProts = this->bb64_wBishopProts;
		pBoard->bb64_wRookAttks   = this->bb64_wRookAttks  ;  pBoard->bb64_wRookProts   = this->bb64_wRookProts  ;
		pBoard->bb64_wQueenAttks  = this->bb64_wQueenAttks ;  pBoard->bb64_wQueenProts  = this->bb64_wQueenProts ;
		pBoard->bb64_wKingAttks   = this->bb64_wKingAttks  ;  pBoard->bb64_wKingProts   = this->bb64_wKingProts  ;

		pBoard->bb64_wPiecesAttksSQs = pBoard->bb64_wPawnAttks | pBoard->bb64_wKnightAttks | pBoard->bb64_wBishopAttks | pBoard->bb64_wRookAttks | pBoard->bb64_wQueenAttks | pBoard->bb64_wKingAttks;
		pBoard->bb64_wPiecesProtsSQs = pBoard->bb64_wPawnProts | pBoard->bb64_wKnightProts | pBoard->bb64_wBishopProts | pBoard->bb64_wRookProts | pBoard->bb64_wQueenProts | pBoard->bb64_wKingProts;

		pBoard->wTotalAttks = pBoard->bb64_wPawnAttks.BitCount() + pBoard->bb64_wKnightAttks.BitCount() + pBoard->bb64_wBishopAttks.BitCount() + pBoard->bb64_wRookAttks.BitCount() + pBoard->bb64_wQueenAttks.BitCount() + pBoard->bb64_wKingAttks.BitCount();
		pBoard->wTotalProts = pBoard->bb64_wPawnProts.BitCount() + pBoard->bb64_wKnightProts.BitCount() + pBoard->bb64_wBishopProts.BitCount() + pBoard->bb64_wRookProts.BitCount() + pBoard->bb64_wQueenProts.BitCount() + pBoard->bb64_wKingProts.BitCount();

		pBoard->bb64_bPawnAttks   = this->bb64_bPawnAttks  ;  pBoard->bb64_bPawnProts   = this->bb64_bPawnProts  ;
		pBoard->bb64_bKnightAttks = this->bb64_bKnightAttks;  pBoard->bb64_bKnightProts = this->bb64_bKnightProts;
		pBoard->bb64_bBishopAttks = this->bb64_bBishopAttks;  pBoard->bb64_bBishopProts = this->bb64_bBishopProts;
		pBoard->bb64_bRookAttks   = this->bb64_bRookAttks  ;  pBoard->bb64_bRookProts   = this->bb64_bRookProts  ;
		pBoard->bb64_bQueenAttks  = this->bb64_bQueenAttks ;  pBoard->bb64_bQueenProts  = this->bb64_bQueenProts ;
		pBoard->bb64_bKingAttks   = this->bb64_bKingAttks  ;  pBoard->bb64_bKingProts   = this->bb64_bKingProts  ;

		pBoard->bb64_bPiecesAttksSQs = pBoard->bb64_bPawnAttks | pBoard->bb64_bKnightAttks | pBoard->bb64_bBishopAttks | pBoard->bb64_bRookAttks | pBoard->bb64_bQueenAttks | pBoard->bb64_bKingAttks;
		pBoard->bb64_bPiecesProtsSQs = pBoard->bb64_bPawnProts | pBoard->bb64_bKnightProts | pBoard->bb64_bBishopProts | pBoard->bb64_bRookProts | pBoard->bb64_bQueenProts | pBoard->bb64_bKingProts;

		pBoard->bTotalAttks = pBoard->bb64_bPawnAttks.BitCount() + pBoard->bb64_bKnightAttks.BitCount() + pBoard->bb64_bBishopAttks.BitCount() + pBoard->bb64_bRookAttks.BitCount() + pBoard->bb64_bQueenAttks.BitCount() + pBoard->bb64_bKingAttks.BitCount();
		pBoard->bTotalProts = pBoard->bb64_bPawnProts.BitCount() + pBoard->bb64_bKnightProts.BitCount() + pBoard->bb64_bBishopProts.BitCount() + pBoard->bb64_bRookProts.BitCount() + pBoard->bb64_bQueenProts.BitCount() + pBoard->bb64_bKingProts.BitCount();

		ASSERT(this  ->bb64_wKings.BitCount() == 1);
		ASSERT(pBoard->bb64_wKings.BitCount() == 1);

	    pBoard->bb64_OccupiedSQs= C_BB_64(this->bb64_OccupiedSQs);       
	    pBoard->bb64_EmptySQs   = C_BB_64(this->bb64_EmptySQs   );       
	    pBoard->bb64_wPiecesSQs = C_BB_64(this->bb64_wPiecesSQs );       
	    pBoard->bb64_bPiecesSQs = C_BB_64(this->bb64_bPiecesSQs );  

	    pBoard->side            = this->side          ;		
	    pBoard->ply             = this->ply           ;       
	    pBoard->hisPly          = this->hisPly        ;   	   
	    pBoard->SQenPas120      = this->SQenPas120    ;       
        pBoard->posKey          = this->posKey        ;       
	    pBoard->fiftyMove       = this->fiftyMove     ;
	    pBoard->castlePerm      = this->castlePerm    ;

		memcpy(pBoard->history, this->history, sizeof(S_UNDO)*MAXGAMEMOVES );   // for (int i = 0; i < MAXGAMEMOVES; i++) pBoard->history[i] = this->history[i];
		memcpy(pBoard->pieces , this->pieces , sizeof(int   )*BRD_SQ_NUM120);   // for (int i = 0; i < BRD_SQ_NUM120; i++) pBoard->pieces[i] = this->pieces[i];
		
		


		for (int i = 0; i < BOTH; i++)
		{
			pBoard->material[i] = this->material[i];
			pBoard->bigPce  [i] = this->bigPce  [i];		  	   
	        pBoard->majPce  [i] = this->majPce  [i];			  
	        pBoard->minPce  [i] = this->minPce  [i];            
		}

		// TODO: are these not needed??
		// below does not need to be clone (rebuilt??)
		//      
		//  	PvArray       [MAX_PLY_DEPTH];
		//      searchHistory [MAXPIECETYPE][BRD_SQ_NUM120];
		//      searchKillers [2][MAX_PLY_DEPTH];
		
		//for (int i=0; i<MAXPIECETYPE; i++) 
		//	for (int j=0; j<BRD_SQ_NUM120; j++) 
		//		pBoard->searchHistory [i][j] = this->searchHistory [i][j];
		//memcpy(pBoard->searchHistory, this->searchHistory, sizeof(int)*MAXPIECETYPE*BRD_SQ_NUM120);
		
		//for (int i=0; i<2; i++) 
		//	for (int j=0; j<MAX_PLY_DEPTH; j++) 
	    //        pBoard->searchKillers [i][j] = this->searchKillers [i][j];
		//memcpy(pBoard->searchKillers, this->searchKillers, sizeof(int)*MAX_PLY_DEPTH * 2);

        //for (int i=0; i<MAX_PLY_DEPTH; i++)
        //    pBoard->PvArray[i] = this->PvArray[i];
		//memcpy(pBoard->PvArray, this->PvArray, sizeof(int)*MAX_PLY_DEPTH);

 
        ASSERT(this->posKey == pBoard->GeneratePosKey());
	 	ASSERT(pBoard->CheckBoard());		
		return pBoard;
	}

#pragma endregion
