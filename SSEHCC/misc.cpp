// misc.c

#include "stdafx.h"
#include "windows.h"

S64 GetTimeMs() 
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	return 1000*(60*(st.wHour*60+st.wMinute) + st.wSecond) + st.wMilliseconds;

}

#pragma region SKAN

std::string token;
std::string skanline;
std::string skan(std::string& line)
{
	size_t tokenindex = line.find(' ');
	if (tokenindex == string::npos)
	{
		token = line;
		line = "";
	}	
	else
	{
		token = line.substr(0, tokenindex);
		line = line.substr(tokenindex + 1);
	}

	return token;
}

#pragma endregion

string Trim(string s)
{
   size_t iStart = 0;
   size_t iEnd   = (int)s.length()-1;

   while ( (iStart < s.length()) && (s.at(iStart) == ' ') )
	   iStart++;
   if (iStart >= s.length())
	   return s;

   while( (iEnd > 0) && (s.at(iEnd) == ' '))
	   iEnd--;

   if (iEnd < 0)
	   return s;

   return s.substr(iStart,iEnd-iStart+1);

}

void PUNT(string msg)
{
	msg = "Error: " + msg;
	throw std::exception(msg.c_str());
}

void PrintBB(U64 bb)
{
   // print binary bitboard
   for (int i = 0; i < 64; i++)
   {
      if ( i%8==0) cout << endl;
      if (bb & 1ULL<<i) cout << " x "; else cout << " . ";
   }
   cout << endl << endl;

}

void CMDLineReaderThread()
{
	std::getline(std::cin, cmdLine);
	cmdLineAvailable = true;
}

void GetCMDLine()
{
	std::getline(std::cin, cmdLine);
	return;
	/*
	// start line reader thread
	thread cmdLineReaderThread(CMDLineReaderThread);
	int ponderCount = 0;
	cmdLineAvailable = false;

	while (!cmdLineAvailable)
	{
#ifdef DEBUG
		if (++ponderCount >= 3000)
		{
			cout << "info pondering..." << endl;
			ponderCount = 0;
		}
#endif
		THREAD_SLEEP_MILLISECS(10);
		// ponder(pBoard); 
	}

	cmdLineReaderThread.join();  // should already be completed
	cmdLineAvailable = false;
	*/
}

boost::asio::io_service                  ioService;
boost::thread_group                      threadPool;
boost::asio::io_service::work            work(ioService);
boost::mutex                             thread_mutex;