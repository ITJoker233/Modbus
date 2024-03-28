#include "hashmap.h"

uint16_t default_HashCode(HashMap *hashMap, uint8_t key) 
{
    return key % sizeof(hashMap->list);
}

void default_get_hashMap(HashMap *hashMap, uint8_t key, uint8_t *value) 
{
    uint8_t index = default_HashCode(hashMap, key);
    *value =  hashMap->list[index];
}

void default_put_hashMap(HashMap *hashMap, uint8_t key, uint8_t value) 
{
    uint8_t index = default_HashCode(hashMap, key);
    hashMap->list[index] = value;
}

void hashMap_init(HashMap *hashMap) 
{
    hashMap->init = 0x01;
    memset(hashMap->list,0x00,sizeof(hashMap->list));
}