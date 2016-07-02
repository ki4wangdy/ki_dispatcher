
#ifndef _TYPE_H_
#define _TYPE_H_

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define ki_error	-1
#define ki_ok		0

#define r_calloc	calloc
#define r_malloc	malloc
#define r_realloc	realloc
#define r_free		free

typedef struct module_manager_st* module_manager_t;
typedef struct module_st* module_t;
typedef struct config_st* config_t;

#endif

