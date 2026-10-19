#ifndef WAREHOUSE_STORAGE_H
#define WAREHOUSE_STORAGE_H

#include "inventory.h"

int storage_load(Inventory *inv, const char *path);
int storage_save(const Inventory *inv, const char *path);

#endif