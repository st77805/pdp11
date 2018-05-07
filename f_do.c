#include <stdio.h>
#include <stdlib.h>
//#include <stdarg.h>
#include "common.h"

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
        pc = pc - 2*nn;
    printf("R%d\n", mr);
    NZVC(pc);
}

void do_clr ()
{
    w_write(dd.a, 0);
}

void do_unknown()
{
    printf("Ooops");
}
