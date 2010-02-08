#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <object.h>

static struct object* parse_seq(char **p, const char* end);
static struct object* parse_number(char **p, const char* end);
static struct object* parse_symbol(char **p, const char* end);

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
  struct object *rev = reverse_list(ret);
  // deal with refcounting here

  return rev;
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
    } else if (**p == '(') {
      ++(*p);
      ret = make_pair(parse_seq(p, end),
                      ret);
    } else if (**p == ')') {
      ++(*p);
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
parse_number(char **p, const char* end)
{
  long num = strtol(*p, p, 10);
  struct object *ret = make_integer(num);
  return ret;
}  

struct object*
parse_symbol(char **p, const char* end)
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
