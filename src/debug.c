#include <debug.h>
#include <object.h>

#include <stdio.h>
#include <string.h>

void
set_debug_info(struct object *obj, struct debuginfo *dinfo)
{
  obj->dinfo = dinfo;
}

void
debug_loc_str(struct object *obj, char *buf, size_t size)
{
  if (obj->dinfo == NULL || obj->dinfo->src != SRC_FILE) {
    strncpy(buf, "<unknown>", size - 1);
    buf[size - 1] = '\0';
    return;
  }

  snprintf(buf, size, "%s:%d", obj->dinfo->src_file,
           obj->dinfo->first_line);
  return;
}
