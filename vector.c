#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "tiger.h"

#define MAX_SIZE 8

static int roundup(int n) {
    if (n == 0)
        return 0;
    int r = 1;
    while (n > r)
        r *= 2;
    return r;  
}

static Vector *do_make_vrctor(int size) {
    Vector *r = (Vector *)malloc(sizeof(Vector));
    size = roundup(size);
    if (size > 0)
        r->val = malloc(sizeof(void *)*size);
    r->nelem = 0;
    r->size = size;
    return r;    
}

static void extend(Vector *vec, int size) {
    if (vec->nelem + size < vec->size)
        return;
    int newsize = roundup(vec->nelem + size);
    newsize = newsize > MAX_SIZE ? newsize : MAX_SIZE;
    void **newval = malloc(sizeof(void *) * newsize);
    memcpy(newval, vec->val, sizeof(void *) * vec->nelem);
    vec->val = newval;
    vec->size = newsize;
}

Vector *make_vectorLen(int i) {
    return do_make_vrctor(i);
}

Vector *make_vector() {
    return do_make_vrctor(0);
}

Vector *make_vector1(void *val) {
    Vector *vec = do_make_vrctor(0);
    vec_push(vec, val);
    return vec;
}

void vec_push(Vector *vec, void *val) {
    extend(vec, 1);
    vec->val[vec->nelem++] = val; 
}

Vector *vec_copy(Vector *src) {
    Vector *vec = do_make_vrctor(src->nelem);
    memcpy(vec->val, src->val, sizeof(void *) * src->nelem);
    vec->nelem = src->nelem;
    return vec;
}

void vec_append(Vector *a, Vector *src) {
    extend(a, src->nelem);
    memcpy(a->val + src->nelem, src->val, sizeof(void *) * src->nelem);
    a->nelem += src->nelem;
}

void *vec_pop(Vector *vec) {
    assert(vec->nelem > 0);
    return vec->val[--vec->nelem];
}

void *vec_get(Vector *vec, int index) {
    assert(index >= 0 && index < vec->nelem);
    return vec->val[index];
}

void *vec_head(Vector *vec) {
    assert(vec->nelem > 0);
    return vec->val[0];
}

void *vec_tail(Vector *vec) {
    assert(vec->nelem > 0);
    return vec->val[vec->nelem - 1];
}

Vector *vec_reverse(Vector *vec) {
    Vector *r = do_make_vrctor(vec->nelem);
    for (int i = 0; i < vec->nelem; i++)
        r->val[i] = vec->val[vec->nelem - i - 1];
    r->nelem = vec->nelem;
    return r;
}

void *vec_body(Vector *vec) {
    return vec->val;
}

int vec_len(Vector *vec) {
    return vec->nelem;
}

void vec_set(Vector *vec, int index, void *val) {
    assert(index >= 0 && index < vec->nelem);
    vec->val[index] = val;
}

void vec_insert(Vector *vec, int index, void *val) {
    assert(index >= 0 && index <= vec->nelem);
    void *val1 = val;
    for (int i = index; i<vec->nelem; i++) {
        val1 = vec->val[i];
        vec_set(vec, i, val);
        val = val1;
    }
    vec_push(vec, val);
}


