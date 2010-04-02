#include <load.h>

#include <parser_aux.h>
#include <stdio.h>
#include <stdlib.h>

struct object *
load(const char *filename, struct vm_context *ctx)
{
  struct object *forms = parse_file(filename);
  struct object *value = eval_sequence(forms, ctx->env);
  // ensures we don't double-free the return value
  INC_REF(value);
  dealloc_obj(forms);
  DEC_REF(value);

  return value;
}

struct object *
load_wrap(struct object *filename, struct vm_context *ctx)
{
  if (filename->type->code != STRING_TYPE) {
    printf("Wrong type for load: %s\n", filename->type->name);
    exit(1);
  }

  return load(filename->sval, ctx);
}
