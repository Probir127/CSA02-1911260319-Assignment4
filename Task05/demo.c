#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include "counters.h"
static int automatic_token(void){auto int token=1000;return ++token;}
static int persistent_token(void){static int token=1000;return ++token;}
static int register_token(void){register int token=1000;return ++token;}
static int static_pair(unsigned kind){static int tokens[2]={1000,1000};return kind<2?++tokens[kind]:-1;}
static int automatic_pair(unsigned kind){int tokens[2]={1000,1000};return kind<2?++tokens[kind]:-1;}
static int args_token(void){static int token=1000;return ++token;}
static bool caller_token(int *counter,int *out){
    if(!counter||!out||*counter==INT_MAX)return false;
    *out=++*counter;return true;
}
int main(void){
    printf("automatic:");for(int i=0;i<3;i++)printf(" %d",automatic_token());putchar('\n');
    printf("static:");for(int i=0;i<3;i++)printf(" %d",persistent_token());putchar('\n');
    int a=external_token(),b=other_source_token(),c=external_token();
    printf("extern across two callers: %d %d %d\n",a,b,c);assert(a==1001&&b==1002&&c==1003);
    printf("register-local:");for(register int i=0;i<3;i++)printf(" %d",register_token());putchar('\n');
    printf("static pair:");for(unsigned i=0;i<4;i++)printf(" %d",static_pair(i%2));putchar('\n');
    printf("extern pair:");for(unsigned i=0;i<4;i++)printf(" %d",external_pair(i%2));putchar('\n');
    printf("automatic pair still resets:");for(unsigned i=0;i<4;i++)printf(" %d",automatic_pair(i%2));putchar('\n');
    int counters[2]={1000,1000};
    printf("caller-owned pair with register loop:");
    for(register unsigned i=0;i<4;i++){int value;assert(caller_token(&counters[i%2],&value));printf(" %d",value);}
    putchar('\n');assert(counters[0]==1002&&counters[1]==1002);
    printf("single printf argument-order experiment: %d %d %d\n",args_token(),args_token(),args_token());
    int limit=INT_MAX,out=0;assert(!caller_token(&limit,&out)&&limit==INT_MAX&&out==0);
    assert(!caller_token(NULL,&out));puts("PASS: independent caller state and overflow rejection.");return 0;
}
