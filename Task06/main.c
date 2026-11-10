#include "demo_data.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
static void sale(Inventory *inv,unsigned id,unsigned quantity,SaleResult expected){
    Inventory saved=*inv;
    SaleResult actual=record_sale(inv,id,quantity,inv->business_day,(Time){9,0});
    printf("Sale item=%u quantity=%u: %s\n",id,quantity,sale_message(actual));assert(actual==expected);
    if(actual!=SALE_OK)assert(memcmp(&saved,inv,sizeof saved)==0);
}
int main(void){
    Inventory inventory;setup_demo(&inventory);
    expiry_warnings(&inventory);
    sale(&inventory,1,2,SALE_OK);sale(&inventory,4,2,SALE_OK);
    sale(&inventory,3,1,SALE_OK);sale(&inventory,5,1,SALE_OK);
    sale(&inventory,4,2,INSUFFICIENT_STOCK);sale(&inventory,2,1,UNAVAILABLE);
    sale(&inventory,99,1,UNKNOWN_ITEM);sale(&inventory,1,0,BAD_REQUEST);
    assert(inventory.total_revenue==21500&&inventory.sale_count==4);
    assert(inventory.items[0].stock==6&&inventory.items[2].stock==3&&inventory.items[4].stock==3);
    print_summary(&inventory);
    printf("sizeof Date=%zu Time=%zu Item=%zu Sale=%zu Inventory=%zu\n",sizeof(Date),sizeof(Time),sizeof(Item),sizeof(Sale),sizeof(Inventory));
    printf("Item offsets: id=%zu name=%zu price=%zu stock=%zu kind=%zu union=%zu sold=%zu revenue=%zu\n",
           offsetof(Item,id),offsetof(Item,name),offsetof(Item,price_paise),offsetof(Item,stock),offsetof(Item,kind),offsetof(Item,data),offsetof(Item,sold_units),offsetof(Item,revenue_paise));
    puts("PASS: expected totals and stocks; every rejected sale left all inventory bytes unchanged.");return 0;
}
