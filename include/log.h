
#pragma once

#ifdef DEBUG
	#define ki_print(str)	\
	do{	\
		fprintf(stderr,str);	\
	} while (0)
#else
	#define ki_print(str)	
#endif 


#ifdef DEBUG
	#define ki_print(str , arg)	\
		do{	\
		fprintf(stderr, str, arg);	\
		} while (0)
#else
	#define ki_print(str)	
#endif 

#ifdef DEBUG
	#define ki_log(b,str, arg)	\
	do{	\
	if(b){	\
		fprintf(stderr, str, arg);	\
		assert(0);	\
	}	\
	} while (0)
#else
	#define ki_log(str)
#endif

#ifdef DEBUG
	#define ki_log(b,str)	\
	do{	\
		if(b){	\
			fprintf(stderr, str);	\
			assert(0);	\
		}	\
	} while (0)
#else
	#define ki_log(str)
#endif