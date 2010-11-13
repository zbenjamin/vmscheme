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

  struct vm_context *global_ctx = make_vm_context(NULL, NULL,
                                                  global_env);
  INC_REF(&global_ctx->obj);
  struct vm_context **pctx = &global_ctx;
  struct object *value;
  value = load("prelude.scm", pctx);
  YIELD_OBJ(value);

  init_compiler();

  value = load("stage2.scm", pctx);
  YIELD_OBJ(value);

  struct vm_context *repl_ctx;
  repl_ctx = make_vm_context(NULL, make_stack(1024),
                             make_environment(global_env));
  INC_REF(&repl_ctx->obj);
  pctx = &repl_ctx;

  struct object *ret = env_lookup(global_env, "initial-repl");
  assert(ret->type->code == PROCEDURE_TYPE);
  struct procedure *repl = container_of(ret, struct procedure, obj);
  apply_and_run(repl, NIL, pctx);

  return 0;
}

