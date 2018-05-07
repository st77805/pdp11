#ifndef _COMMON_H
#define _COMMON_H

#define RELELASE 0
#define DEBUG 1
#define FULL_DEBAG 2

typedef unsigned char byte;
typedef int word;
typedef word adr;

#define NO_PARAM 0
#define HAS_XX 1
#define HAS_SS (1<<1)
#define HAS_DD (1<<2)
#define HAS_NN (1<<3)
#define HAS_MR (1<<4)
#define HAS_LR (1<<5)

#define pc reg[7]

struct SSDD {
    word val;
    adr a;
};

struct comm {
    word opcode;
    word mask;
    const char * name;
    void (*do_func)();
    byte param;
    int is_no_bit;
};

extern int dbg_level;
extern int no_bit;
extern int N, Z, V, C;
extern int nn, xx, mr, lr;
extern byte mem[56*1024];
extern word reg[8];

extern struct SSDD ss, dd;
extern struct comm command [];

void b_write(adr a, byte x);
void w_write(adr a, word x);
byte b_read(adr a);
word w_read(adr a);
word bw_read (adr a, int nb, int n);

void do_mov();
void do_movb();
void do_add();
void do_sob();
void do_clr();
void do_halt();
void do_unknown();

void reg_print ();
void NZVC (word w);

#endif // _COMMON_H
