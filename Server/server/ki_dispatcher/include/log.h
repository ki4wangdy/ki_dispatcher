
#pragma once

#ifdef DEBUG
	#define ki_log(str)	\
	do{	\
		fprintf(stderr, str);	\
		assert(0);	\
	} while (0)
#else
	#define ki_log(str)
#endif