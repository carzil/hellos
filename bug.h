#pragma once

#define S1(x) #x
#define S2(x) S1(x)

#define BUG_ON(expr) if (expr) { panic(__FILE__ ":" S2(__LINE__) ": assetion '" #expr "' failed"); }
#define BUG_ON_NULL(expr) BUG_ON(expr == NULL)
