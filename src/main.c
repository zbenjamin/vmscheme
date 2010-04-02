#include <stdio.h>
#include <stdlib.h>

#include <compiler.h>
#include <environment.h>
#include <eval.h>
#include <instruction.h>
#include <load.h>
#include <opcode.h>
#include <primitive_procedures.h>
#include <stack.h>
#include <symbol.h>
#include <type.h>

int
main(int argc, char* argv[])
{
  init_symbol_table();
  init_builtin_types();
  init_global_env();
  init_singleton_objects();
  init_primitive_procs();

  struct vm_context global_ctx;
  global_ctx.env = global_env;
  struct object *value;
  value = load("prelude.scm", &global_ctx);
  YIELD_OBJ(value);

  init_compiler();

  value = load("stdmacro.scm", &global_ctx);
  YIELD_OBJ(value);

  struct vm_context repl_ctx;
  repl_ctx.stk = make_stack(1024);
  repl_ctx.env = make_environment(global_env);
  repl_ctx.pc = NULL;
  INC_REF(repl_ctx.env);

  struct object *repl = env_lookup(global_env, "initial-repl");
  apply_and_run(repl, NIL, &repl_ctx);

  return 0;
}

