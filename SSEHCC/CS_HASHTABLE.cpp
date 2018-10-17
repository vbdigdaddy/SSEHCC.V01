#pragma once
#include "stdafx.h"
#include "C_BOARD.h"
#include "CS_HASHTABLE.h"
#include <ppl.h>

	CS_HASHTABLE::CS_HASHTABLE() 
	{
		//pHashTable = NULL;
    }
	CS_HASHTABLE& CS_HASHTABLE::getInstance()
	{
		static CS_HASHTABLE instance; // the one and only instance
		return instance;
	}
  
	void CS_HASHTABLE::ClearHashTable (                                                                         )
	{
		cout << endl << "Clearing Hash Table" << endl;
		CLOCK_START;

		// this->pHashTable[].posKey         = 0;
		// this->pHashTable[].move           = NOMOVE;
		// this->pHashTable[].depthRemaining = 0;
		// this->pHashTable[].score          = 0;
		// this->pHashTable[].flags          = 0;
	    memset(this->pHashTable, 0, sizeof(S_HASHENTRY)*htNumEntries);

	    htNewWrites = 0;
	    htOverWrite = 0;
	    htHits 		= 0;
	    htCuts      = 0;
		
		CLOCK_STOP;
		if (THECLOCK <= 0) THECLOCK = 1;
		cout << "Hash Table Clear ET = " << FormatWithCommas(THECLOCK) << " ms" << endl;
 	}

	U64 LimitHashTableSize(U64 sz)
	{
		// limit to range [MIN_HASHTABLE_SIZE,MAX_HASHTABLE_SIZE]
		return __min(CS_HASHTABLE::MAX_HASHTABLE_SIZE,__max(CS_HASHTABLE::MIN_HASHTABLE_SIZE, sz)); 
	}

	void CS_HASHTABLE::InitHashTable  (U64 HashTableSizeInMB                                                    )
	{
		U64 HashTableSize = LimitHashTableSize(HashTableSizeInMB) * 1024 * 1024;

		htNumEntries = (U32)(HashTableSize / sizeof(S_HASHENTRY) );
		
		if (pHashTable != NULL)
			free(pHashTable);

		pHashTable = new (std::nothrow)S_HASHENTRY[htNumEntries]; 
		int sz = htNumEntries*(int)sizeof(S_HASHENTRY) / 1000000;
		cout << endl << "HashTable:" << endl
		     << "    HashTable total size:      " << FormatWithCommas(sz) << " MB" << endl
		     << "    HashTable clear completed: " << FormatWithCommas(htNumEntries) << " total possible entries." << endl
			 << endl;
		
		ClearHashTable();

	}
	bool CS_HASHTABLE::ProbeHashEntry (const C_BOARD *pBoard, int *move, int *score, int alpha,   int beta, DEPTH depthRemaining)
	{
		if (!USE_HASH_TABLES)
			return false;

		bool returnValue = false;

		U64 index = pBoard->posKey % htNumEntries;

		ASSERT(index          >= 0               && index          <= htNumEntries - 1);
		ASSERT(depthRemaining >= 1               && depthRemaining <  MAX_PLY_DEPTH);
		ASSERT(alpha          < beta);
		ASSERT(alpha          >= -CHESSINFINITE  && alpha          <= CHESSINFINITE);
		ASSERT(beta           >= -CHESSINFINITE  && beta           <= CHESSINFINITE);
		ASSERT(pBoard->ply    >= 0               && pBoard->ply    <  MAX_PLY_DEPTH);

		HT_LOCK_PROTECT( if (pHashTable[index].posKey == pBoard->posKey)
		                 {
		                 	*move = pHashTable[index].move; // found PV Move; return it
		                 	if (pHashTable[index].depthRemaining >= depthRemaining)
		                 	{
		                 		htHits++;
		                 		ASSERT(pHashTable[index].depthRemaining >= 1       && pHashTable[index].depthRemaining <  MAX_PLY_DEPTH);
		                 		ASSERT(pHashTable[index].flags          >= HFALPHA && pHashTable[index].flags          <= HFEXACT);
		                 		*score = pHashTable[index].score;
		                 		ASSERT(*score >= -CHESSINFINITE && *score <= CHESSINFINITE);
		                 
		                 		switch(pHashTable[index].flags)
				                {
				                	case HFALPHA:
				                		if (*score <= alpha)
				                		{
				                			*score = alpha;
				                			returnValue = true;
				                		}
				                		break;
				                
				                	case HFBETA:
				                		if (*score >= beta)
				                		{
				                			*score = beta;
				                			returnValue = true;
				                		}
				                		break;
				                
				                	case HFEXACT:
				                		returnValue = true;
				                		break;
				                
				                	default:
				                		ASSERT(false); 
				                		break;
				                }
		                 	}
		                 }
		               );


		return returnValue;
	}
	void CS_HASHTABLE::StoreHashEntry (const C_BOARD *pBoard, int  move, int  score, FLAGS flags, DEPTH depthRemaining)
	{
		if (!USE_HASH_TABLES)
			return;

		U64 index = pBoard->posKey % htNumEntries;

		ASSERT(index          >= 0              && index          <= htNumEntries - 1);
		ASSERT(depthRemaining >= 1              && depthRemaining <  MAX_PLY_DEPTH);
		ASSERT(flags          >= HFALPHA        && flags          <= HFEXACT);
		ASSERT(score          >= -CHESSINFINITE && score          <= CHESSINFINITE);
		ASSERT(pBoard->ply    >= 0              && pBoard->ply    <  MAX_PLY_DEPTH);
        
		HT_LOCK_PROTECT( if (pHashTable[index].posKey == 0) htNewWrites++; else htOverWrite++;
						 pHashTable[index].posKey          = pBoard->posKey;
						 pHashTable[index].move            = move;
						 pHashTable[index].flags           = flags;
						 pHashTable[index].score           = score;
						 pHashTable[index].depthRemaining  = depthRemaining;
		               );
	}
	int  CS_HASHTABLE::ProbePvMove    (U64 posKey)
	{
		if (!USE_HASH_TABLES)
			return false;

		U64 index     = posKey % htNumEntries;
		ASSERT(index >= 0 && index <= htNumEntries - 1);

		int move;
		HT_LOCK_PROTECT(move = (pHashTable[index].posKey == posKey) ? pHashTable[index].move : NOMOVE; ); 

		return move;
	}
