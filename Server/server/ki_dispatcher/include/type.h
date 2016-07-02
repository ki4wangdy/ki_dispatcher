
#ifndef _TYPE_H_
#define _TYPE_H_

typedef char				int8_t;
typedef unsigned char		uint8_t;
typedef short				int16_t;
typedef unsigned short		uint16_t;
typedef int					int32_t;
typedef unsigned int		uint32_t;
typedef long long			int64_t;
typedef unsigned long long	uint64_t;

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

#endif

