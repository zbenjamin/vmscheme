#ifndef VMSCHEME_DEBUG_H
#define VMSCHEME_DEBUG_H

#include <sys/types.h>

enum src_type {
  SRC_UNKNOWN,
  SRC_FILE,
  SRC_STDIN
};

struct debuginfo {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char *src_file;  // or the input string if src == REPL
  enum src_type src;
};

struct object;

void set_debug_info(struct object *obj, struct debuginfo *dinfo);
void debug_loc_str(struct object *obj, char *buf, size_t size);

#endif // VMSCHEME_DEBUG_H
