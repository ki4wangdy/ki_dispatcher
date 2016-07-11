
#pragma once

#ifdef DEBUG
	#define ki_print(str)	\
		do{	\
		fprintf(stderr, str);	\
		} while (0)
#else
	#define ki_print(str)	
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