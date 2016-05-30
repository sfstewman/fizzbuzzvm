#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* FizzBuzz VM: a VM for FizzBuzz
 *
 * Blame Nilium on #go-nuts for instigating this
 */

/* Each opcode is an unsigned short, which should correspond to at least 16 bits.
 *
 * Register machine version.  The register machine has 8 registers and
 * no main memory.  Because we don't need main memory.
 *
 * Opcodes are decoded based on the instruction.  The 3 LSBs indicate
 * the opcode
 * There are 
 * Opcodes are arranged as follows:
 *
 *    LSB              MSB
 * Bit: 0123456789ABCDEF
 *      iiiaaaaaaaaaaaaa        single argument, may be register or intermediate
 *      iiiaaabbbbbbbbbb        two arguments, first is a register
 *      iiiaaabbbccccccc        three arguments, first two are registers
 */

enum OPCODES {
  OP_NOP        = 0,    /* NOP, remaining bits are unused (may be used for labels and such) */
  OP_LOAD       = 1,    /* LOAD rD, imm         rD = imm, imm must be between -512 and 511 */
  OP_MOVE       = 2,    /* MOVE rD, rS          rD = rS */
  OP_ADD        = 3,    /* ADD  rD, rA          rD = rD + rS */
  OP_MOD        = 4,    /* MOD  rD, rA          rD = rD % rA */
  OP_CJMP       = 5,    /* CJMP rA, rel         if rA == 0, jump relative by rel
                                                nb: rel must be between -512 and 511 */
  OP_PRINTCHAR  = 6,    /* PRINTCHAR imm        if imm < 8, specifies register r1 - r16, and
                                                the character in that register is printed.
                                                Otherwise, the string constant at imm-16 is
                                                printed */
  OP_PRINTNUM   = 7,    /* PRINTNUM rA          prints the numeric value in rA */

};

/* some convenience macros for encoding */
#define REG(rn) ((unsigned short)((rn-1) & 7))
#define OP(arg) ((unsigned short)(arg & 7))
#define IMM(arg) ((unsigned short)(arg & 0x1fff))

#define OP1(op,arg) ((unsigned short)(OP(op) | (IMM(arg) << 3)))
#define OP2(op,ra,arg) ((unsigned short)(OP(op) | (REG(ra) << 3) | (IMM(arg) << 6)))
#define OP3(op,ra,rb,arg) ((unsigned short)(OP(op) | (REG(ra) << 3) | (REG(rb) << 6) | (IMM(arg) << 9)))

#define LOAD(rdest, val)        OP2(OP_LOAD, rdest, (val + 512))
#define MOVE(rdest, rsrc)       OP2(OP_MOVE, rdest, REG(rsrc))
#define ADD(rdest, rsrc)        OP2(OP_ADD, rdest, REG(rsrc))
#define MOD(rdest, rsrc)        OP2(OP_MOD, rdest, REG(rsrc))
#define CJMP(rz,rel)            OP2(OP_CJMP, rz, (rel+512))
#define PRINTSTR(s)             OP1(OP_PRINTCHAR, (s+8))
#define PRINTCHAR(reg)          OP1(OP_PRINTCHAR, REG(reg))
#define PRINTNUM(reg)           OP1(OP_PRINTNUM, REG(reg))

static unsigned short fizzbuzz[] = {
  /* start by loading constants */
  LOAD(2, 1),           /*  1 */
  LOAD(3, 3),           /*  2 */
  LOAD(4, 5),           /*  3 */
  LOAD(5, 15),          /*  4 */
  LOAD(7, 0),           /*  5 */

  /* load counter */
  LOAD(1,0),            /*  6 */

  /* lbl: loop0 */
  ADD(1,2),             /*  7 */

  /* test for fizz+buzz */
  MOVE(6,1),            /*  8 */
  MOD(6,5),             /*  9 */
  CJMP(6,9),            /* 10 */

  /* test for fizz */
  MOVE(6,1),            /* 11 */
  MOD(6,3),             /* 12 */
  CJMP(6,8),            /* 13 */

  /* test for buzz */
  MOVE(6,1),            /* 14 */
  MOD(6,4),             /* 15 */
  CJMP(6,7),            /* 16 */

  PRINTNUM(1),          /* 17 */
  CJMP(7, 6),           /* 18 */

  /* lbl: fizzbuzz */
  PRINTSTR(0),          /* 19 */
  CJMP(7, 4),           /* 20 */

  /* lbl: fizz */
  PRINTSTR(1),          /* 21 */
  CJMP(7, 2),           /* 22 */

  /* lbl: buzz */
  PRINTSTR(2),          /* 23 */
  /* fall through */

  /* lbl: loop1 */
  LOAD(6, '\n'),        /* 24 */
  PRINTCHAR(6),         /* 25 */

  LOAD(6, -100),        /* 26 */
  ADD(6,1),             /* 27 */
  CJMP(6, 3),           /* 28 */

  LOAD(6, 0),           /* 29 */
  CJMP(6, -23),         /* 30 */

  LOAD(1, 0),           /* 31 */
  /* STOP */
};

const char *strings[] = {
  "FizzBuzz",
  "Fizz",
  "Buzz"
};

int runvm(unsigned short *instr, size_t num_instr, const char *strings[], size_t num_strings)
{
  size_t i;
  int reg[8] = { 0 };

  i = 0;
restart:
  for(; i < num_instr; i++) {
    int ci, op, r1, r2, arg;

    ci = instr[i];
    op = ci & 0x7;

    /*
    fprintf(stderr, "[%3zu] %2x 0x%4x\n", i, op, ci >> 3);
    fprintf(stderr, "registers:\n");
    for (int j=0; j < 8; j++) {
      fprintf(stderr, "    r%1d   %d\n", j+1, reg[j]);
    }

    {
      char buf[256];
      fgets(buf, sizeof buf, stdin);
    }
    */

    switch (op)
    {
      case OP_NOP:
        break;

      case OP_LOAD:
        r1 = (ci >> 3) & 0x7;
        arg = (ci >> 6) - 512;
        reg[r1] = arg;
        break;

      case OP_MOVE:
        r1 = (ci >> 3) & 0x7;
        r2 = (ci >> 6) & 0x7;
        reg[r1] = reg[r2];
        break;

      case OP_ADD:
        r1 = (ci >> 3) & 0x7;
        r2 = (ci >> 6) & 0x7;
        reg[r1] += reg[r2];
        break;

      case OP_MOD:
        r1 = (ci >> 3) & 0x7;
        r2 = (ci >> 6) & 0x7;
        reg[r1] = reg[r1] % reg[r2];
        break;

      case OP_CJMP:
        r1 = (ci >> 3) & 0x7;
        arg = (ci >> 6) - 512;
        if (reg[r1] == 0) {
          i += arg;
          goto restart;
        }
        break;

      case OP_PRINTCHAR:
        arg = ci >> 3;
        if (arg < 8) {
          putchar(reg[arg]);
        } else {
          arg -= 8;
          assert(arg >= 0 && (size_t)arg < num_strings);
          fputs(strings[arg],stdout);
        }
        break;

      case OP_PRINTNUM:
        arg = (ci >> 3) & 0x7;
        printf("%d",reg[arg]);
        break;

      default:
        fprintf(stderr, "unknown opcode, aborting.\n");
        abort();
    }

  }

  return reg[0];
}

int main(void)
{
  return runvm(fizzbuzz, sizeof fizzbuzz / sizeof fizzbuzz[0], strings, sizeof strings / sizeof strings[0]);
}







