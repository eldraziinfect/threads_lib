#include <stdlib.h>

typedef struct s_TCB 
{	int tid;
	int state;
	int prio;
	context;
	void *data;
} TCB_t;