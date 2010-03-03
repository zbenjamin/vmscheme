#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <object.h>
#include <primitive_procedures.h>

static struct object* parse_seq(char **p, const char* end);
static struct object* parse_number(char **p, const char* end);
static struct object* parse_symbol(char **p, const char* end);
static struct object* parse_boolean(char **p, const char* end);

struct object*
parse(char *str)
{
  const char *end;
  char **p;
  int len = strlen(str);
  p = &str;
  end = str + len;

  struct object *ret = NIL;
  while (*p != end) {
    if (isspace(**p)) {
      ++(*p);
    } else if (isdigit(**p)) {
      ret = make_pair(parse_number(p, end),
                      ret);
    } else if (**p == '#') {
      ret = make_pair(parse_boolean(p, end),
                      ret);
    } else if (**p == '(') {
      ++(*p);
      ret = make_pair(parse_seq(p, end),
                      ret);
    } else if (**p == ')') {
      printf("Mismatched parenthesis\n");
      exit(1);
    } else {
      ret = make_pair(parse_symbol(p, end),
                      ret);
    }
  }
  struct object *tmp = reverse_list(ret);
  DEC_REF(ret);
  ret = tmp;
  return ret;
}

struct object*
parse_seq(char **p, const char* end)
{
  struct object *ret = NIL;
  while (*p != end) {
    if (isspace(**p)) {
      ++(*p);
    } else if (isdigit(**p)) {
      ret = make_pair(parse_number(p, end),
                      ret);
    } else if (**p == '#') {
      ret = make_pair(parse_boolean(p, end),
                      ret);
    } else if (**p == '(') {
      ++(*p);
      ret = make_pair(parse_seq(p, end),
                      ret);
    } else if (**p == ')') {
      ++(*p);
      struct object *tmp = reverse_list(ret);
      DEC_REF(ret);
      ret = tmp;
      return ret;
    } else {
      ret = make_pair(parse_symbol(p, end),
                      ret);
    }
  }
  printf("error: encountered end of input while expecting a ')'\n");
  exit(1);
}

struct object*
parse_number(char **p, const char *end)
{
  long num = strtol(*p, p, 10);
  struct object *ret = make_integer(num);
  return ret;
}

struct object*
parse_boolean(char **p, const char *end)
{
  ++(*p);
  char c = **p;
  ++(*p);
  if (*p != end
      && ! isspace(**p)
      && **p != '('
      && **p != ')') {
    printf("Parse error: bad #-token\n");
    exit(1);
  }

  if (c == 't' || c == 'T') {
    return TRUE;
  } else if (c == 'f'|| c == 'F') {
    return FALSE;
  }

  printf("Parse error: bad #-token\n");
  exit(1);
}

struct object*
parse_symbol(char **p, const char *end)
{
  char* start = *p;
  while (*p != end
         && ! isspace(**p)
         && **p != '('
         && **p != ')') {
    ++(*p);
  }

  int len = *p - start;
  char *sym = malloc(sizeof(char) * (len + 1));
  memcpy(sym, start, len);
  sym[len] = '\0';
  return make_symbol(sym);
}

struct object*
parse_file(const char *filename)
{
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    int error = errno;
    printf("Could not load '%s': %s\n", filename, strerror(error));
    exit(1);
  }

  ssize_t bufsize = 1024;
  char *buf = malloc(sizeof(char) * bufsize);
  char *pos = buf;
  ssize_t offset = pos - buf;
  size_t nbytes = 1023;
  ssize_t bytes_read = 0;
  while ((bytes_read = read(fd, pos, nbytes)) != 0) {
    pos += bytes_read;
    offset = pos - buf;
    if (offset + nbytes >= bufsize) {
      char *new_buf = malloc(sizeof(char) * bufsize * 2);
      memcpy(new_buf, buf, bufsize);
      bufsize *= 2;
      free(buf);
      buf = new_buf;
      pos = new_buf + offset;
    }
  }
  *pos = '\0';

  return parse(buf);
}
