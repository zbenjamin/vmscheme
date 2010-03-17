#include <stdio.h>
#include <stdlib.h>

#include <compiler.h>
#include <eval.h>
#include <opcode.h>
#include <environment.h>
#include <instruction.h>
#include <parser_aux.h>
#include <primitive_procedures.h>
#include <stack.h>
#include <type.h>

int
main(int argc, char* argv[])
{
  init_builtin_types();
  init_global_env();
  init_singleton_objects();
  init_primitive_procs();
  init_compiler();

  eval_sequence(parse_file("prelude.scm"), global_env);

  struct vm_context repl_ctx;
  repl_ctx.stk = make_stack(1024);
  repl_ctx.env = make_environment(global_env);
  repl_ctx.pc = NULL;

  struct object *repl = env_lookup(global_env, "initial-repl");
  apply_and_run(repl, NIL, &repl_ctx);

  return 0;
}

