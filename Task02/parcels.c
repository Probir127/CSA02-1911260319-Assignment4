#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "../common/date.h"
typedef struct { char city[32],pin[7]; } Address;
typedef struct { char name[64];Address address; } Person;
typedef enum { NORMAL,EXPRESS } Service;
typedef struct { unsigned id,weight_grams;Date booked;Person sender,receiver;Service service;long long charge_paise; } Parcel;
typedef struct {
    unsigned id,weight_grams;int day,month,year;
    char sender_name[64],sender_city[32],sender_pin[7];
    char receiver_name[64],receiver_city[32],receiver_pin[7];
    Service service;long long charge_paise;
} FlatParcel;
struct simple_address {char city[20];int pincode;};
struct simple_parcel {int id;float weight;struct simple_address to;Date booked;};
int main(void){
    Parcel bookings[100]={0};size_t count=3;
    const char *cities[]={"Madurai","Chennai","Coimbatore"};
    const char *pins[]={"625001","600077","641001"};
    unsigned weights[]={4500,1200,2800};
    for(size_t i=0;i<count;i++){
        bookings[i].id=501+(unsigned)i;bookings[i].weight_grams=weights[i];
        bookings[i].booked=(Date){12+(i==2),10,2026};
        strcpy(bookings[i].sender.name,"Sender");strcpy(bookings[i].sender.address.city,"Chennai");
        strcpy(bookings[i].sender.address.pin,"600077");strcpy(bookings[i].receiver.name,"Receiver");
        strcpy(bookings[i].receiver.address.city,cities[i]);strcpy(bookings[i].receiver.address.pin,pins[i]);
        bookings[i].service=i==1?EXPRESS:NORMAL;bookings[i].charge_paise=10000+(long long)i*1000;
        assert(date_valid(bookings[i].booked));
    }
    unsigned comparisons=0,swaps=0;
    for(size_t i=0;i+1<count;i++){
        size_t lightest=i;
        for(size_t j=i+1;j<count;j++){++comparisons;if(bookings[j].weight_grams<bookings[lightest].weight_grams)lightest=j;}
        if(lightest!=i){Parcel t=bookings[i];bookings[i]=bookings[lightest];bookings[lightest]=t;++swaps;}
    }
    assert(comparisons==3&&swaps==2&&bookings[0].id==502&&bookings[2].id==501);
    printf("Rich parcel size=%zu; flat size=%zu; comparisons=%u swaps=%u\n",sizeof(Parcel),sizeof(FlatParcel),comparisons,swaps);
    printf("Supplied parcel size=%zu; logical bytes per swap=%zu; total two swaps=%zu\n",
           sizeof(struct simple_parcel),3*sizeof(struct simple_parcel),6*sizeof(struct simple_parcel));
    printf("Rich parcel logical bytes per swap=%zu\n",3*sizeof(Parcel));
    Parcel *p=bookings;
    printf("Third receiver PIN via dot=%s; via pointer=%s\n",bookings[2].receiver.address.pin,(p+2)->receiver.address.pin);
    puts("Receiver city filter: Chennai");unsigned matches=0;
    for(size_t i=0;i<count;i++)if(!strcmp(bookings[i].receiver.address.city,"Chennai")){
        printf("ID=%u weight=%u grams\n",bookings[i].id,bookings[i].weight_grams);++matches;
    }
    assert(matches==1);puts("PASS: whole-record sort and receiver-city filter.");return 0;
}
