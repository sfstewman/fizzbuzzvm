#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* FizzBuzz VM: a VM for FizzBuzz
 *
 * Blame Nilium on #go-nuts for instigating this
 */

/* Each opcode is an unsigned short, which should correspond to at least 16 bits.
 *
 * The first four bits indicate the opcode.  The remaining bits are used
 * as an optional argument.
 *
 * The optional argument is interpreted as a signed value, and is
 * decoded as:  (opcode >> 4) - 2048
 */

enum OPCODES {
  OP_PUSH          =  0,   /* PUSH imm      pushes imm value onto the stack */

  OP_DUP           =  1,   /* DUP           duplicates the top of the stack */

  OP_ADD           =  2,   /* ADD imm       if imm==0, pops the top two values,
                                            adds them, and pushes them onto the stack.
                                            If imm != 0, pops the top value, adds
                                            imm to it and pushes the
                                            result onto the stack. */

  OP_REMAINDER     =  3,   /* REMAINDER     Pops top-1 and top, divides top-1 by top,
                                            pushes the remainder onto the stack */

  OP_CJMP          =  4,   /* CJMP rel      if rel == 0, this is a nop.
                                            otherwise, pops the top and
                                            compares it with zero.  If
                                            the value is zero, jump by
                                            the signed relative offset.
                                            Otherwise, go to the next
                                            instruction.

                                            NB: if rel == 1, this is a
                                            POP.
                                           */

  OP_PRINTCHAR     =  5,   /* PRINTCHAR imm 
                                            if imm == 0, pops the top of the stack
                                            and prints a character,
                                            else print string constant imm */

  OP_PRINTNUM      =  6,   /* PRINTNUM      prints the top of the stack as
                                            a number, popping it */

  OP_STOP          =  7,   /* STOP          Stops execution */
};

/* some convenience macros for encoding */
#define OPARG(op,arg) (((unsigned short)(2048 + arg) << 4) | op)
#define PUSHIMM(imm)  OPARG(OP_PUSH, imm)
#define CJMP(rel)     OPARG(OP_CJMP, rel)
#define ADD(imm)      OPARG(OP_ADD, imm)
#define PRINTCHAR(imm) OPARG(OP_PRINTCHAR, imm)

static unsigned short fizzbuzz[] = {
  /* op                   op count */

  /* START: */
  PUSHIMM(0),           /*  0 */

  /* LOOP: */
  ADD(1),               /*  1 */

  /* test for FizzBuzz */
  OP_DUP,               /*  2 */
  PUSHIMM(15),          /*  3 */
  OP_REMAINDER,         /*  4 */
  CJMP(13),             /*  5 */

  /* test for Fizz */
  OP_DUP,               /*  6 */
  PUSHIMM(3),           /*  7 */
  OP_REMAINDER,         /*  8 */
  CJMP(12),             /*  9 */

  /* test for Buzz */
  OP_DUP,               /* 10 */
  PUSHIMM(5),           /* 11 */
  OP_REMAINDER,         /* 12 */
  CJMP(11),             /* 13 */

  /* print number, go to loop end */
  OP_DUP,               /* 14 */
  OP_PRINTNUM,          /* 15 */
  PUSHIMM(0),           /* 16 */
  CJMP(8),              /* 17 */

  /* handle FizzBuzz case, go to loop end */
  PRINTCHAR(3),         /* 18. imm == 3: 'FizzBuzz' */
  PUSHIMM(0),           /* 19. unconditional jump */
  CJMP(5),              /* 20. to loop condition */

  /* handle Fizz case, go to loop end */
  PRINTCHAR(1),         /* 21. imm == 1: 'Fizz' */
  PUSHIMM(0),           /* 22. unconditional jump */
  CJMP(2),              /* 23. to loop condition */

  /* handle Buzz case */
  PRINTCHAR(2),         /* 24. imm == 1: 'Buzz' */

  /* Loop end, print newline, check end */
  PUSHIMM('\n'),        /* 25 */
  PRINTCHAR(0),         /* 26 */
  OP_DUP,               /* 27 */
  ADD(-100),            /* 28 */
  CJMP(3),              /* 29 */

  /* repeat the loop */
  PUSHIMM(0),           /* 30 */
  CJMP(-30),            /* 31 */

  PUSHIMM(0),           /* 32 */

  /* STOP */
};

const char *strings[] = {
  "Fizz",
  "Buzz",
  "FizzBuzz"
};

int runvm(unsigned short *instr, size_t num_instr, const char *strings[], size_t num_strings)
{
  size_t i;
  int stack[16], top=0;
  const int max_stack = sizeof stack / sizeof stack[0];

  i = 0;
restart:
  for(; i < num_instr; i++) {
    int op,arg;

    op = instr[i] & 0xf;
    arg = (instr[i] >> 4) - 2048;

    /*
    fprintf(stderr, "[%3zu] %2x %5d\n", i, op, arg);
    fprintf(stderr, "stack:\n");
    for (int j=0; j < top; j++) {
      fprintf(stderr, "    - %2d -   %d\n", j, stack[j]);
    }

    {
      char buf[256];
      fgets(buf, sizeof buf, stdin);
    }
    */

    switch (op)
    {
      case OP_PUSH:
        assert(top < max_stack);
        stack[top++] = arg;
        break;

      case OP_DUP:
        assert(top > 0 && top < max_stack);
        stack[top] = stack[top-1];
        top++;
        break;

      case OP_ADD:
        if (arg == 0) {
          assert(top > 1);
          stack[top-2] += stack[top-1];
          top--;
        } else {
          assert(top > 0);
          stack[top-1] += arg;
        }
        break;

      case OP_REMAINDER:
        assert(top > 1);
        stack[top-2] = stack[top-2] % stack[top-1];
        top--;
        break;

      case OP_CJMP:
        if (arg != 0) {
          assert(top > 0);
          if (stack[--top] == 0) {
            i += arg;
            goto restart;
          }
        }
        break;

      case OP_PRINTCHAR:
        if (arg == 0) {
          assert(top > 0);
          putchar(stack[--top]);
        } else {
          assert(arg > 0 && (size_t)arg <= num_strings);
          fputs(strings[arg-1],stdout);
        }
        break;
        
      case OP_PRINTNUM:
        assert(top > 0);
        printf("%d", stack[--top]);
        break;

      case OP_STOP:
        goto end;

      default:
        fprintf(stderr, "unknown opcode, aborting.\n");
        abort();
    }
  }

end:
  return (top > 0) ? stack[top-1] : 0;
}

int main(void)
{
  return runvm(fizzbuzz, sizeof fizzbuzz / sizeof fizzbuzz[0], strings, sizeof strings / sizeof strings[0]);
}







