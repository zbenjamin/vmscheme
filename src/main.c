#include <stdio.h>
#include <stdlib.h>

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

  eval_sequence(parse_file("prelude.scm"), global_env);

  while (1) {
    printf("> ");
    struct object *input = parse_interactive();
    if (input == NULL) {
      break;
    }
    print_obj(input);
    printf("\n");

    struct object *value;
    value = eval_sequence(input, global_env);
    print_obj(value);
    printf("\n");
    DEC_REF(value);

    DEC_REF(input);
  }
  return 0;
}

