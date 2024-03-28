#include <stdint.h>
#include <stdio.h>
#include <string.h> 

#define KVDB_MAX_ENTRY 0xFFFF
#define KVDB_MAX_KEY   0xFF
#define KVDB_MAX_VALUE 0xFF

enum KVDB_OP_RESULT
{
    KVDB_OK = 0x00,
    KVDB_FAILED = 0x01,
    KVDB_EMPTY = 0x02,
    KVDB_FULL = 0x03,
    KVDB_NOT_FOUND = 0x04,
    KVDB_KEY_OR_VALUE_INVALID = 0x05
};

typedef struct
{
    char bit;
    char key[KVDB_MAX_KEY];
    uint8_t value[KVDB_MAX_VALUE];
} TYP_KVDB_ENTRY;

typedef struct
{
    uint32_t size;
    uint32_t hash;
    TYP_KVDB_ENTRY entry[KVDB_MAX_ENTRY];
} TYP_KVDB;

uint32_t kv_hash(char *key,uint32_t key_length)
{
    uint32_t i,x,hash;
    for(i = 0,hash=0,x=0; i < key_length; key++, i++)
    {
        hash = (hash << 4) + (*key);
        if((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }
    return hash;
}

uint8_t kv_init(TYP_KVDB *kv)
{
    memset(kv,0x00,sizeof(TYP_KVDB));
    return KVDB_OK;
}

uint8_t kv_get(TYP_KVDB *kv,char *key,uint32_t key_length,uint8_t *out_value)
{
    kv->hash = kv_hash(key,key_length);
    if(*key == NULL || key_length == 0)
    {
        return KVDB_KEY_OR_VALUE_INVALID;
    }
    if(kv->hash >= KVDB_MAX_ENTRY)
    {
        return KVDB_NOT_FOUND;
    }
    if(kv->entry[kv->hash].bit == 0x00)
    {
        return KVDB_NOT_FOUND;
    }
    
    *out_value = kv->entry[kv->hash].value;
    return KVDB_OK;
}

uint8_t kv_set(TYP_KVDB *kv,char *key,uint8_t *value,uint32_t key_length)
{
    if(*key == NULL || *value == NULL || key_length == 0)
    {
        return KVDB_KEY_OR_VALUE_INVALID;
    }
    if(kv->size >= KVDB_MAX_ENTRY)
    {
        return KVDB_FULL;
    }
    kv->hash = kv_hash(key,key_length);
    kv->size++;
    kv->entry[kv->hash].bit = 0x01;
    *kv->entry[kv->hash].key = key;
    *kv->entry[kv->hash].value = value;
    return KVDB_OK;
}

uint8_t kv_del(TYP_KVDB *kv,char *key,uint32_t key_length)
{
    if(kv->size == 0x00)
    {
        return KVDB_EMPTY;
    }
    if(*key == NULL || key_length == 0x00)
    {
        return KVDB_KEY_OR_VALUE_INVALID;
    }
    kv->hash = kv_hash(key,key_length);
    kv->entry[kv->hash].bit = 0x00;
    kv->size--;
    return KVDB_OK;
}
