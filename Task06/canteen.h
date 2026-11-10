#ifndef CSA02_CANTEEN_H
#define CSA02_CANTEEN_H
#include <stddef.h>
#include <stdbool.h>
#include "../common/date.h"
#define MAX_ITEMS 32
#define MAX_SALES 128
#define MAX_PARTS 3
typedef struct {unsigned hour,minute;} Time;
typedef enum {PACKAGED,COOKED,COMBO} ItemKind;
typedef struct {unsigned item_id,quantity;} Part;
typedef struct {
    unsigned id;char name[40];long long price_paise;unsigned stock;
    ItemKind kind;
    union {
        struct {Date expiry;} packaged;
        struct {Date prepared_date;Time prepared_time;} cooked;
        struct {size_t count;Part parts[MAX_PARTS];} combo;
    } data;
    unsigned long long sold_units;long long revenue_paise;
} Item;
typedef struct {unsigned item_id,quantity;long long unit_price,total;Date day;Time time;} Sale;
typedef struct {
    Date business_day;Item items[MAX_ITEMS];size_t count;
    Sale sales[MAX_SALES];size_t sale_count;long long total_revenue;
} Inventory;
typedef enum {SALE_OK,BAD_REQUEST,UNKNOWN_ITEM,INSUFFICIENT_STOCK,UNAVAILABLE,SALES_FULL,AMOUNT_OVERFLOW} SaleResult;
bool add_item(Inventory *inventory,const Item *item);
SaleResult record_sale(Inventory *inventory,unsigned id,unsigned quantity,Date day,Time time);
void expiry_warnings(const Inventory *inventory);
void print_summary(const Inventory *inventory);
const char *sale_message(SaleResult result);
#endif
