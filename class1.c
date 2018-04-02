#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <WinError.h>
#include <string.h>
#include <stdarg.h>
#define RELELASE 0
#define DEBUG 1
#define FULL_DEBAG 2

int dbg_lvl  = DEBUG;

void trace (int dbg_lvl, const char * format, ...); //принт с аргументом, только дебаг
{
    if (dbg_lvl ...)
        return;
    va_list ap;
    va_start(format, ap);
    vprintf(format, ap);
    va_end(ap);
}

typedef unsigned char byte;
typedef int word;
typedef word adr;

byte mem[56*1024];

word reg[8];

void do_mov()
{
    ;
}
void do_add()
{
    ;
}
void do_halt ()
{
    printf("THE END!!!\n");
    exit(1);
}
void do_unknown()
{
    ;
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
	mem[a] = (byte)(x & 0xFF);
	mem[a+1] = (byte)((x >> 8) & 0xFF);
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
	trace("%04x = %02hhx%02hhx\n", w, b1, b0);

	w = 0x0B0A;
	w_write(2, w);
	b0 = b_read(2);
	b1 = b_read(3);
	trace("%04x = %02hhx%02hhx\n", w, b1, b0);
}

void load_file()
{
	FILE * f = NULL;
	adr a;
	word n;
	f = fopen("in.txt", "r");
	if (f == NULL) {
		perror("in.txt");
//		return 7;
	}

	byte b;
	while (fscanf(f, "%x%x", &a, &n) == 2)
	{
		adr i;
		for (i = a; i < a + n; i++)
		{
			fscanf(f, "%02hhx", &b);
			b_write(i, b);
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
		trace("%06o : %06o\n", i, w);
	}
}

struct comm {
    word opcode;
    word mask;
    const char * name;
    void (*do_func)();
} command [] = {
    {0010000, 0170000, "mov", do_mov},
    {0060000, 0170000, "add", do_add},
    {0000000, 0177777, "halt", do_halt},
    {0777777, 0170000, "unknown", do_unknown},
};

void run()
{
    word pc = 001000;
    int i;
    while(1)
    {
        word w = w_read(pc);
        fprintf(stdout, "%06o : %06o", pc, w);
        pc += 2;
        struct comm cmd;
        for (i = 0; ((w & command[i].mask) != command[i].opcode) && (strcmp(command[i].name, "unknown") == 0); i++)
            ;
        cmd = command[i];
        trace("%s ", cmd.name);
        cmd.do_func();
    }
}

int main()
{
    run();
    return 1;
}




