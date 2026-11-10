#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
enum Kind { TEMP,HUM,STATUS };
union Value { float celsius;int percent;char status[8]; };
struct AllFields {float celsius;int percent;char status[8];};
struct TaggedAll {enum Kind type;struct AllFields value;};
struct Packet {enum Kind type;union Value value;};
static bool set_status(struct Packet *p,const char *s){
    if(!p||!s||strlen(s)>7)return false;
    *p=(struct Packet){.type=STATUS,.value.status={0}};strcpy(p->value.status,s);return true;
}
int main(void){
    printf("int=%zu float=%zu enum=%zu pointer=%zu\n",sizeof(int),sizeof(float),sizeof(enum Kind),sizeof(void*));
    printf("all-fields=%zu tagged-all=%zu union=%zu tagged-union=%zu\n",sizeof(struct AllFields),sizeof(struct TaggedAll),sizeof(union Value),sizeof(struct Packet));
    printf("union alignment=%zu packet alignment=%zu type offset=%zu value offset=%zu\n",
           _Alignof(union Value),_Alignof(struct Packet),offsetof(struct Packet,type),offsetof(struct Packet,value));
    printf("10000 packets: all-fields=%zu tagged-all=%zu tagged-union=%zu bytes\n",
           10000*sizeof(struct AllFields),10000*sizeof(struct TaggedAll),10000*sizeof(struct Packet));
    struct Packet p;assert(set_status(&p,"ABCDEFG"));assert(!set_status(&p,"ABCDEFGH"));
    assert(!strcmp(p.value.status,"ABCDEFG"));
    union Value misuse={.celsius=31.5f};
    printf("Misuse experiment on this platform: stored temperature=%.1f; humidity member=%d\n",misuse.celsius,misuse.percent);
    const unsigned char *bytes=(const unsigned char*)&misuse.celsius;
    printf("Float object bytes:");for(size_t i=0;i<sizeof(float);i++)printf(" %02x",(unsigned)bytes[i]);putchar('\n');
    puts("Inactive-member result is representation-dependent, not a humidity conversion.");
    puts("PASS: status length7 accepted; length8 rejected without changing packet.");return 0;
}
