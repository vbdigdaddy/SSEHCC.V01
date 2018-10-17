#include "stdafx.h"
#include "C_BB_64.h"

         C_BB_64::C_BB_64(      ) { this->value = 0;  }
         C_BB_64::C_BB_64(U64 bb) { this->value = bb; }
	     
int      C_BB_64::BitGet        ()
{  
	U64 b = this->value ^ (this->value - 1);
	unsigned long index;
	_BitScanForward64(&index, this->value);

#ifdef DEBUG
	int n = BitTable[((unsigned)((b & 0xffffffff) ^ (b >> 32)) * 0x783a9b23) >> 26];
	ASSERT(n == index);
	ASSERT(this->value != 0);
#endif

	return (int)index;
}
int      C_BB_64::BitPop        ()
{   
	int n        = BitGet();          // get  Bit #
	this->value &= (this->value - 1); // turn bit off
	return n;
}
int      C_BB_64::BitCount      ()
{
	return (int)__popcnt64(this->value);
}
int      C_BB_64::BitTrailing0  ()
{
	if (this->value == 0)
		return BRD_SQ_NUM64;
	
	U32 hiVal  = this->value >> 32;
	U32 lowVal = (U32)this->value;
	
	if (lowVal == 0)
		return Mod37BitPosition[(-(S64)hiVal & hiVal) % 37] + 32;
	
	return Mod37BitPosition[(-(S64)lowVal & lowVal) % 37];

}
int      C_BB_64::BitLeading0   ()
{

	if (this->value == 0)
		return BRD_SQ_NUM64;

	S32  hiVal  = (S32)(this->value >> 32);
	S32  lowVal = (S32)this->value;
	S32  val    = (hiVal == 0) ? lowVal : hiVal;
	int  n      = 0;
	int  bits   = 32;


	while (bits-- > 0)
	{
		if (val == 0) break;
		n++;
		val >>= 1;
	}

	if (hiVal == 0)
		return (BRD_SQ_NUM64 - n);
	return (32 - n);
}
C_BB_64  C_BB_64::BitReverse    ()
{
	if (this->value == 0)
		return 0;

	U64 i0 = BitReverseTable[this->value >>  0 & 0xff];
	U64 i1 = BitReverseTable[this->value >>  8 & 0xff];
	U64 i2 = BitReverseTable[this->value >> 16 & 0xff];
	U64 i3 = BitReverseTable[this->value >> 24 & 0xff];
	U64 i4 = BitReverseTable[this->value >> 32 & 0xff];
	U64 i5 = BitReverseTable[this->value >> 40 & 0xff];
	U64 i6 = BitReverseTable[this->value >> 48 & 0xff];
	U64 i7 = BitReverseTable[this->value >> 56 & 0xff];
	
	U64 rst = ((((((((        i0
		             ) << 8 | i1
		            )  << 8 | i2
		           )   << 8 | i3
		          )    << 8 | i4
		         )     << 8 | i5
		        )      << 8 | i6
		       )       << 8 | i7
		      );
	
	return C_BB_64(rst);
}

void     C_BB_64::SetBit(int n)
{
	this->value |= SetMask[n];
}	
void     C_BB_64::ClrBit(int n)
{
	this->value &= ClearMask[n];
}


// assignment operators
C_BB_64  C_BB_64::operator =  (C_BB_64& bb64)
{
	this->value = bb64.value;
	return ( C_BB_64(this->value) );
}
C_BB_64  C_BB_64::operator =  (U64    n   )
{
	this->value = n;
	return (C_BB_64(this->value));
}

// unary operators
C_BB_64  C_BB_64::operator ~  ()
{
	return C_BB_64( ~this->value );
}

// binary operators
C_BB_64  C_BB_64::operator ^  (C_BB_64  bb64) const  // bitwise xor
{
	return (this->value ^ bb64.value);
}
C_BB_64  C_BB_64::operator |  (C_BB_64  bb64) const  // bitwise or
{
	return (this->value | bb64.value);
}
C_BB_64  C_BB_64::operator |  (U64      n   ) const  // bitwise or
{
	return  (this->value | n );
}
C_BB_64  C_BB_64::operator || (C_BB_64  bb64) const  //         or
{
	return ( this->value || bb64.value );
}
C_BB_64  C_BB_64::operator &  (C_BB_64  bb64) const  // bitwise and
{
	return ( this->value &  bb64.value );
}
C_BB_64  C_BB_64::operator &  (U64      n   ) const  // bitwise and
{
	return ( this->value & n );
}
C_BB_64  C_BB_64::operator && (C_BB_64  bb64) const  //         and
{
	return ( this->value && bb64.value );
}

// comparison operators
bool     C_BB_64::operator == (C_BB_64  bb64) const
{
	return ( this->value == bb64.value );
}
bool     C_BB_64::operator == (U64      n   ) const
{
	return ( this->value == n );
}
bool     C_BB_64::operator != (C_BB_64  bb64) const
{
	return ( this->value != bb64.value );
}
bool     C_BB_64::operator != (U64      n   ) const
{
	return (this->value != n);
}

void     C_BB_64::PrintBitBoard ()
{

	U64 shiftMe = this->value;

	int r     = 0;
	int f     = 0;
	int sq120 = 0;
	int sq64  = 0;

	cout << endl;
	for (r = RANK_8; r >= RANK_1; r--)
	{
		for (f = FILE_A; f <= FILE_H; f++)
		{
			sq120 = FR2SQ(f, r);			
			sq64  = SQ64(sq120);      

			if (((U64)1 << sq64) & this->value)
				cout << "X";
			else
				cout << "-";

		}
		cout << endl;
	}
	cout << endl << endl;
}
