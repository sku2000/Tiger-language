#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiger.h"

#define OFFSET(operand, align) ((operand +(align - 1)) & ~(align - 1))

Vector *strVec = NULL;
int offset = 0;
Set *recodeTypeSet = NULL;

void frameDecs(Vector *vec, Map *map);
void frameExps(Vector *vec, Map *map);
Type *frameExp(Node *node, Map *map);
void frameLet(Node *node, Map *map);
void frameFun(Node *node, Map *map);
void frameFields(Node *node, Map *map);
void recodeOffset(Type *ty);

void recodeOffset(Type *ty) {
    Map *map = make_map();
    int len = vec_len(ty->fields);
    int flag = 0;
    int offset = 0;
    for (int i = 0; i<len; i++) {
        char *tyName = vec_get(ty->fields, i);
        Type *ty1 = map_get(ty->recordType, tyName);
        int *off = calloc(1, sizeof(int));
        if (ty1 == INTTYPE)
        {
            offset = OFFSET(offset, 4);
            *off = offset;
            offset += 4;
        } else
        {
            offset = OFFSET(offset, 8);
            *off = offset;
            offset += 8;
            flag = 1;
        }
        map_put(map, tyName, off);
    }
    if (flag == 1)
        offset = OFFSET(offset, 8);    
    ty->offsetMap = map;
    ty->len = offset;
}

void frameDecs(Vector *vec, Map *map) {
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        if (node->ntype == ASSIGN)
        {
            if (node->left->type->type == RECORD && node->left->type->mark == 0)
            {
                recodeOffset(node->left->type);
                node->left->type->mark = 1;
            }
                        
            offset += 8;
            node->left->offset = -offset;
            frameExp(node->right, map);
            map_put(map, node->left->varName, node->left);            
        }
    }

    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        if (node->ntype == FUNCTION)
        {
            frameFun(node, map);
        }
    }
}

void frameFun(Node *node, Map *map) {
    Map *nmap = make_parent_map(map);
    Vector *vec = node->fargs;
    int len = (vec == NULL ? 0 : vec_len(vec));
    for (int i = 0; i<len; i++) {
        Node *arg = vec_get(vec, i);
        if (arg->left->type->type == RECORD && arg->left->type->mark == 0) {
            recodeOffset(arg->left->type);
            arg->left->type->mark = 1;
        }
        arg->left->offset = (len - 1 -i) * 8 + 24;
        map_put(nmap, arg->left->varName, arg->left);
    }

    if (node->body->ntype == LET)    
        frameLet(node->body, nmap);
    else {
        int off = offset;
        offset = 0;
        frameExp(node->body, nmap);
        node->loopVar = offset;
        offset = off;
    }
}

Type *frameExp(Node *node, Map *map) {
    if (node->ntype == VAR)
    {
        node->offset = ((Node *)map_get1(map, node->varName))->offset;
        node->level = map->frame;
    } else if (node->ntype == ARROP)
    {
        frameExp(node->left, map);
        frameExp(node->right, map);
    } else if (node->ntype == DOT)
    {
        frameExp(node->left, map);        
        Map *vmap = node->left->type->offsetMap;
        node->right->offset = *((int *)map_get(vmap, node->right->varName));
    }        
    else if (node->ntype == LET)
    {
        Map *nmap = make_parent_map(map);
        frameLet(node, nmap);
    } else if (node->ntype == ASSIGN)
    {      
        frameExp(node->right, map);
        frameExp(node->left, map);
    } else if (node->ntype == PLUS || node->ntype == MINUS || node->ntype == TIMES || 
            node->ntype == DIVIDE || node->ntype == LT || node->ntype == LE || 
            node->ntype == GT || node->ntype == GE || node->ntype == OR || node->ntype == AND ||
            node->ntype == EQ || node->ntype == NEQ)
    {
        frameExp(node->left, map);
        frameExp(node->right, map);
    } else if (node->ntype == CALL)
    {
        Vector *real = node->args;
        int len = real == NULL ? 0 : vec_len(real);
        for (int i = 0; i<len; i++) {
            Node *node = vec_get(real, i);
            frameExp(node, map);            
        }
    } else if (node->ntype == NSTRING)
    {
        vec_push(strVec, node);
    } else if (node->ntype == COND)
    {
        frameExp(node->cond, map);
        frameExp(node->then, map);
        if (node->els != NULL)
            frameExp(node->els, map);
    } else if (node->ntype == WHILE)
    {
        frameExp(node->exp1, map);
        frameExp(node->exp2, map);
    } else if (node->ntype == FOR)
    {    
        frameExp(node->exp1->right, map);
        frameExp(node->exp2, map);
        offset += 8;
        node->exp1->left->offset = -offset;
        if (map_contaion1(map, node->exp1->left->varName) == 1)
        {
            Node *n = map_get(map, node->exp1->left->varName);            
            map_put(map, node->exp1->left->varName, node->exp1->left);
            frameExp(node->exp3, map);
            map_put(map, node->exp1->left->varName, n);
        }
        else
        {
            map_put(map, node->exp1->left->varName, node->exp1->left);
            frameExp(node->exp3, map);
            map_remove(map, node->exp1->left->varName);
        }   
    } else if (node->ntype == SEQ)
    {
        Vector *vec = node->sequence;
        int len = vec_len(vec);
        for (int i = 0; i<len; i++) 
            frameExp(vec_get(vec, i), map);        
    } else if (node->ntype == NEGATIVE)
    {
        frameExp(node->unary, map);
    } else if (node->ntype == ARRAY)
    {
        frameExp(node->arrlen, map);
        frameExp(node->initArr, map);
    } else if (node->ntype == NRECORD)
    {
        frameFields(node, map);
    }
    return node->type;
}

void frameFields(Node *node, Map *map) {
    Vector *vec = node->tyfields;
    int len = vec_len(vec);
    Map *tymap = node->type->offsetMap;
    for (int i = 0; i<len; i++) {
        Node *n = vec_get(vec, i);
        int *off = (int *)map_get(tymap, n->left->varName);
        n->left->offset = *off;
        frameExp(n->right, map);
    }
}

void frameExps(Vector *vec, Map *map) {
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        frameExp(node, map);
    }
}

void frameLet(Node *node, Map *map) {
    int off = offset;
    offset = 0;
    frameDecs(node->decs, map);
    frameExps(node->expseq, map);
    node->localVarLen = offset;
    offset = off;
}

Frame *frame(Node *node) {
    recodeTypeSet = make_set();
    strVec = make_vector();
    Map *map = make_map();
    frameLet(node, map);

    Frame *fm = calloc(1, sizeof(Frame));
    fm->stVec = strVec;
    fm->node = node;
    return fm;
}