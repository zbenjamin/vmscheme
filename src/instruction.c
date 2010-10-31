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
disassemble_wrap(struct object *proc)
{
  if (proc->type->code != PROCEDURE_TYPE) {
    printf("Wrong type for disassemble: %s\n", proc->type->name);
    exit(1);
  }

  disassemble(container_of(proc, struct instruction, obj));
  return UNSPECIFIC;
}

void
disassemble(struct instruction *stream)
{
  struct instruction *ins = stream;
  while (1) {
    printf("  %p  %02x ", ins, ins->op);
    switch (ins->op) {
    case PUSH:
      printf("PUSH\t %p ", ins->arg);
      print_obj(ins->arg);
      printf("\n");
      break;
    case POP:
      printf("POP\n");
      break;
    case LOOKUP:
      printf("LOOKUP\t %p ", ins->arg);
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
      return;
      break;
    case DEFINE:
      printf("DEFINE\t %p ", ins->arg);
      print_obj(ins->arg);
      printf("\n");
      break;
    case SET:
      printf("SET\t %p ", ins->arg);
      print_obj(ins->arg);
      printf("\n");
      break;
    case LAMBDA:
      printf("LAMBDA\t %p\n", ins->arg);
      break;
    case IF:
      printf("IF\n");
      break;
    case TAILIF:
      printf("TAILIF\n");
      break;
    default:
      printf("\nUnknown opcode: %d\n", ins->op);
      exit(1);
    }
    ++ins;
  }
}
