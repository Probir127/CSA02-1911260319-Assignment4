#include "demo_data.h"
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
static void rejected(Inventory *inv,unsigned id,unsigned q,Date day,Time time,SaleResult expected){
    Inventory saved=*inv;assert(record_sale(inv,id,q,day,time)==expected);
    assert(!memcmp(&saved,inv,sizeof saved));
}
int main(void){
    Inventory inv;setup_demo(&inv);Date day=inv.business_day;Time now={9,0};
    rejected(&inv,3,1,day,(Time){7,59},UNAVAILABLE);
    rejected(&inv,1,1,(Date){11,11,2026},now,BAD_REQUEST);
    rejected(&inv,1,1,day,(Time){24,0},BAD_REQUEST);
    rejected(&inv,1,1000001,day,now,BAD_REQUEST);
    inv.sale_count=MAX_SALES;rejected(&inv,1,1,day,now,SALES_FULL);inv.sale_count=0;
    inv.items[0].revenue_paise=LLONG_MAX-1;rejected(&inv,1,1,day,now,AMOUNT_OVERFLOW);inv.items[0].revenue_paise=0;
    inv.total_revenue=LLONG_MAX-1;rejected(&inv,1,1,day,now,AMOUNT_OVERFLOW);inv.total_revenue=0;
    Item double_water={.id=6,.name="DoubleWater",.price_paise=3500,.kind=COMBO,.data.combo={2,{{1,1},{1,2}}}};
    assert(add_item(&inv,&double_water));
    rejected(&inv,6,4,day,now,INSUFFICIENT_STOCK); /* repeated IDs need12, not merely4or8 */
    assert(record_sale(&inv,6,3,day,now)==SALE_OK&&inv.items[0].stock==1);
    assert(inv.total_revenue==10500&&inv.items[0].revenue_paise==0); /* no double revenue */
    Item bad=double_water;bad.id=7;bad.data.combo.parts[0].item_id=4;assert(!add_item(&inv,&bad));
    bad=double_water;bad.id=7;bad.data.combo.parts[0].item_id=99;assert(!add_item(&inv,&bad));
    bad=double_water;bad.id=7;bad.data.combo.parts[0].quantity=0;assert(!add_item(&inv,&bad));
    bad=double_water;bad.id=7;bad.data.combo.count=4;assert(!add_item(&inv,&bad));
    bad=double_water;bad.id=7;bad.kind=(ItemKind)99;assert(!add_item(&inv,&bad));
    assert(!add_item(&inv,&double_water));
    assert(date_valid((Date){29,2,2024})&&!date_valid((Date){29,2,2025}));
    assert(date_ordinal((Date){1,3,2024})-date_ordinal((Date){28,2,2024})==2);
    assert(date_ordinal((Date){1,1,2027})-date_ordinal((Date){30,12,2026})==2);
    setup_demo(&inv);inv.items[0].data.packaged.expiry=day;
    assert(record_sale(&inv,1,1,day,now)==SALE_OK); /* expiry today inclusive */
    inv.items[0].data.packaged.expiry=(Date){9,11,2026};rejected(&inv,1,1,day,now,UNAVAILABLE);
    setup_demo(&inv);inv.items[2].data.cooked.prepared_date=(Date){9,11,2026};
    rejected(&inv,4,1,day,now,UNAVAILABLE); /* invalid component invalidates entire combo */
    setup_demo(&inv);for(unsigned id=6;id<=MAX_ITEMS;id++){
        Item item=inv.items[0];item.id=id;assert(add_item(&inv,&item));
    }
    Item extra=inv.items[0];extra.id=MAX_ITEMS+1;assert(!add_item(&inv,&extra));
    puts("PASS: duplicate component demand aggregated; successful combo revenue counted once.");
    puts("PASS: rejected sale snapshots unchanged for stock, expiry, time, capacity and overflow.");
    puts("PASS: invalid recipes/tags/IDs and item capacity rejected; leap/year date boundaries verified.");
    puts("PASS: expiry today allowed; expired ingredient and previous-day cooked item rejected.");return 0;
}
