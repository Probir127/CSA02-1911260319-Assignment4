#include "demo_data.h"
#include <assert.h>
void setup_demo(Inventory *inv){
    *inv=(Inventory){.business_day={10,11,2026}};
    Item water={.id=1,.name="Water",.price_paise=2000,.stock=10,.kind=PACKAGED,.data.packaged={{12,11,2026}}};
    Item snack={.id=2,.name="Snack",.price_paise=3000,.stock=5,.kind=PACKAGED,.data.packaged={{9,11,2026}}};
    Item cooked={.id=3,.name="Idli",.price_paise=4000,.stock=8,.kind=COOKED,.data.cooked={{10,11,2026},{8,0}}};
    Item combo={.id=4,.name="Breakfast",.price_paise=5500,.kind=COMBO,.data.combo={2,{{1,1},{3,2}}}};
    Item milk={.id=5,.name="Milk",.price_paise=2500,.stock=4,.kind=PACKAGED,.data.packaged={{11,11,2026}}};
    assert(add_item(inv,&water)&&add_item(inv,&snack)&&add_item(inv,&cooked)&&add_item(inv,&combo)&&add_item(inv,&milk));
}
