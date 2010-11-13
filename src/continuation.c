#include <continuation.h>
#include <stack.h>
#include <stdlib.h>
#include <object.h>
#include <stdio.h>

static struct vm_context *clone_context(struct vm_context *ctx);

struct vm_context *
clone_context(struct vm_context *ctx)
{
  struct codeptr *new_pc = make_codeptr(ctx->pc->base,
                                        ctx->pc->offset);

  struct stack *new_stack = make_stack(ctx->stk->size);
  int i;
  for (i = 0; i < ctx->stk->top; ++i) {
    stack_push(new_stack, ctx->stk->elems[i]);
    if (ctx->stk->elems[i]) {
      INC_REF(ctx->stk->elems[i]);
    }
  }

  struct vm_context *new_ctx;
  new_ctx = make_vm_context(new_pc, new_stack, ctx->env);
  return new_ctx;
}  

void
dealloc_continuation(struct vm_context *cont)
{
  if (cont->pc) {
    DEC_REF(&cont->pc->obj);
  }
  dealloc_stack(cont->stk);
  DEC_REF(&cont->env->obj);
  free(cont);
}

struct object *
call_cc(struct object *proc, struct vm_context **ctx)
{
  if (proc->type->code != PROCEDURE_TYPE) {
    printf("Wrong type for call/cc: %s\n",
           proc->type->name);
    exit(1);
  }

  // this is an awful hack.  we're using the same call
  // instruction that invoked call_cc to invoke the passed procedure
  struct vm_context *new_ctx = clone_context(*ctx);
  INC_REF(&new_ctx->obj);

  struct vm_context *old_ctx = *ctx;
  *ctx = new_ctx;

  struct pair *args = make_pair(&old_ctx->obj, &NIL->obj);

  apply(container_of(proc, struct procedure, obj), args, ctx);
  YIELD_OBJ(&args->obj);

  return NULL;
}

struct object *
invoke_continuation(struct object *cont, struct object *value,
                    struct vm_context **ctx)
{
  if (cont->type->code != CONTINUATION_TYPE) {
    printf("Wrong type for invoke-continuation: %s\n",
           cont->type->name);
    exit(1);
  }

  struct vm_context *new_ctx;
  new_ctx = container_of(cont, struct vm_context, obj);

  struct vm_context *clone = clone_context(new_ctx);
  INC_REF(&clone->obj);

  /* XXX: */
  /* DEC_REF(&(*ctx)->obj); */
  *ctx = clone;

  return value;
}
