#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "tiger.h"

Node *testLet(Node *node, Map *map);
void testDecs(Vector *vec, Map *map);
void testVardec(Node *node, Map *map);
Node *testExp(Node *node, Map *map);
Node *testLvalue(Node *node, Map *map);
Node *testExps(Vector *vec, Map *map);
Map *getRecordMap(Node *node, Map *map);
Vector *getArray(Node *node, Map *map);
void initMap(Map *map);
Node *tprint(Map *map);
Node *tflush();
Node *tgetchar();
Node *tord(Map *map);
Node *tchr(Map *map);
Node *tsize(Map *map);
Node *tsubstring(Map *map);
Node *tconcat(Map *map);
Node *tnot(Map *map);
void texit(Map *map);

static int loopFlag = 0;

Node *NILNODE = &(Node){
    .ntype = NIL,
    .type = &(Type){
        .type = RECORD
    }
};

void initMap(Map *map) {
     Type *ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("print");
    ty->args = make_vector();
    char *arg = copyString("s");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, STRTYPE);
    ty->returnType = VOIDTYPE;
    Node *node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("flush");
    ty->args = NULL;
    ty->argsType = NULL;
    ty->returnType = VOIDTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);
   

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("getchar");
    ty->args = NULL;
    ty->argsType = NULL;
    ty->returnType = STRTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("ord");
    ty->args = make_vector();
    arg = copyString("s");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, STRTYPE);
    ty->returnType = INTTYPE;
    map_put(map, ty->typeName, ty);node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("chr");
    ty->args = make_vector();
    arg = copyString("i");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, INTTYPE);
    ty->returnType = STRTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("size");
    ty->args = make_vector();
    arg = copyString("s");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, STRTYPE);
    ty->returnType = INTTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("substring");
    ty->args = make_vector();
    char *arg1 = copyString("s");
    char *arg2 = copyString("first");
    char *arg3 = copyString("n");
    vec_push(ty->args, arg1);
    vec_push(ty->args, arg2);
    vec_push(ty->args, arg3);
    ty->argsType = make_map();
    map_put(ty->argsType, arg1, STRTYPE);
    map_put(ty->argsType, arg2, INTTYPE);
    map_put(ty->argsType, arg3, INTTYPE);
    ty->returnType = STRTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("concat");
    ty->args = make_vector();
    arg1 = copyString("s1");
    arg2 = copyString("s2");
    vec_push(ty->args, arg1);
    vec_push(ty->args, arg2);
    ty->argsType = make_map();
    map_put(ty->argsType, arg1, STRTYPE);
    map_put(ty->argsType, arg2, STRTYPE);
    ty->returnType = STRTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("not");
    ty->args = make_vector();
    arg = copyString("i");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, INTTYPE);
    ty->returnType = INTTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("exit");
    ty->args = make_vector();
    arg = copyString("i");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, INTTYPE);
    ty->returnType = VOIDTYPE;
    node = calloc(1, sizeof(Node));
    node->type = ty;
    map_put(map, ty->typeName, node);
}

void testTiger(Node *node) {
    Node *result = NULL;
    Map *map = make_map();
    initMap(map);
    if (node->ntype == LET)
    {
        result = testLet(node, map);
    }       
}

Node *testLet(Node *node, Map *map) {
    testDecs(node->decs, map); 
    return testExps(node->expseq, map);   
}

void testDecs(Vector *vec, Map *map) {
    Node *node = NULL;
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        node = vec_get(vec, i);
        if (node->ntype == ASSIGN)
        {
            testVardec(node, map);
        } else if (node->ntype == FUNCTION)
        {
            node->context = map;
            map_put(map, node->funName->varName, node);
        }                
    }
}

void testVardec(Node *node, Map *map) {    
    Node *result = testExp(node->right, map);
    map_put(map, node->left->varName, result);    
}

Node *testExps(Vector *vec, Map *map) {
    Node *result = NULL;
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        result = testExp(node, map);
        if (loopFlag == 1)
            break;        
    }
    return result;
}

Node *testExp(Node *node, Map *map) {
    Node *result = NULL;

    if (node->ntype == VAR || node->ntype == DOT || node->ntype == ARROP)
    {
        result = testLvalue(node, map);
    }
    else if (node->ntype == NINT)
    {
        Node *n = calloc(1, sizeof(Node));
        memcpy(n, node, sizeof(Node));
        return n;
    }
    else if (node->ntype == NSTRING)
    {
        Node *n = calloc(1, sizeof(Node));
        memcpy(n, node, sizeof(Node));
        return n;
    }
    else if (node->ntype == PLUS || node->ntype == MINUS || node->ntype == TIMES || 
            node->ntype == DIVIDE || node->ntype == LT || node->ntype == LE || 
            node->ntype == GT || node->ntype == GE || node->ntype == OR || node->ntype == AND)
    {
        Node *n1 = testExp(node->left, map);
        Node *n2 = testExp(node->right, map);
        result = calloc(1, sizeof(Node));

        result->type = n1->type;
        if (node->ntype == PLUS)
            result->ival = n1->ival + n2->ival;
        else if (node->ntype == MINUS)
            result->ival = n1->ival - n2->ival;
        else if (node->ntype == TIMES)
            result->ival = n1->ival * n2->ival;
        else if (node->ntype == DIVIDE)
            result->ival = n1->ival / n2->ival;
        else if (node->ntype == LT)
            result->ival = n1->ival < n2->ival;
        else if (node->ntype == LE)
            result->ival = n1->ival <= n2->ival;
        else if (node->ntype == GT)
            result->ival = n1->ival > n2->ival;
        else if (node->ntype == GE)
            result->ival = n1->ival >= n2->ival;
        else if (node->ntype == OR)
            result->ival = n1->ival || n2->ival;
        else if (node->ntype == AND)
            result->ival = n1->ival && n2->ival;
    } else if (node->ntype == EQ || node->ntype == NEQ)
    {
        Node *n1 = testExp(node->left, map);
        Node *n2 = testExp(node->right, map);
        result = calloc(1, sizeof(Node));
        result->type = INTTYPE;        
        if (node->ntype == EQ)
        {            
            if (n1->type == INTTYPE && n2->type == INTTYPE)
            {                
                result->ival = (n1->ival == n2->ival);
            }else if (n1->type == STRTYPE && n2->type == STRTYPE)
            {
                result->ival = !strcmp(n1->strVal, n2->strVal);
            } else
            {
                result->ival = (n1 == n2);
            }   
        }
        else if (node->ntype == NEQ)
        {
            if (n1->type == INTTYPE && n2->type == INTTYPE)
            {                
                result->ival = !(n1->ival == n2->ival);
            }else if (n1->type == STRTYPE && n2->type == STRTYPE)
            {
                result->ival = strcmp(n1->strVal, n2->strVal);
            } else
            {
                result->ival = !(n1 == n2);
            }
        }  
    }     
    else if (node->ntype == NRECORD)
    {
        Node *newRecord = calloc(1, sizeof(Node));
        memcpy(newRecord, node, sizeof(Node));
        Vector *vec = newRecord->tyfields;
        int len = vec_len(vec);
        Map *fieldMap = make_map();
        for (int i = 0; i<len; i++) {
            Node *n = vec_get(vec, i);
            Node *left = n->left;
            Node *right = n->right;
            Node *exp = testExp(right, map);
            map_put(fieldMap, left->varName, exp);           
        }
        newRecord->recode = fieldMap;
        result = newRecord;
    } else if (node->ntype == ASSIGN)
    {
        Node *left = node->left;
        Node *right = node->right;
        if (left->ntype == VAR)
        { 
            Node *n = testExp(right, map);           
            map_updata(map, left->varName, n);            
        } else if (left->ntype == DOT)
        {
            Node *re = testExp(right, map);
            Map *vmap =  getRecordMap(left->left, map);    
            map_updata(vmap, left->right->varName, re);        
        } else if (left->ntype == ARROP)
        {
            Node *re = testExp(right, map);
            Vector *vec = getArray(left->left, map);
            Node *inode = testExp(left->right, map);
            vec_set(vec, inode->ival, re);
        }         
    } else if (node->ntype == ARRAY)
    {
        Node *newARR = calloc(1, sizeof(Node));
        memcpy(newARR, node, sizeof(Node));

        Node *arrLen = testExp(newARR->arrlen, map);
        Node *arrInlt = testExp(newARR->initArr, map);        
        newARR->arr = make_vectorLen(arrLen->ival);
        for (int i = 0; i<arrLen->ival; i++) {
            Node *n = calloc(1 ,sizeof(Node));
            memcpy(n, arrInlt, sizeof(Node));
            vec_push(newARR->arr, n);
        }
        result = newARR;
    } else if (node->ntype == NEGATIVE)
    {
        result = testExp(node->unary, map);
        result->ival = - result->ival;
    } else if (node->ntype == NIL)
    {
        return NILNODE;
    }    
    else if (node->ntype == COND)
    {
        Node *condr = testExp(node->cond, map);
        if (node->els != NULL)
        {
            if (condr->ival != 0)
            {
                result = testExp(node->then, map);
            } else
            {
                result = testExp(node->els, map);
            } 
        } else
        {
            if (condr->ival != 0)
                testExp(node->then, map);           
        }   
    } else if (node->ntype == WHILE)
    {               
        Node *r = testExp(node->exp1, map); 

        while (r->ival != 0)
        {
            testExp(node->exp2, map);
            r = testExp(node->exp1, map);
            if (loopFlag == 1)
                break;            
        }
        loopFlag = 0;   
    } else if (node->ntype == BREAK)
    {
        loopFlag = 1;
    } else if (node->ntype == SEQ)
    {
        Vector *vec = node->sequence;
        int len = vec_len(vec);
        for (int i = 0; i<len - 1; i++) {
            testExp(vec_get(vec, i), map);
            if (loopFlag == 1)
                return NILNODE;            
        }
        result = testExp(vec_get(vec, len-1), map);
    } else if (node->ntype == FOR)
    {
        Node *begin = testExp(node->exp1->right, map);
        Node *end = testExp(node->exp2, map);
        Map *vmap = make_parent_map(map);
        Node *n = calloc(1, sizeof(Node));
        map_put(vmap, node->exp1->left->varName, begin);
        for (; begin->ival <= end->ival; (begin->ival)++) {
           testExp(node->exp3, vmap);
           if (loopFlag == 1)
                break; 
        }
        loopFlag = 0;
    } else if (node->ntype == LET)
    {
        Map *vmap = make_parent_map(map);
        result = testLet(node, vmap);
    } else if (node->ntype == CALL)
    {
        Node *fun = map_get(map, node->callName->varName);
        Type *funTy = fun->type;
        Vector *form = funTy->args;
        Vector *real = node->args;
        int len = form == 0 ? 0 : vec_len(form);
        Map *funMap = make_parent_map(fun->context);
        for (int i = 0; i<len; i++) {
            Node *n = vec_get(real, i);
            Node *r = testExp(n, map);
            map_put(funMap, vec_get(form, i), r);
        }
        char *fname = node->callName->varName;
        if (!strcmp(fname, "print"))
        {
            return tprint(funMap);
        } else if (!strcmp(fname, "flush"))
        {
            return tflush();
        } else if (!strcmp(fname, "getchar"))
        {
            return tgetchar();
        } else if (!strcmp(fname, "ord"))
        {
            return tord(funMap);
        } else if (!strcmp(fname, "chr"))
        {
            return tchr(funMap);
        } else if (!strcmp(fname, "size"))
        {
            return tsize(funMap);
        } else if (!strcmp(fname, "substring"))
        {
            return tsubstring(funMap);
        } else if (!strcmp(fname, "concat"))
        {
            return tconcat(funMap);
        } else if (!strcmp(fname, "not"))
        {
            return tnot(funMap);
        } else if (!strcmp(fname, "exit"))
        {
            texit(funMap);
        }
        result = testExp(fun->body, funMap);
    }       
    return result;
}

void texit(Map *map) {
    Node *n = map_get(map, "i");
    exit(n->ival);
}

Node *tnot(Map *map) {
    Node *n = map_get(map, "i");
    Node *node = calloc(1, sizeof(Node));
    node->ival = n->ival == 0;
    node->type = INTTYPE;
}

Node *tconcat(Map *map) {
    Node *s1 = map_get(map, "s1");
    Node *s2 = map_get(map, "s2");

    char *s = calloc(1, (s1->strLen + s2->strLen + 1) * sizeof(char));
    strcat(s, s1->strVal);
    strcat(s, s2->strVal);

    Node *node = calloc(1, sizeof(Node));
    node->strVal = s;
    node->strLen = s1->strLen + s2->strLen;
    node->type = STRTYPE;
    return node;
}

Node *tsubstring(Map *map) {
    Node *str = map_get(map, "s");
    Node *first = map_get(map, "first");
    Node *len = map_get(map, "n");

    char *s = calloc(1, (len->strLen + 1) * sizeof(char));
    int end = first->ival + len->ival;
    int index = 0;
    for (int i = first->ival; i<end && i<str->strLen; i++) {
        //后面的字符会改变
        *(s + index) = *((str->strVal) + i);
        index++;
    }
    *(s + index) = '\0';
    Node *node = calloc(1 ,sizeof(Node));
    node->strVal = s;
    node->strLen = index+1;
    node->type = STRTYPE;
    return node;
}

Node *tsize(Map *map) {
    Node *node = map_get(map, "s");
    int i = node->strLen;

    Node *n = calloc(1, sizeof(Node));
    n->type = INTTYPE;
    n->ival = i;

    return n;
}

Node *tchr(Map *map) {
    Node *node = map_get(map, "i");
    int i = node->ival;
    if (i < 0 || i > 127)
    {
        exit(1);
    } else
    {
        Node *node = calloc(1, sizeof(Node));
        node->type = STRTYPE;
        node->strVal = calloc(1, 2*sizeof(char));
        *(node->strVal) = i;
        node->strLen = 1;
        return node;
    }
}

Node *tord(Map *map) {
    Node *node = map_get(map, "s");
    char *s = node->strVal;

    Node *n = calloc(1, sizeof(Node));
    n->type = INTTYPE;
    if (node->strLen == 0)
    {
        n->ival = -1;
    } else if (node > 0)
    {
        n->ival = (int)(*s);
    } else  
        printf("err \n");
    return n;
}

Node *tgetchar() {
    int c =  getchar();
    Node *node = calloc(1 ,sizeof(Node));
    node->type = STRTYPE;
    
    if (c == EOF)
    {
        node->strVal = NULL;
        node->strLen = 0;
    } else
    {
        node->strVal = calloc(1, 2*sizeof(char));
        node->strLen = 1;
        *(node->strVal) = (char)c;
    }
    return node;
}

Node *tflush() {
    fflush(stdout);
    Node *n = calloc(1, sizeof(Node));
    n->type = VOIDTYPE;
    return n;
}

Node *tprint(Map *map) {
    Node *node = map_get(map, "s");
    if (node->strLen > 0)
    {
        printf("%s", node->strVal);
    }    
    Node *n = calloc(1, sizeof(Node));
    n->type = VOIDTYPE;
    return n;
}

Vector *getArray(Node *node, Map *map) {
    if (node->ntype == ARROP)
    {
        Vector *vec = getArray(node->left, map);
        Node *n = vec_get(vec, ((Node *)testExp(node->right, map))->ival);
        return n->arr;
    } else if (node->ntype == VAR)
    {
        Node *n = map_get(map, node->varName);
        return n->arr;
    }
}

Map *getRecordMap(Node *node, Map *map) {
    if (node->ntype == DOT)
    {
        Map *vmap = getRecordMap(node->left, map);  
        Node *n = map_get(vmap, node->right->varName);
        return n->recode;      
    } else if (node->ntype == VAR)
    {
        Node *n = map_get(map, node->varName);
        return n->recode;
    }    
}

Node *testLvalue(Node *node, Map *map) {
    Node *result = NULL;
    if (node->ntype == VAR) {
        result = map_get(map, node->varName);
    } else if (node->ntype == DOT)
    {
        Node *n = testLvalue(node->left, map);
        result = map_get(n->recode, node->right->varName);
    } else if (node->ntype == ARROP)
    {
        Node *n = testLvalue(node->left, map);
        Node *index = testExp(node->right, map);
        result = vec_get(n->arr, index->ival);
    }
    return result;    
}