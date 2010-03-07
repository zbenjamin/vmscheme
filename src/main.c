#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <eval.h>
#include <opcode.h>
#include <compiler.h>
#include <environment.h>
#include <instruction.h>
#include <parser_aux.h>
#include <primitive_procedures.h>
#include <stack.h>
#include <type.h>

static struct object* run_code(struct object *form,
                               struct stack *stk,
                               struct object *env);
int
main(int argc, char* argv[])
{
  init_global_env();
  init_builtin_types();
  init_singleton_objects();
  init_primitive_procs();

  struct stack *stk = make_stack(1024);

  /* run_code(parse_file("prelude.scm"), stk, global_env); */

  while (1) {
    printf("> ");
    struct object *input = parse_interactive();
    if (input == NULL) {
      break;
    }
    print_obj(input);
    printf("\n");

    struct object *value = run_code(input, stk, global_env);
    assert(stack_empty(stk));
    print_obj(value);
    printf("\n");
    DEC_REF(value);

    DEC_REF(input);
  }
  dealloc_stack(stk);
  return 0;
}

struct object *
run_code(struct object *form, struct stack *stk,
         struct object *env)
{
  struct instruction *prog = compile(form);
  // push magic "end of instructions" return address
  struct object *end_marker = make_code(NULL);
  end_marker->refcount = -1;
  push_stack(stk, end_marker);
  push_stack(stk, env);

  eval(prog, stk, env);
  dealloc_bytecode(prog);
  return pop_stack(stk);
}
