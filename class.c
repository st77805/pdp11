#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <WinError.h>
#include <stdarg.h>
#include "common.h"

int dbg_level = DEBUG;
int no_bit;

void trace (int dbg_lvl, const char * format, ...) //принт с аргументом, только дебаг
{
    if (dbg_lvl != dbg_level)
        return;
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

int N, Z, V, C;
int nn, xx, mr, lr;
byte mem[56*1024];
word reg[8];

struct SSDD ss, dd;

void reg_print ()
{
    int i;
    for (i = 0; i < 8; i += 2)
        printf("R%d : %06o  ", i, reg[i]);
    printf("\n");
    for (i = 1; i < 8; i += 2)
        printf("R%d : %06o  ", i, reg[i]);

}

void load_file()
{
	FILE * f = NULL;
	adr a;
	word n;
	f = fopen("C:\\Users\\Tanya\\pdp11\\gitrepo\\tests\\03_sob_byte\\sumvar_byte.txt.o", "r");
	if (f == NULL) {
		perror("C:\\Users\\Tanya\\pdp11\\gitrepo\\tests\\03_sob_byte\\sumvar_byte.txt.o");
		exit(7);
	}

	unsigned int b;
	while (fscanf(f, "%x%x", &a, &n) == 2)
	{
		int i;
		for (i = 0; i < n; i++)
		{
			fscanf(f, "%x", &b);
			b_write(i + a, (byte)b);
		}
	}
}

void mem_dump(adr s, word n)
{
	word w;
	adr i;
	assert(s % 2 == 0);
	for (i = s; i < s + n; i = i + 2)
	{
		w = w_read(i);
		trace(dbg_level, "%06o : %06o\n", i, w);
	}
}

struct SSDD get_m (word w) {
    struct SSDD res;
    int n = w & 7;
    int mode = (w >> 3) & 7;
    int b, b0;
    b0 = no_bit;
    b = (((no_bit)||(n==6)||(n==7)) ? 2 : 1);
    switch (mode) {
    case 0:
        res.a = n;
        res.val = reg[n];
        printf("0R%d ", n);
        break;
    case 1:
        res.a = reg[n];
        res.val = bw_read(res.a, b0, n);
        printf("1R%d ", n);
        break;
    case 2:
        //printf("+n=%d, regn=%o+\n", n, reg[n]);
        res.a = reg[n];
        res.val = bw_read(res.a, b0, n); //why 0??
        reg[n] += b;
        if (n == 7)
            printf("2#%o ", res.val);
        else
        {
            printf("(2R%d)+ ", n);
            //printf("--2#%o-- ", res.val);
        }
        break;
    case 3:
        res.a = reg[n];
        reg[n] += b;
        res.a = bw_read(res.a, b0, n);
        res.val = bw_read(res.a, b0, n);
        if (n == 7)
            printf("3@#%o ", res.val);
        else
            printf("(3R%d)+ ", n);
        break;
    case 4:
        reg[n] -= b;
        res.a = reg[n];
        res.val = bw_read(res.a, b0, n);
        if (n == 7)
            printf("4#%o ", res.val);
        else
            printf("(4R%d)- ", n);
        break;
    case 5:
        res.a = reg[n];
        reg[n] -= b;
        res.a = bw_read(res.a, b0, n);
        res.val = bw_read(res.a, b0, n);
        if (n == 7)
            printf("5@#%o ", res.val);
        else
            printf("(R%d)- ", n);
        break;
    default:
        printf("This mode hasn't been open yet");
    }
    return res;
}

void NZVC (word w)
{
    N = (no_bit ? w >> 15 : w >> 7) & 1;
    Z = (w == 0);
    C = (no_bit ? w >> 16 : w >> 8) & 1;
}
struct comm command [] = {
    {0010000, 0170000, "mov", do_mov, HAS_SS | HAS_DD, 1},
    {0060000, 0170000, "add", do_add, HAS_SS | HAS_DD, 1},
    {0000000, 0177777, "halt", do_halt, NO_PARAM, 1},
    {0110000, 0170000, "movb", do_movb, HAS_SS | HAS_DD, 0},
    {0077000, 0177000, "sob", do_sob, HAS_NN | HAS_MR, 1},
    {0005000, 0177700, "clr", do_clr, HAS_DD, 1},
    {0, 0, "unknown", do_unknown, NO_PARAM, 1},
};

void run()
{
    pc = 001000;
    int i;
    while(1)
    {
        word w = w_read(pc) & 0xFFFF;
        fprintf(stdout, "%06o : %06o", pc, w);
        pc += 2;
        struct comm cmd;
        for (i = 0; ; i++)
        {
            cmd = command[i];
            if ((w & cmd.mask) == cmd.opcode)
            {
                printf(" %s ", cmd.name);
                no_bit = cmd.is_no_bit;
                if (cmd.param & HAS_SS)
                {
                    ss = get_m(w>>6);
                    //printf ("ss.a =%o, ss.val = %o ");
                }
                if (cmd.param & HAS_DD)
                {
                    dd = get_m(w);
                    //printf ("dd.a =%o, dd.val = %o ");
                }
                if (cmd.param & HAS_NN)
                {
                    nn = w&63;
                }
                if (cmd.param & HAS_MR)
                {
                    mr = (w>>6)&7;
                }
                cmd.do_func();
                break;
            }
        }
        printf("\n");
        /*printf("***\n");
        reg_print();
        printf("***\n");*/
    }
}
/*
int main()
{
    load_file();
    run();
    return 0;
}
*/
