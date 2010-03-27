#include <instruction.h>
#include <object.h>

#include <stdio.h>
#include <stdlib.h>

void
dealloc_bytecode(struct instruction *stream)
{
  struct instruction *pc = stream;
  while (pc->op != END) {
    if (pc->arg) {
      DEC_REF(pc->arg);
    }
    ++pc;
  };

  free(stream);
}

struct object*
disassemble(struct object *proc)
{
  if (proc->type->code != PROCEDURE_TYPE) {
    printf("Wrong type for disassemble: %s\n", proc->type->name);
    exit(1);
  }

  struct instruction *ins = proc->proc_val->code->cval;
  while (1) {
    printf("  %02x ", ins->op);
    switch (ins->op) {
    case PUSH:
      printf("PUSH\t 0x%p ", ins->arg);
      print_obj(ins->arg);
      printf("\n");
      break;
    case POP:
      printf("POP\n");
      break;
    case LOOKUP:
      printf("LOOKUP\t 0x%p ", ins->arg);
      print_obj(ins->arg);
      printf("\n");
      break;
    case CALL:
      printf("CALL\n");
      break;
    case TAILCALL:
      printf("TAILCALL\n");
      break;
    case RET:
      printf("RET\n");
      return NIL;
      break;
    case DEFINE:
      printf("DEFINE\t 0x%p ", ins->arg);
      print_obj(ins->arg);
      printf("\n");
      break;
    case LAMBDA:
      printf("LAMBDA\t 0x%p\n", ins->arg);
      break;
    case IF:
      printf("IF\n");
      break;
    case TAILIF:
      printf("TAILIF\n");
      break;
    default:
      printf("Unknown opcode: %d\n", ins->op);
      exit(1);
    }
    ++ins;
  }
  return NIL;
}
