/* Compile-only constraint violation: the address of a register object is unavailable. */
int main(void){register int i=0;int *p=&i;return *p;}
