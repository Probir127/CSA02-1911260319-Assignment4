#include "canteen.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
static int find_item(const Inventory *inv,unsigned id){
    for(size_t i=0;i<inv->count;i++)if(inv->items[i].id==id)return (int)i;
    return -1;
}
static bool valid_time(Time t){return t.hour<24&&t.minute<60;}
static unsigned minutes(Time t){return 60*t.hour+t.minute;}
static void money(long long p){printf("Rs %lld.%02lld",p/100,p%100);}
bool add_item(Inventory *inv,const Item *item){
    if(!inv||!item||inv->count>=MAX_ITEMS||!date_valid(inv->business_day)||!item->id||
       find_item(inv,item->id)>=0||!memchr(item->name,'\0',sizeof item->name)||!item->name[0]||
       item->price_paise<=0||item->price_paise>1000000000LL||item->stock>1000000||
       item->sold_units||item->revenue_paise)return false;
    switch(item->kind){
    case PACKAGED:if(!date_valid(item->data.packaged.expiry))return false;break;
    case COOKED:
        if(!date_valid(item->data.cooked.prepared_date)||!valid_time(item->data.cooked.prepared_time))return false;
        break;
    case COMBO:
        if(item->stock||item->data.combo.count==0||item->data.combo.count>MAX_PARTS)return false;
        for(size_t j=0;j<item->data.combo.count;j++){
            Part part=item->data.combo.parts[j];int at=find_item(inv,part.item_id);
            if(at<0||inv->items[at].kind==COMBO||!part.quantity||part.quantity>1000000)return false;
        }
        break;
    default:return false;
    }
    inv->items[inv->count++]=*item;return true;
}
static bool available(const Item *item,Date day,Time now){
    if(item->kind==PACKAGED)return date_ordinal(item->data.packaged.expiry)>=date_ordinal(day);
    if(item->kind==COOKED)return date_ordinal(item->data.cooked.prepared_date)==date_ordinal(day)&&
        minutes(item->data.cooked.prepared_time)<=minutes(now);
    return false;
}
SaleResult record_sale(Inventory *inv,unsigned id,unsigned quantity,Date day,Time time){
    if(!inv||!quantity||quantity>1000000||!date_valid(day)||!valid_time(time)||
       !date_valid(inv->business_day)||date_ordinal(day)!=date_ordinal(inv->business_day))return BAD_REQUEST;
    int at=find_item(inv,id);if(at<0)return UNKNOWN_ITEM;
    if(inv->sale_count>=MAX_SALES)return SALES_FULL;
    Item *sold=&inv->items[at];uint64_t needed[MAX_ITEMS]={0};
    if(sold->kind==COMBO){
        for(size_t j=0;j<sold->data.combo.count;j++){
            Part part=sold->data.combo.parts[j];int ingredient=find_item(inv,part.item_id);
            if(ingredient<0||inv->items[ingredient].kind==COMBO)return BAD_REQUEST;
            uint64_t amount=(uint64_t)quantity*part.quantity;
            if(needed[ingredient]>UINT64_MAX-amount)return AMOUNT_OVERFLOW;
            needed[ingredient]+=amount;
        }
    } else needed[at]=quantity;
    for(size_t i=0;i<inv->count;i++)if(needed[i]){
        if(!available(&inv->items[i],day,time))return UNAVAILABLE;
        if(needed[i]>inv->items[i].stock)return INSUFFICIENT_STOCK;
    }
    if(sold->price_paise>LLONG_MAX/quantity)return AMOUNT_OVERFLOW;
    long long charge=sold->price_paise*quantity;
    if(sold->revenue_paise>LLONG_MAX-charge||inv->total_revenue>LLONG_MAX-charge||
       sold->sold_units>ULLONG_MAX-quantity)return AMOUNT_OVERFLOW;
    /* Commit phase: every validation succeeded; nothing below can reject. */
    for(size_t i=0;i<inv->count;i++)inv->items[i].stock-=(unsigned)needed[i];
    sold->sold_units+=quantity;sold->revenue_paise+=charge;inv->total_revenue+=charge;
    inv->sales[inv->sale_count++]=(Sale){id,quantity,sold->price_paise,charge,day,time};
    return SALE_OK;
}
void expiry_warnings(const Inventory *inv){
    puts("Expiry warnings (inclusive today through two days ahead):");
    for(size_t i=0;i<inv->count;i++){
        const Item *p=&inv->items[i];if(p->kind!=PACKAGED)continue;
        int delta=date_ordinal(p->data.packaged.expiry)-date_ordinal(inv->business_day);
        if(delta<0)printf("EXPIRED: %u %s (%d days overdue)\n",p->id,p->name,-delta);
        else if(delta<=2)printf("EXPIRING: %u %s (in %d days)\n",p->id,p->name,delta);
    }
}
void print_summary(const Inventory *inv){
    size_t order[MAX_ITEMS];for(size_t i=0;i<inv->count;i++)order[i]=i;
    for(size_t i=1;i<inv->count;i++){
        size_t key=order[i],j=i;
        while(j>0){
            const Item *a=&inv->items[order[j-1]],*b=&inv->items[key];
            if(a->revenue_paise>b->revenue_paise||(a->revenue_paise==b->revenue_paise&&a->id<b->id))break;
            order[j]=order[j-1];--j;
        }
        order[j]=key;
    }
    puts("Daily summary: descending revenue; equal revenues ordered by item ID");
    for(size_t i=0;i<inv->count;i++){
        const Item *p=&inv->items[order[i]];
        printf("ID=%u %-12s sold=%llu revenue=",p->id,p->name,p->sold_units);money(p->revenue_paise);
        if(p->kind==COMBO)puts(" stock=derived from components");else printf(" stock=%u\n",p->stock);
    }
    printf("Sales=%zu total=",inv->sale_count);money(inv->total_revenue);putchar('\n');
}
const char *sale_message(SaleResult s){
    switch(s){case SALE_OK:return "accepted";case BAD_REQUEST:return "invalid request";
    case UNKNOWN_ITEM:return "unknown item";case INSUFFICIENT_STOCK:return "insufficient stock";
    case UNAVAILABLE:return "expired or not available at sale time";case SALES_FULL:return "sale log full";
    default:return "amount overflow";}
}
