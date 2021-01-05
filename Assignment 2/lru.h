#ifndef _LRU_
#define _LRU_

#include <stdio.h>
#include <stdbool.h>

#include "simulator.h"
#include "hashtable.h"



void LRU_algorithm(struct Table* ptable1, struct Table* ptable2, int page_num, char op, int new_pid, int reading);



#endif