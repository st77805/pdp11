#include <stdio.h>
#include <stdlib.h>
//#include <stdarg.h>
#include "common.h"

void do_mov()
{
    w_write(dd.a, ss.val);
    NZVC(ss.val);
}

void do_movb()
{
    b_write(dd.a, ss.val);
    NZVC(ss.val);
    if (dd.a == odata)
        printf("%c", ss.val);
}

void do_add()
{
    w_write(dd.a, (ss.val + dd.val)&0xFF);
    NZVC(ss.val+dd.val);
}

void do_halt ()
{
    printf("THE END!!!\n");
    reg_print();
    exit(0);
}

void do_sob ()
{
    reg[mr]--;
    if (reg[mr] != 0)
        pc -= 2*nn;
    printf("R%d\n", mr);
}

void do_clr ()
{
    w_write(dd.a, 0);
    Z = 1;
}

void do_br ()
{
    pc += xx*2;
    printf("%06o ", pc);
}

void do_beq ()
{
    if (Z == 1)
        do_br();
}

void do_bpl ()
{
    if (N == 0)
        do_br();
}

void do_tstb ()
{
    NZVC(dd.val);
    C = 0;
}

void do_jsr ()
{
    sp -= 2;
    w_write(sp, reg[mr]);
    reg[mr] = pc;
    pc = dd.val;
    printf("R%d ", mr);
}

void do_rts ()
{
    pc = reg[lr];
    reg[lr] = w_read(sp);
    sp += 2;
    printf("R%d ", lr);
}

void do_unknown()
{
    printf("Ooops");
}
