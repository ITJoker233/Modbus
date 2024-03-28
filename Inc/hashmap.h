
#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
typedef struct HashMap
{
    uint8_t init;
    uint8_t list[255]; // 有效空间
} HashMap;

void hashMap_init(HashMap *hashMap);
void default_put_hashMap(HashMap *hashMap, uint8_t key, uint8_t value);
void default_get_hashMap(HashMap *hashMap, uint8_t key,uint8_t *value);
#endif
