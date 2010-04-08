#include <debug.h>
#include <object.h>

void
set_debug_info(struct object *obj, struct debuginfo *dinfo)
{
  obj->dinfo = dinfo;
}
