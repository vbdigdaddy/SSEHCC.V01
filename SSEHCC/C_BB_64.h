#pragma once
#include "stdafx.h"

class C_BB_64 
{

public:
	C_BB_64();
	C_BB_64(U64 bb);

	int      BitGet();
	int      BitPop();
	int      BitCount();
	int      BitTrailing0();
	int      BitLeading0();
	C_BB_64  BitReverse();

	void     SetBit(int n);
	void     ClrBit(int n);

	void     PrintBitBoard();

	// assignment operators
	C_BB_64  operator =  (C_BB_64& bb64);
	C_BB_64  operator =  (U64    n   );

	// unary operators
	C_BB_64  operator ~  ();

	// binary operators
	C_BB_64  operator ^  (C_BB_64  bb64) const;  // bitwise xor
	C_BB_64  operator |  (C_BB_64  bb64) const;  // bitwise or
	C_BB_64  operator |  (U64      n   ) const;  // bitwise or
	C_BB_64  operator || (C_BB_64  bb64) const;  //         or
	C_BB_64  operator &  (C_BB_64  bb64) const;  // bitwise and
	C_BB_64  operator &  (U64      n   ) const;  // bitwise or
	C_BB_64  operator && (C_BB_64  bb64) const;  //         and

	// comparison operators
	bool     operator == (C_BB_64  bb64) const;
	bool     operator == (U64      n   ) const;
	bool     operator != (C_BB_64  bb64) const;
	bool     operator != (U64      n   ) const;


//private:
	U64      value;  // TODO should be private after we get need operators overloaded
};

