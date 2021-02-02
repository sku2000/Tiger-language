#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "tiger.h"

#define SET_VAL (void *)1

Set *make_set() {
    Set *set = calloc(1, sizeof(Set));
    Map *map = make_map();
    set->map = map;
    return set;
}
void set_add(Set *set, char *key) {
    map_put(set->map, key, SET_VAL);
}
int set_contaion(Set *set, char *key) {
    void * val = map_get(set->map, key);
    if (val == SET_VAL)
        return 1;
    else 
        return 0;    
}
void set_remove(Set *set, char *key) {
    map_remove(set->map, key);
}

int set_len(Set *set) {
    return set->map->nelem;
}
