#include "inventory.h"
#include "warehouse_storage.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static Product make_product(const char *id, const char *name,
                            int quantity, int min_quantity,
                            double price)
{
    Product p = {0};

    snprintf(p.id, sizeof(p.id), "%s", id);
    snprintf(p.name, sizeof(p.name), "%s", name);
    snprintf(p.category, sizeof(p.category), "电子配件");
    snprintf(p.unit, sizeof(p.unit), "个");
    p.quantity = quantity;
    p.min_quantity = min_quantity;
    p.unit_price = price;

    return p;
}

int main(void)
{
    Inventory inv;
    Inventory loaded;
    Product p;
    Product updated;

    inventory_init(&inv);

    p = make_product("P001", "鼠标", 10, 3, 25.0);
    assert(inventory_add(&inv, &p) == 1);
    assert(inv.count == 1);

    /* 重复编号应被拒绝 */
    assert(inventory_add(&inv, &p) == 0);

    /* 非法数据应被拒绝 */
    p.quantity = -1;
    assert(inventory_add(&inv, &p) == 0);
    p.quantity = 10;

    assert(inventory_find(&inv, "P001") == 0);
    assert(inventory_find(&inv, "NOT-FOUND") == -1);

    assert(inventory_stock_in(&inv, "P001", 5) == 1);
    assert(inv.items[0].quantity == 15);

    assert(inventory_stock_out(&inv, "P001", 4) == 1);
    assert(inv.items[0].quantity == 11);

    /* 不能超出库存出库 */
    assert(inventory_stock_out(&inv, "P001", 99) == 0);
    assert(inv.items[0].quantity == 11);

    /* 变动数量必须大于零 */
    assert(inventory_stock_in(&inv, "P001", 0) == 0);
    assert(inventory_stock_out(&inv, "P001", -1) == 0);

    p = make_product("P002", "键盘", 2, 5, 100.0);
    assert(inventory_add(&inv, &p) == 1);
    assert(inventory_low_stock_count(&inv) == 1);

    assert(fabs(inventory_total_value(&inv) - 475.0) < 0.001);

    updated = make_product("P001", "无线鼠标", 11, 3, 30.0);
    assert(inventory_update(&inv, "P001", &updated) == 1);
    assert(strcmp(inv.items[0].name, "无线鼠标") == 0);

    /* 不允许通过修改接口更换商品编号 */
    updated = make_product("P999", "错误编号", 1, 1, 1.0);
    assert(inventory_update(&inv, "P001", &updated) == 0);

    /* 测试保存和重新加载 */
    assert(storage_save(&inv, "test_inventory.csv") == 1);

    inventory_init(&loaded);
    assert(storage_load(&loaded, "test_inventory.csv") == 1);
    assert(loaded.count == 2);
    assert(inventory_find(&loaded, "P001") >= 0);

    remove("test_inventory.csv");

    assert(inventory_delete(&inv, "P002") == 1);
    assert(inventory_delete(&inv, "P002") == 0);
    assert(inv.count == 1);

    puts("All inventory tests passed.");
    return 0;
}