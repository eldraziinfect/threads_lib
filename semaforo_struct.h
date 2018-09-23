#include <stdlib.h>

typedef struct s_sem
{	int count;	//indica se o recurso está ocupado ou não (livre > 0, ocupado <= 0).
	PFILA2 file;	//ponteiro para uma fila de threads bloqueadas no semáforo.
} csem_t;