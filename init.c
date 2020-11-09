#include "init.h"
#include "defs.h"
#include "ext2.h"

#include <stddef.h>
#include <stdint.h>

void init_late() {
    init_ext2();
}
