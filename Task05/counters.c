#include "counters.h"
int shared_counter=1000;
int shared_pair[2]={1000,1000};
int external_token(void){return ++shared_counter;}
int external_pair(unsigned kind){return kind<2?++shared_pair[kind]:-1;}
