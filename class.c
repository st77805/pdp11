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

void test_m()
{
	byte b0, b1;
	word w;

	b0 = 0xA;
	b1 = 0xB;
	b_write(0, b0);
	b_write(1, b1);
	w = w_read(0);
	printf("%04x = %02hhx%02hhx\n", w, b1, b0);

	w = 0x0B0A;
	w_write(2, w);
	b0 = b_read(2);
	b1 = b_read(3);
	printf("%04x = %02hhx%02hhx\n", w, b1, b0);
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

struct SSDD get_m (word w) {
    struct SSDD res;
    int n = w & 7;
    int mode = (w >> 3) & 7;
    switch (mode) {
    case 0:
        res.a = n;
        res.val = reg[n];
        printf("R%d ", n);
        break;
    case 1:
        res.a = reg[n];
        res.val = w_read(res.a);
        printf("R%d ", n);
        break;
    case 2:
        res.a = reg[n];
        res.val = w_read(res.a);
        reg[n] += 2;
        if (n == 7)
            printf("#%o ", res.val);
        else
            printf("(R%d)+ ", n);
        break;
    case 3:
        res.a = reg[n];
        reg[n] += 2;
        res.a = w_read(res.a);
        res.val = w_read(res.a);
        if (n == 7)
            printf("#%o ", res.val);
        else
            printf("(R%d)+ ", n);
        break;
    case 4:
        reg[n] -= 2;
        res.a = reg[n];
        res.val = w_read((res.a));
        if (n == 7)
            printf("#%o ", res.val);
        else
            printf("(R%d)+ ", n);
        break;
    case 5:
        res.a = reg[n];
        reg[n] -= 2;
        res.a = w_read(res.a);
        res.val = w_read(res.a);
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

void do_mov()
{
    w_write(dd.a, ss.val);
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
} command [] = {
    {0010000, 0170000, "mov", do_mov, HAS_SS | HAS_DD},
    {0060000, 0170000, "add", do_add, HAS_SS | HAS_DD},
    {0000000, 0177777, "halt", do_halt, NO_PARAM},
    {0, 0, "unknown", do_unknown, NO_PARAM},
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
        for (i = 0; i < 4; i++)
        {
            cmd = command[i];
            if ((w & cmd.mask) == cmd.opcode)
            {
                printf(" %s ", cmd.name);
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



