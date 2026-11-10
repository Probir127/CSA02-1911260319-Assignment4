#include <stdio.h>
struct date    { int day, month, year; };
struct address { char city[20]; int pincode; };
struct parcel  { int id; float weight; struct address to; struct date booked; };
void print_parcel(const struct parcel *p)
{
    // TODO 1: Print id, weight (1 decimal), city, pincode and booking date (dd/mm/yyyy)
    //   using the -> operator
    printf("%d  %.1f kg  %s-%d  %02d/%02d/%04d\n",p->id,p->weight,p->to.city,p->to.pincode,
           p->booked.day,p->booked.month,p->booked.year);
}
/* Selection sort of the parcels in ascending order of weight. */
void sort_by_weight(struct parcel p[], int n)
{
    // TODO 2: For each position i, find the lightest parcel among i..n-1 and swap the
    //   whole structure into position i
    for(int i=0;i<n-1;i++){
        int lightest=i;
        for(int j=i+1;j<n;j++)if(p[j].weight<p[lightest].weight)lightest=j;
        if(lightest!=i){struct parcel temporary=p[i];p[i]=p[lightest];p[lightest]=temporary;}
    }
}
int main(void)
{
    struct parcel list[3] = {
        {501, 4.5f, {"Madurai", 625001}, {12, 10, 2026}},
        {502, 1.2f, {"Chennai", 600077}, {12, 10, 2026}},
        {503, 2.8f, {"Coimbatore", 641001}, {13, 10, 2026}}
    };
    sort_by_weight(list, 3);
    for (int i = 0; i < 3; i++) print_parcel(&list[i]);
    return 0;
}
