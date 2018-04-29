#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <WinError.h>
#include <string.h>
#include <stdarg.h>

#define RELELASE 0
#define DEBUG 1
#define FULL_DEBAG 2

int dbg_level  = DEBUG;
int no_bit;

//void trace (int dbg_lvl, const char * format, ...); //принт с аргументом, только дебаг
//{
 //   if (dbg_lvl != dbg_level)
   //     return;
    //va_list ap;
//    va_start(format, ap);
  //  vprintf(format, ap);
    //va_end(ap);
//}

typedef unsigned char byte;
typedef int word;
typedef word adr;

#define NO_PARAM 0
#define HAS_XX 1
#define HAS_SS (1<<1)
#define HAS_DD (1<<2)
#define HAS_NN (1<<3)
#define HAS_MR (1<<4)


int N, Z, V, C;
int nn, xx;
byte mem[56*1024];
word reg[8];
#define pc reg[7]

struct SSDD {
    word val;
    adr a;
} ss, dd;

void reg_print ()
{
    int i;
    for (i = 0; i < 8; i++)
        printf("R%d : %06o\n", i, reg[i]);
}

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

void load_file()
{
	FILE * f = NULL;
	adr a;
	word n;
	f = fopen("sum.txt", "r");
	if (f == NULL) {
		perror("sum.txt");
		exit(7);
	}

	unsigned int b;
	while (fscanf(f, "%x%x", &a, &n) == 2)
	{
		int i;
		for (i = 0; i < n; i++)
		{
			fscanf(f, "%02hhx", &b);
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
		printf("%06o : %06o\n", i, w);
	}
}

word bw_read (adr a, int nb)
{
    return (nb ? w_read(a) : b_read(a));
}

struct SSDD get_m (word w) {
    struct SSDD res;
    int n = w & 7;
    int mode = (w >> 3) & 7;
    int b, b0;
    b0 = no_bit;
    b = (no_bit ? 2 : 1);
    switch (mode) {
    case 0:
        res.a = n;
        res.val = reg[n];
        printf("R%d ", n);
        break;
    case 1:
        res.a = reg[n];
        res.val = bw_read(res.a, b0);
        printf("R%d ", n);
        break;
    case 2:
        res.a = reg[n];
        res.val = bw_read(res.a, b0);
        reg[n] += b;
        if (n == 7)
            printf("#%o ", res.val);
        else
            printf("(R%d)+ ", n);
        break;
    case 3:
        res.a = reg[n];
        reg[n] += b;
        res.a = bw_read(res.a, b0);
        res.val = bw_read(res.a, b0);
        if (n == 7)
            printf("#%o ", res.val);
        else
            printf("(R%d)+ ", n);
        break;
    case 4:
        reg[n] -= b;
        res.a = reg[n];
        res.val = bw_read(res.a, b0);
        if (n == 7)
            printf("#%o ", res.val);
        else
            printf("(R%d)+ ", n);
        break;
    case 5:
        res.a = reg[n];
        reg[n] -= b;
        res.a = bw_read(res.a, b0);
        res.val = bw_read(res.a, b0);
        if (n == 7)
            printf("#%o ", res.val);
        else
            printf("(R%d)+ ", n);
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

void do_mov()
{
    w_write(dd.a, ss.val);
}

void do_movb()
{
    b_write(dd.a, ss.val);
}

void do_add()
{
    w_write(dd.a, ss.val + dd.val);
}

void do_halt ()
{
    printf("THE END!!!\n");
    reg_print();
    exit(0);
}

void do_sob ()
{

}

void do_unknown()
{
    printf("Ooops");
}

struct comm {
    word opcode;
    word mask;
    const char * name;
    void (*do_func)();
    byte param;
    int is_no_bit;
} command [] = {
    {0010000, 0170000, "mov", do_mov, HAS_SS | HAS_DD, 1},
    {0060000, 0170000, "add", do_add, HAS_SS | HAS_DD, 1},
    {0000000, 0177777, "halt", do_halt, NO_PARAM, 1},
    {0110000, 0170000, "movb", do_movb, HAS_SS | HAS_DD, 0},
    {0077000, 0177000, "sob", do_sob, HAS_NN | HAS_MR, 1},
    {0, 0, "unknown", do_unknown, NO_PARAM, 1},
};

void run()
{
    pc = 001000;
    int i;
    while(1)
    {
        word w = w_read(pc);
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
                 //   printf ("ss.a =%d, ss.val = %d");
                }
                if (cmd.param & HAS_DD)
                {
                    dd = get_m(w);
                 //   printf ("dd.a =%d, dd.val = %d");
                }
                if (cmd.param & HAS_NN)
                {
                    nn = w&63;
                }
                if (cmd.param & HAS_MR)
                {
                    ;
                }
                cmd.do_func();
                break;
            }
        }
        printf("\n");
    }
}


int main()
{
    load_file();
    run();
    return 0;
}



