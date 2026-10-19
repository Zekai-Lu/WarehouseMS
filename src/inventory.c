#include "inventory.h"

#include <limits.h>
#include <string.h>
#include <math.h>

static int valid_text(const char *s, size_t capacity)
{
    if (s == NULL || s[0] == '\0') {
        return 0;
    }

    if (strchr(s, ',') != NULL ||
        strchr(s, '\n') != NULL ||
        strchr(s, '\r') != NULL) {
        return 0;
    }

    return strlen(s) < capacity;
}

static int valid_product(const Product *p)
{
    if (p == NULL) {
        return 0;
    }

    if (!valid_text(p->id, ID_LEN) ||
        !valid_text(p->name, NAME_LEN) ||
        !valid_text(p->category, CATEGORY_LEN) ||
        !valid_text(p->unit, UNIT_LEN)) {
        return 0;
    }

    if (p->quantity < 0 || p->min_quantity < 0) {
        return 0;
    }

    if (!isfinite(p->unit_price) || p->unit_price < 0.0) {
        return 0;
    }

    return 1;
}

void inventory_init(Inventory *inv)
{
    if (inv != NULL) {
        memset(inv, 0, sizeof(*inv));
    }
}

int inventory_find(const Inventory *inv, const char *id)
{
    size_t i;

    if (inv == NULL || id == NULL) {
        return -1;
    }

    for (i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].id, id) == 0) {
            return (int)i;
        }
    }

    return -1;
}

int inventory_add(Inventory *inv, const Product *p)
{
    if (inv == NULL || !valid_product(p)) {
        return 0;
    }

    if (inv->count >= MAX_PRODUCTS ||
        inventory_find(inv, p->id) >= 0) {
        return 0;
    }

    inv->items[inv->count++] = *p;
    return 1;
}

int inventory_update(Inventory *inv, const char *id,
                     const Product *updated)
{
    int index;

    if (inv == NULL || id == NULL || updated == NULL ||
        !valid_product(updated)) {
        return 0;
    }

    index = inventory_find(inv, id);
    if (index < 0) {
        return 0;
    }

    /* 修改商品时不允许改变商品编号 */
    if (strcmp(id, updated->id) != 0) {
        return 0;
    }

    inv->items[index] = *updated;
    return 1;
}

int inventory_delete(Inventory *inv, const char *id)
{
    int index;
    size_t i;

    if (inv == NULL || id == NULL) {
        return 0;
    }

    index = inventory_find(inv, id);
    if (index < 0) {
        return 0;
    }

    for (i = (size_t)index; i + 1 < inv->count; i++) {
        inv->items[i] = inv->items[i + 1];
    }

    inv->count--;
    return 1;
}

int inventory_stock_in(Inventory *inv, const char *id, int amount)
{
    int index;

    if (inv == NULL || amount <= 0) {
        return 0;
    }

    index = inventory_find(inv, id);
    if (index < 0) {
        return 0;
    }

    if (inv->items[index].quantity > INT_MAX - amount) {
        return 0;
    }

    inv->items[index].quantity += amount;
    return 1;
}

int inventory_stock_out(Inventory *inv, const char *id, int amount)
{
    int index;

    if (inv == NULL || amount <= 0) {
        return 0;
    }

    index = inventory_find(inv, id);
    if (index < 0) {
        return 0;
    }

    if (inv->items[index].quantity < amount) {
        return 0;
    }

    inv->items[index].quantity -= amount;
    return 1;
}

double inventory_total_value(const Inventory *inv)
{
    double total = 0.0;
    size_t i;

    if (inv == NULL) {
        return 0.0;
    }

    for (i = 0; i < inv->count; i++) {
        total += inv->items[i].unit_price *
                 inv->items[i].quantity;
    }

    return total;
}

int inventory_low_stock_count(const Inventory *inv)
{
    int count = 0;
    size_t i;

    if (inv == NULL) {
        return 0;
    }

    for (i = 0; i < inv->count; i++) {
        if (inv->items[i].quantity <= inv->items[i].min_quantity) {
            count++;
        }
    }

    return count;
}