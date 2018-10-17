// SSEHCC.cpp : main project file.
#include "stdafx.h"
#include "C_BOOKTABLE.h"
#include "C_BOARD.h"
#include "CS_HASHTABLE.h"
		      
extern void Uci_Loop     (C_BOARD *pBoard, SEARCHINFO *pInfo);		      
extern void Console_Loop (C_BOARD *pBoard, SEARCHINFO *pInfo);

#pragma warning( disable: 4996)

int main(int argc, char *argv[]) 
{
#ifdef DEBUG
	cout << "DEBUG code enabled"  << endl;
#else
	cout << "DEBUG code disabled" << endl;
#endif

	AllInit();

    C_BOARD       *pBoard = new C_BOARD();
	SEARCHINFO  *pInfo  = new SEARCHINFO();

	pInfo->quit = false;
	HASH.InitHashTable(CS_HASHTABLE::DEFAULT_HASHTABLE_SIZE);

	#pragma region Handle Arguments
	for (int ArgNum = 0; ArgNum < argc; ++ArgNum)
	{
		if (strncmp(argv[ArgNum], "NoBook", 6) == 0)
		{
			EngineOptions->UseBook = false;
			cout << "UseBook Off" << endl;
		}
	}
	#pragma endregion

	cout << endl << endl << endl << "Welcome to SSEHCC! Type 'con' for console mode..." << endl;

	std::string line;

	while (true)
	{
		if (!std::getline(std::cin, line))
			continue;

		if ( line.compare("uci") == 0)
		{
			Uci_Loop(pBoard, pInfo);
			if ( pInfo->quit ) break;
			continue;
		}

		if (line.compare("con") == 0  || line.compare("con ") == 0)
		{
			cout << "console mode selected" << endl;
			Console_Loop(pBoard, pInfo);
			if ( pInfo->quit ) break;
			continue;
		}

		if (line.compare("quit") == 0)
		{
			break;
		}
		
		cout << "unknown input-'" << line << endl;

	}

	BOOK.CleanPolyBook();
    ioService.stop();
	return 0;
}
