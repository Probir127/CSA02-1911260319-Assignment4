#ifndef CSA02_COUNTERS_H
#define CSA02_COUNTERS_H
extern int shared_counter;
extern int shared_pair[2];
int external_token(void);
int other_source_token(void);
int external_pair(unsigned kind);
#endif
