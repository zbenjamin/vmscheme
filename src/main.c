#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <eval.h>
#include <opcode.h>
#include <compiler.h>
#include <environment.h>
#include <instruction.h>
#include <parser.h>
#include <primitive_procedures.h>
#include <stack.h>
#include <type.h>

int
main(int argc, char* argv[])
{
  init_global_env();
  init_builtin_types();
  init_nil();
  init_primitive_procs();

  int ret;
  char *buf = NULL;
  size_t nchars;
  while (1) {
    printf("> ");
    ret = getline(&buf, &nchars, stdin);
    if (ret == -1) {
      break;
    }
    struct object *input = parse(buf);
    print_obj(input);
    printf("\n");

    struct instruction *prog = compile(input);
    struct stack *stk = make_stack(1024);
    struct object *value;
    eval(prog, stk, global_env);
    if (stack_empty(stk)) {
      value = NIL;
    } else {
      value = pop_stack(stk);
    }
    assert(stack_empty(stk));
    print_obj(value);
    printf("\n");
    DEC_REF(value);

    free(buf);
    dealloc_stack(stk);
    DEC_REF(input);
    dealloc_bytecode(prog);
    buf = NULL;
  }
  return 0;
}
