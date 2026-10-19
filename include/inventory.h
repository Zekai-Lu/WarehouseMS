#ifndef INVENTORY_H
#define INVENTORY_H

#include <stddef.h>

#define MAX_PRODUCTS 500
#define ID_LEN 20
#define NAME_LEN 64
#define CATEGORY_LEN 32
#define UNIT_LEN 16

typedef struct {
    char id[ID_LEN];
    char name[NAME_LEN];
    char category[CATEGORY_LEN];
    char unit[UNIT_LEN];
    int quantity;
    int min_quantity;
    double unit_price;
} Product;

typedef struct {
    Product items[MAX_PRODUCTS];
    size_t count;
} Inventory;

void inventory_init(Inventory *inv);
int inventory_find(const Inventory *inv, const char *id);

int inventory_add(Inventory *inv, const Product *p);
int inventory_update(Inventory *inv, const char *id, const Product *updated);
int inventory_delete(Inventory *inv, const char *id);

int inventory_stock_in(Inventory *inv, const char *id, int amount);
int inventory_stock_out(Inventory *inv, const char *id, int amount);

double inventory_total_value(const Inventory *inv);
int inventory_low_stock_count(const Inventory *inv);

#endif