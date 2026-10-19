#include "warehouse_storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static int parse_int(const char *s, int *value)
{
    char *end;
    long number;

    errno = 0;
    number = strtol(s, &end, 10);

    if (errno != 0 || s == end || *end != '\0' ||
        number < 0 || number > 2147483647L) {
        return 0;
    }

    *value = (int)number;
    return 1;
}

static int parse_double_value(const char *s, double *value)
{
    char *end;
    double number;

    errno = 0;
    number = strtod(s, &end);

    if (errno != 0 || s == end || *end != '\0' ||
        number < 0.0 || number > 1.0e100) {
        return 0;
    }

    *value = number;
    return 1;
}

static int parse_line(char *line, Product *p)
{
    char *fields[7];
    char *token;
    int i = 0;

    line[strcspn(line, "\r\n")] = '\0';

    token = strtok(line, ",");
    while (token != NULL && i < 7) {
        fields[i++] = token;
        token = strtok(NULL, ",");
    }

    if (i != 7 || token != NULL) {
        return 0;
    }

    memset(p, 0, sizeof(*p));

    if (strlen(fields[0]) >= ID_LEN ||
        strlen(fields[1]) >= NAME_LEN ||
        strlen(fields[2]) >= CATEGORY_LEN ||
        strlen(fields[3]) >= UNIT_LEN) {
        return 0;
    }

    strcpy(p->id, fields[0]);
    strcpy(p->name, fields[1]);
    strcpy(p->category, fields[2]);
    strcpy(p->unit, fields[3]);

    if (!parse_int(fields[4], &p->quantity) ||
        !parse_int(fields[5], &p->min_quantity) ||
        !parse_double_value(fields[6], &p->unit_price)) {
        return 0;
    }

    return 1;
}

int storage_load(Inventory *inv, const char *path)
{
    FILE *fp;
    char line[512];
    int line_number = 0;

    if (inv == NULL || path == NULL) {
        return 0;
    }

    fp = fopen(path, "r");
    if (fp == NULL) {
        /* 文件不存在时视为新库存 */
        if (errno == ENOENT) {
            inventory_init(inv);
            return 1;
        }
        return 0;
    }

    inventory_init(inv);

    while (fgets(line, sizeof(line), fp) != NULL) {
        Product p;

        line_number++;

        if (strchr(line, '\n') == NULL && !feof(fp)) {
            fclose(fp);
            inventory_init(inv);
            return 0;
        }

        /* 第一行是表头 */
        if (line_number == 1 &&
            strncmp(line, "id,", 3) == 0) {
            continue;
        }

        if (!parse_line(line, &p) || !inventory_add(inv, &p)) {
            fclose(fp);
            inventory_init(inv);
            return 0;
        }
    }

    if (ferror(fp)) {
        fclose(fp);
        inventory_init(inv);
        return 0;
    }

    fclose(fp);
    return 1;
}

int storage_save(const Inventory *inv, const char *path)
{
    FILE *fp;
    char temp_path[512];
    size_t i;
    int ok = 1;

    if (inv == NULL || path == NULL ||
        strlen(path) + 5 >= sizeof(temp_path)) {
        return 0;
    }

    snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);

    fp = fopen(temp_path, "w");
    if (fp == NULL) {
        return 0;
    }

    if (fprintf(fp, "id,name,category,unit,quantity,min_quantity,unit_price\n") < 0) {
        ok = 0;
    }

    for (i = 0; ok && i < inv->count; i++) {
        const Product *p = &inv->items[i];

        if (fprintf(fp, "%s,%s,%s,%s,%d,%d,%.2f\n",
                    p->id, p->name, p->category, p->unit,
                    p->quantity, p->min_quantity,
                    p->unit_price) < 0) {
            ok = 0;
        }
    }

    if (fclose(fp) != 0) {
        ok = 0;
    }

    if (!ok) {
        remove(temp_path);
        return 0;
    }

    /*
     * Windows 下先移除旧文件再重命名。
     * 这是一种简单的演示实现，并非严格的原子替换。
     */
    if (remove(path) != 0 && errno != ENOENT) {
        remove(temp_path);
        return 0;
    }

    if (rename(temp_path, path) != 0) {
        remove(temp_path);
        return 0;
    }

    return 1;
}