#ifndef VMSCHEME_DEBUG_H
#define VMSCHEME_DEBUG_H

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

#endif // VMSCHEME_DEBUG_H
