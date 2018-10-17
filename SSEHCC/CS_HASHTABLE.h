#pragma once
#include "stdafx.h"

#define HT_LOCK                  { htLock.lock();   }
#define HT_UNLOCK                { htLock.unlock(); }
#define HT_LOCK_PROTECT(stmts)   {  HT_LOCK;   stmts;  HT_UNLOCK;  } 


class CS_HASHTABLE // Singalton Class
{
protected:
	CS_HASHTABLE();

public:
	static CS_HASHTABLE& getInstance();

	void   ClearHashTable ();
	void   InitHashTable  (U64 HashTableSizeInMB);
	bool   ProbeHashEntry (const C_BOARD *pBoard, int *move, int *score, int alpha, int beta, DEPTH depth);
	void   StoreHashEntry (const C_BOARD *pBoard, int  move, int  score, FLAGS flags,         DEPTH depth);
	int    ProbePvMove    (U64 posKey);

    const static int MAX_HASHTABLE_SIZE     = (4*1024);   // MB (note: theoritical maximum size is limited to U64 and available physical memory)
	const static int DEFAULT_HASHTABLE_SIZE = (4*1024);   // MB 
    const static int MIN_HASHTABLE_SIZE     = (256);      // MB

	bool USE_HASH_TABLES = true;

	U64             htNewWrites;
	U64             htOverWrite;
	U64             htHits;
	U64             htCuts;

private:

	mutex           htLock;
	S_HASHENTRY    *pHashTable;
	U32             htNumEntries;

};