#ifndef _SEC_
#define _SEC_

#include <stdbool.h>
#include <stdio.h>

#include "hashtable.h"


void SECC_algorithm(struct Table* ptable1, struct Table* ptable2, int page_num, char op, int new_pid, int reading);


#endif