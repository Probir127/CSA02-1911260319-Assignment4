#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../common/date.h"
#define MAX_PATIENTS 100
typedef struct { unsigned systolic,diastolic; } BloodPressure;
typedef struct {
    unsigned id;char name[64];unsigned age;char phone[11];
    BloodPressure bp;Date visited;
} Patient;
typedef struct { Patient patients[MAX_PATIENTS];size_t count; } PatientRegister;
static bool valid(const Patient *p){
    if(!p||!p->id||!memchr(p->name,'\0',sizeof p->name)||!p->name[0]||p->age>130||
       !memchr(p->phone,'\0',sizeof p->phone)||strlen(p->phone)!=10||!date_valid(p->visited))return false;
    for(size_t i=0;i<10;i++)if(p->phone[i]<'0'||p->phone[i]>'9')return false;
    return p->bp.systolic>0&&p->bp.systolic<=300&&p->bp.diastolic>0&&
           p->bp.diastolic<=200&&p->bp.systolic>=p->bp.diastolic;
}
static bool addPatient(PatientRegister *r,const Patient *p){
    if(!r||r->count>=MAX_PATIENTS||!valid(p))return false;
    for(size_t i=0;i<r->count;i++)if(r->patients[i].id==p->id)return false;
    r->patients[r->count++]=*p;return true;
}
static void sort_age(PatientRegister *r){
    for(size_t i=1;i<r->count;i++){
        Patient key=r->patients[i];size_t j=i;
        while(j>0&&r->patients[j-1].age>key.age){r->patients[j]=r->patients[j-1];--j;}
        r->patients[j]=key;
    }
}
int main(void){
    PatientRegister r={0};
    Patient a={1,"Mina",42,"0123456789",{120,80},{10,11,2026}};
    Patient b={2,"Arun",19,"9876543210",{110,70},{9,11,2026}};
    assert(addPatient(&r,&a)&&addPatient(&r,&b));sort_age(&r);
    for(size_t i=0;i<r.count;i++){
        const Patient *p=&r.patients[i];
        printf("%u %s age=%u phone=%s BP=%u/%u date=%02d/%02d/%04d\n",p->id,p->name,p->age,p->phone,
               p->bp.systolic,p->bp.diastolic,p->visited.day,p->visited.month,p->visited.year);
    }
    assert(r.patients[0].id==2&&!strcmp(r.patients[0].phone,b.phone));
    size_t saved=r.count;assert(!addPatient(&r,&a));
    Patient bad=a;bad.id=3;strcpy(bad.phone,"123");assert(!addPatient(&r,&bad));
    bad=a;bad.id=3;bad.visited=(Date){29,2,2025};assert(!addPatient(&r,&bad));
    bad=a;bad.id=3;bad.bp=(BloodPressure){80,120};assert(!addPatient(&r,&bad));
    bad=a;bad.id=3;memset(bad.name,'x',sizeof bad.name);assert(!addPatient(&r,&bad));
    assert(r.count==saved);
    for(unsigned id=3;id<=100;id++){a.id=id;assert(addPatient(&r,&a));}
    a.id=101;assert(!addPatient(&r,&a)&&r.count==100);
    printf("sizeof Patient=%zu; 100 records=%zu bytes; count=%zu\n",sizeof(Patient),sizeof r.patients,r.count);
    printf("Patient offsets: id=%zu name=%zu age=%zu phone=%zu bp=%zu date=%zu\n",
           offsetof(Patient,id),offsetof(Patient,name),offsetof(Patient,age),offsetof(Patient,phone),offsetof(Patient,bp),offsetof(Patient,visited));
    puts("PASS: associated fields, invalid data, duplicate ID and capacity boundary.");return 0;
}
