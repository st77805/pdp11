#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <stdarg.h>
#include "common.h"

void b_write(adr a, byte x)
{
    if (a < 15)
        reg[a] = x & 0xff;
    else
        mem[a] = x;
}

byte b_read(adr a)
{
	return mem[a];
}

void w_write(adr a, word x)
{
    if (a < 15)
        reg[a] = x;
    else
    {
        mem[a] = (byte)(x & 0xFF);
        mem[a+1] = (byte)((x >> 8) & 0xFF);
    }
}

word w_read(adr a)
{
	word res;
	assert(a % 2 == 0);
	res = (word)(mem[a]) | (word)(mem[a+1] << 8);
	return res;
}

word bw_read (adr a, int nb)
{
    return (nb ? w_read(a) : b_read(a));
}
