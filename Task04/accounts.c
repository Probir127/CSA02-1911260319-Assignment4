#include <stdio.h>
#include <assert.h>
struct account {int number;float balance;};
static void ineffective(struct account a,float rate,const struct account *original){
    printf("number=%d original=%p local-copy=%p different=%d\n",a.number,(const void*)original,(void*)&a,&a!=original);
    assert(&a!=original);a.balance+=a.balance*rate/100;
}
static void addInterest(struct account *a,float rate){a->balance+=a->balance*rate/100;}
static struct account returned(struct account a,float rate){a.balance+=a.balance*rate/100;return a;}
int main(void){
    struct account list[3]={{101,5000},{102,12000},{103,800}};
    const float before[]={5000,12000,800},after[]={5200,12480,832};
    for(int i=0;i<3;i++){ineffective(list[i],4,&list[i]);assert(list[i].balance==before[i]);}
    puts("After by-value: 5000.00 12000.00 800.00");
    for(int i=0;i<3;i++){
        struct account alternate=returned(list[i],4);addInterest(&list[i],4);
        assert(list[i].balance==after[i]&&alternate.balance==list[i].balance);
        printf("Pointer repair: %d %.2f; returned copy: %.2f\n",list[i].number,list[i].balance,alternate.balance);
    }
    printf("sizeof account=%zu pointer=%zu; conceptual two-copy payload=%zu\n",sizeof(struct account),sizeof(struct account*),2*sizeof(struct account));
    puts("PASS: original unchanged by value; pointer and return-copy repairs agree.");return 0;
}
