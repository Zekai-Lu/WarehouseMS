#include "inventory.h"
#include "warehouse_storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DATA_FILE "data/inventory.csv"

static void read_line(const char *prompt, char *buf, size_t size)
{
    size_t len;

    printf("%s", prompt);

    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }

    len = strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
        buf[--len] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* 丢弃超长输入剩余部分 */
        }
    }
}

static int read_int(const char *prompt, int *value)
{
    char buf[64];
    char *end;
    long n;

    read_line(prompt, buf, sizeof(buf));

    errno = 0;
    n = strtol(buf, &end, 10);

    if (buf[0] == '\0' || errno != 0 || end == buf ||
        *end != '\0' || n < 0 || n > 2147483647L) {
        printf("输入无效，请输入非负整数。\n");
        return 0;
    }

    *value = (int)n;
    return 1;
}

static int read_price(const char *prompt, double *value)
{
    char buf[64];
    char *end;
    double n;

    read_line(prompt, buf, sizeof(buf));

    errno = 0;
    n = strtod(buf, &end);

    if (buf[0] == '\0' || errno != 0 || end == buf ||
        *end != '\0' || n < 0.0 || n > 1.0e100) {
        printf("输入无效，请输入非负价格。\n");
        return 0;
    }

    *value = n;
    return 1;
}

static void print_product(const Product *p)
{
    printf("%-12s %-16s %-12s %-8s %8d %8d %10.2f\n",
           p->id, p->name, p->category, p->unit,
           p->quantity, p->min_quantity, p->unit_price);
}

static void print_header(void)
{
    printf("%-12s %-16s %-12s %-8s %8s %8s %10s\n",
           "编号", "名称", "分类", "单位",
           "库存", "预警线", "单价");
}

static void list_products(const Inventory *inv)
{
    size_t i;

    if (inv->count == 0) {
        printf("当前没有商品。\n");
        return;
    }

    print_header();

    for (i = 0; i < inv->count; i++) {
        print_product(&inv->items[i]);
    }
}

static int read_product(Product *p)
{
    memset(p, 0, sizeof(*p));

    read_line("商品编号: ", p->id, sizeof(p->id));
    read_line("商品名称: ", p->name, sizeof(p->name));
    read_line("商品分类: ", p->category, sizeof(p->category));
    read_line("计量单位: ", p->unit, sizeof(p->unit));

    if (p->id[0] == '\0' || p->name[0] == '\0' ||
        p->category[0] == '\0' || p->unit[0] == '\0' ||
        strchr(p->id, ',') || strchr(p->name, ',') ||
        strchr(p->category, ',') || strchr(p->unit, ',')) {
        printf("字段不能为空，也不能包含英文逗号。\n");
        return 0;
    }

    if (!read_int("当前库存: ", &p->quantity) ||
        !read_int("最低库存预警线: ", &p->min_quantity) ||
        !read_price("商品单价: ", &p->unit_price)) {
        return 0;
    }

    return 1;
}

static void add_product(Inventory *inv)
{
    Product p;

    if (!read_product(&p)) {
        return;
    }

    if (inventory_add(inv, &p)) {
        printf("添加成功。\n");
    } else {
        printf("添加失败：商品编号重复、数据不合法或库存已满。\n");
    }
}

static void search_product(const Inventory *inv)
{
    char id[ID_LEN];
    int index;

    read_line("输入要查询的商品编号: ", id, sizeof(id));
    index = inventory_find(inv, id);

    if (index < 0) {
        printf("未找到商品。\n");
        return;
    }

    print_header();
    print_product(&inv->items[index]);
}

static void update_product(Inventory *inv)
{
    char id[ID_LEN];
    int index;
    Product p;

    read_line("输入要修改的商品编号: ", id, sizeof(id));
    index = inventory_find(inv, id);

    if (index < 0) {
        printf("未找到商品。\n");
        return;
    }

    printf("请重新输入商品信息（编号必须保持不变）。\n");

    if (!read_product(&p)) {
        return;
    }

    if (inventory_update(inv, id, &p)) {
        printf("修改成功。\n");
    } else {
        printf("修改失败，请检查编号和字段。\n");
    }
}

static void delete_product(Inventory *inv)
{
    char id[ID_LEN];
    char confirm[8];

    read_line("输入要删除的商品编号: ", id, sizeof(id));
    read_line("确认删除？输入 y 确认: ", confirm, sizeof(confirm));

    if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0) {
        printf("已取消。\n");
        return;
    }

    printf("%s\n", inventory_delete(inv, id) ?
           "删除成功。" : "删除失败，商品不存在。");
}

static void change_stock(Inventory *inv, int is_in)
{
    char id[ID_LEN];
    int amount;
    int ok;

    read_line("商品编号: ", id, sizeof(id));

    if (!read_int("变动数量: ", &amount)) {
        return;
    }

    if (amount == 0) {
        printf("变动数量必须大于零。\n");
        return;
    }

    ok = is_in ? inventory_stock_in(inv, id, amount)
               : inventory_stock_out(inv, id, amount);

    if (ok) {
        printf("%s成功。\n", is_in ? "入库" : "出库");
    } else {
        printf("操作失败：请检查商品编号、数量和当前库存。\n");
    }
}

static void show_statistics(const Inventory *inv)
{
    size_t i;
    long long total_quantity = 0;

    for (i = 0; i < inv->count; i++) {
        total_quantity += inv->items[i].quantity;
    }

    printf("商品种类数：%zu\n", inv->count);
    printf("库存总件数：%lld\n", total_quantity);
    printf("库存总价值：%.2f\n", inventory_total_value(inv));
    printf("低库存商品数：%d\n", inventory_low_stock_count(inv));
}

static void show_low_stock(const Inventory *inv)
{
    size_t i;
    int found = 0;

    print_header();

    for (i = 0; i < inv->count; i++) {
        if (inv->items[i].quantity <= inv->items[i].min_quantity) {
            print_product(&inv->items[i]);
            found = 1;
        }
    }

    if (!found) {
        printf("没有低库存商品。\n");
    }
}

int main(void)
{
    Inventory inv;
    int choice;
    int running = 1;

    inventory_init(&inv);

    if (!storage_load(&inv, DATA_FILE)) {
        printf("读取数据失败，请检查 %s 文件。\n", DATA_FILE);
        return 1;
    }

    printf("=== 仓库库存管理系统 ===\n");

    while (running) {
        printf("\n"
               "1. 查看全部商品\n"
               "2. 新增商品\n"
               "3. 查询商品\n"
               "4. 修改商品\n"
               "5. 删除商品\n"
               "6. 入库\n"
               "7. 出库\n"
               "8. 库存统计\n"
               "9. 低库存预警\n"
               "0. 保存并退出\n");

        if (!read_int("请选择操作: ", &choice)) {
            continue;
        }

        switch (choice) {
        case 1:
            list_products(&inv);
            break;
        case 2:
            add_product(&inv);
            break;
        case 3:
            search_product(&inv);
            break;
        case 4:
            update_product(&inv);
            break;
        case 5:
            delete_product(&inv);
            break;
        case 6:
            change_stock(&inv, 1);
            break;
        case 7:
            change_stock(&inv, 0);
            break;
        case 8:
            show_statistics(&inv);
            break;
        case 9:
            show_low_stock(&inv);
            break;
        case 0:
            if (storage_save(&inv, DATA_FILE)) {
                printf("数据已保存，再见！\n");
                running = 0;
            } else {
                printf("保存失败！请检查 data 目录及文件权限。\n");
            }
            break;
        default:
            printf("无效选项，请重新输入。\n");
            break;
        }
    }

    return 0;
}