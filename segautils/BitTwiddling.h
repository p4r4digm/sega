#pragma once

#include "extern_c.h"

SEXTERN_C

typedef unsigned char byte;

int minByteCount(int bitCount);
void setBit(byte *dest, byte pos/*0-7*/, byte value/*0-1*/);
void setBitInArray(byte *dest, int pos, byte value/*0-1*/);
byte getBit(byte dest, byte pos/*0-7*/);
byte getBitFromArray(const byte *dest, int pos);

//returns 0 if compressed size is larger than inBitCount
int compressBitsRLE(const byte *in, const int inBitCount, byte *out);
void decompressRLE(byte *src, int compressedBitCount, byte *dest);

byte arrayIsSolid(byte *src, int bitCount);

END_SEXTERN_C