#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "tiger.h"

#define INIT_SIZE 16
#define TOMBSTONE ((void *)-1)

static uint32_t hash(char *p) {
    uint32_t r = 2166136261;
    for (;*p;p++)
    {        
        r ^= *p;
        r *= 16777619;
    }
    return r;    
}

static Map *do_make_map(Map *parent, int size) {
    Map *map = (Map *)calloc(1, sizeof(Map));
    map->parent = parent;
    map->key = (char **)calloc(size, sizeof(char *));
    map->val = (void **)calloc(size, sizeof(void *));
    map->size = size;
    map->nelem = 0;
    map->nused = 0;

    map->hash = hash;
    map->eq = strcmp;

    return map;    
}

Map *make_map() {
    return do_make_map(NULL, INIT_SIZE);
}

Map *make_parent_map(Map *parent) {
    return do_make_map(parent, INIT_SIZE);
}

void *map_get(Map *map, void *key) {
    if (map == NULL) 
        return NULL;

    if (map->key == NULL)
        return NULL;
    
    void *r = NULL;

    int mask = map->size - 1;
    int i = (map->hash)(key) & mask;

    for (;map->key[i] != NULL; i = (i + 1) & mask)    
        if ( map->key[i] != TOMBSTONE && ! map->eq(map->key[i], key))
            r = map->val[i];        
    
    if (r)
        return r;
    if (map->parent != NULL) 
        return map_get(map->parent, key); 

    return NULL;   
}

static void maybe_rehash(Map *map) {
    if (map->nused < map->size * 0.75)
        return;

    char **key = map->key;
    void **val = map->val;
    int oldsize = map->size;

    int newsize = (map->nelem < map->size*0.35) ? map->size : 2*map->size;
    map->key = (char **)calloc(newsize, sizeof(char *));
    map->val = (void **)calloc(newsize, sizeof(void *));
    map->size = newsize;
    map->nelem = 0;
    map->nused = 0;

    for (int i = 0; i < oldsize; i++)    
        if (key[i] != NULL && key[i] != TOMBSTONE)
            map_put(map, key[i], val[i]);        
}

int map_updata(Map *map, void *key, void *val) {
    int mask = map->size - 1;
    int i = (map->hash)(key) & mask;
    while (1)
    {
        char *k = map->key[i];
        if (k == NULL)
        {
            if (map->parent == NULL)            
                return 0;
            else
                return map_updata(map->parent, key, val);           
        } 
        if (!map->eq(key, k))
        {
            map->val[i] = val;
            return 1;
        }
        i = (i + 1) & mask;
    }    
}

int map_contation(Map *map, char *key) {
    if (map == NULL) 
        return 0;

    if (map->key == NULL)
        return 0;
    
    void *r = NULL;

    int mask = map->size - 1;
    int i = (map->hash)(key) & mask;

    for (;map->key[i] != NULL; i = (i + 1) & mask)    
        if ( map->key[i] != TOMBSTONE && !map->eq(map->key[i], key))
            r = map->val[i];

    if (r)
        return 1;
    else
        return 0;
}

void map_put(Map *map, void *key, void *val) {
    maybe_rehash(map);
    
    int mask = map->size - 1;
    int i = (map->hash)(key) & mask;

    while (1)
    {
        char *k = map->key[i];
        if (k == NULL || k == TOMBSTONE)
        {
            map->key[i] = key;
            map->val[i] = val;
            map->nelem++;
            if (k == NULL)            
                map->nused++;            
            break;            
        }
        if (!map->eq(key, k))
        {
            map->val[i] = val;
            break;
        }        
        i = (i + 1) & mask;
    }
}

void map_remove(Map *map, char *key) {
    if (!map->key)
        return;
    
    int mask = map->size - 1;
    int i = (map->hash)(key) & mask;

    for (;map->key[i] != NULL; i = (i+1)&mask)
    {
        if (map->key != TOMBSTONE && !map->eq(key, map->key[i]))
        {
            map->key[i] = TOMBSTONE;
            map->val[i] = NULL;
            map->nelem--;
        }        
    }
}

int map_len(Map *map) {
    return map->size;
}

int map_contaion1(Map *map, char *key) {
    char *k = map_get1(map, key);
    if (map->frame == 0 || k !=NULL)
        return 1;
    else
        return 0;
}

void *map_get1(Map *map, char *key) {
    if (map == NULL) 
        return NULL;

    if (map->key == NULL)
        return NULL;
    
    void *r = NULL;

    int mask = map->size - 1;
    int i = (map->hash)(key) & mask;

    for (;map->key[i] != NULL; i = (i + 1) & mask)    
        if ( map->key[i] != TOMBSTONE &&!map->eq(map->key[i], key))
            r = map->val[i];        
    
    if (r) {
        map->frame = 0;
        return r;
    }
    if (map->parent != NULL) {
        void *k = map_get1(map->parent, key); 
        map->frame = map->parent->frame + 1;
        return k;
    }

    return NULL;   
}
