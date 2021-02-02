#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiger.h"

Type *checkLet(Node *node, Map *varMap, Map *typeMap);
Type *checkDecs(Vector *vec, Map *varMap, Map *typeMap);
void collectDecHead(Node *node, Map *typeMap);
void findType(Node *node, Map *typeMap);
Type *finalType(Map *typeMap, char *typeName, Set *set);
void findTyfields(Vector *vec, Map *map, Map *typeMap, Vector *fields);
void findFunSign(Node *node, Map *typeMap);
void checkVardec(Node *node, Map *varMap, Map *typeMap);
void fieldsToMap(Node *node, Map *child, Map *typeMap);
Type *findType1(Node *node, Map *typeMap);
Type *checkExps(Vector *vec, Map *varMap, Map *typeMap);
Type *checkExp(Node *node, Map *varMap, Map *typeMap);
Type *checkLvalue(Node *node, Map *varMap, Map *typeMap);
void checkFields(Type *ty, Vector *vec, Map *typeMap, Map *varMap);
Map *initTypeMap();

Type *UNDEFTYPE = &(Type){
    .type = ERR
};

Type *INTTYPE = &(Type){
    .type = INT
};

Type *STRTYPE = &(Type){
    .type = STRING
};

Type *ERRTYPE = &(Type){
    .type = ERR
};
Type *VOIDTYPE = &(Type){
    .type = VOID
};
Type *NILTYPE = &(Type) {
    .type = RECORD
};
Type *READONLY = &(Type) {
    .type = READ_ONLY
};

static int err = 1;
static int level = 0;

Type *returnERRTYPE(int line) {
    errormsg("semantic error %d\n", line);
    err = 0; 
    return ERRTYPE;
}

Map *initTypeMap() {
    Map *map = make_map();

    Type *ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("print");
    ty->args = make_vector();
    char *arg = copyString("s");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, STRTYPE);
    ty->returnType = VOIDTYPE;
    map_put(map, ty->typeName, ty);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("flush");
    ty->args = NULL;
    ty->argsType = NULL;
    ty->returnType = VOIDTYPE;
    map_put(map, ty->typeName, ty);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("getchar");
    ty->args = NULL;
    ty->argsType = NULL;
    ty->returnType = STRTYPE;
    map_put(map, ty->typeName, ty);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("ord");
    ty->args = make_vector();
    arg = copyString("s");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, STRTYPE);
    ty->returnType = INTTYPE;
    map_put(map, ty->typeName, ty);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("chr");
    ty->args = make_vector();
    arg = copyString("i");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, INTTYPE);
    ty->returnType = STRTYPE;
    map_put(map, ty->typeName, ty);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("size");
    ty->args = make_vector();
    arg = copyString("s");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, STRTYPE);
    ty->returnType = INTTYPE;
    map_put(map, ty->typeName, ty);

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
    map_put(map, ty->typeName, ty);

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
    map_put(map, ty->typeName, ty);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("not");
    ty->args = make_vector();
    arg = copyString("i");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, INTTYPE);
    ty->returnType = INTTYPE;
    map_put(map, ty->typeName, ty);

    ty = calloc(1, sizeof(Type));
    ty->typeName = copyString("exit");
    ty->args = make_vector();
    arg = copyString("i");
    vec_push(ty->args, arg);
    ty->argsType = make_map();
    map_put(ty->argsType, arg, INTTYPE);
    ty->returnType = VOIDTYPE;
    map_put(map, ty->typeName, ty);
}

int checkTiger(Node *node) {
    Map *varMap = make_parent_map(NULL);
    Map *typeMap = initTypeMap();

    checkLet(node, varMap, typeMap);
    return err;
}

void findFunSign(Node *node, Map *typeMap) {
    Type *ty = calloc(1, sizeof(Type));
    ty->type = FUNTYPE;
    ty->typeName = node->funName->varName;
    if (node->fargs != NULL)
    {
        Vector *args = make_vector();
        Map *argsType = make_map();
        int len = vec_len(node->fargs);
        for (int i = 0; i<len; i++) {
            Node *arg = vec_get(node->fargs, i);
            Type *t = findType1(arg->right, typeMap);
            arg->left->type = t;
            vec_push(args, arg->left->varName);
            map_put(argsType, arg->left->varName, t);
        }
        ty->argsType = argsType;
        ty->args = args;
    }

    Type *t = NULL;
    if (node->returnType != NULL)
    {
        t = findType1(node->returnType, typeMap);
    } else
    {
        t = VOIDTYPE;        
    }   
    ty->returnType = t;  
    ty->level = level;

    map_put(typeMap, node->funName->varName, ty);
}

Type *checkLet(Node *node, Map *varMap, Map *typeMap) {
    Type *ty = NULL;

    if (node->decs != NULL && vec_len(node->decs) > 0)
    {
        ty = checkDecs(node->decs, varMap, typeMap);
    }

    if (node->expseq != NULL && vec_len(node->expseq) > 0) {
        ty = checkExps(node->expseq, varMap, typeMap);
    }   
    return ty;
}

void collectDecHead(Node *node, Map *typeMap) {
    Node *left = node->left;
    Node *right = node->right;
    if (right->ntype == KINT)
    {
        map_put(typeMap, left->varName, INTTYPE);
    } else if (right->ntype == KSTR)
    {
        map_put(typeMap, left->varName, STRTYPE);
    } else if (right->ntype == VAR)
    {
        Type *ty = calloc(1, sizeof(Type));
        ty->type = ALIAS;
        ty->typeName = right->varName;
        map_put(typeMap, left->varName, ty);
    } else if (right->ntype == NRECORD)
    {
        Type *ty = calloc(1, sizeof(Type));
        ty->type = RECORD;
        ty->typeName = left->varName;
        map_put(typeMap, left->varName, ty);
    } else if (right->ntype == ARRAY)
    {
        Type *ty = calloc(1, sizeof(Type));
        ty->type = ARRAYTYPE;
        ty->typeName = left->varName;
        map_put(typeMap, left->varName, ty);
    } else
    {
        printf("err %d\n", __LINE__);
        returnERRTYPE(node->line);      
    }
}

Type *finalType(Map *typeMap, char *typeName, Set *set) {
    Type *ty = map_get(typeMap, typeName);
    if (ty == NULL)
    {
        printf("err %d\n", __LINE__);
        err = 0; 
        return UNDEFTYPE;
    }
    
    if (ty == INTTYPE)
    {
        return INTTYPE;
    } else if (ty == STRTYPE)
    {
        return STRTYPE;
    } else if (ty->type == ALIAS)
    {
        if (set_contaion(set, ty->typeName))
        {
            map_put(typeMap, typeName, ERRTYPE);
            return ERRTYPE;
        } else
        {
            set_add(set, ty->typeName);
            Type *fty = finalType(typeMap, ty->typeName, set);
            if (fty->type == ERR || fty == UNDEFTYPE)  {
                errormsg("semantic error \n");
                err = 0;
            }
            map_put(typeMap, typeName, fty);
            return fty;
        }    
    } else if (ty->type == RECORD)
    {
       return ty; 
    } else if (ty->type == ARRAYTYPE)
    {
        return ty;
    } else
    {
        printf("err %d\n", __LINE__);
        err = 0; 
        return UNDEFTYPE;
    } 
}

void findType(Node *node, Map *typeMap) {
    Type *ty = map_get(typeMap, node->left->varName);    
    if (ty->type == ALIAS)
    {
        if (!strcmp(node->left->varName, ty->typeName))
        {
            map_put(typeMap, node->left->varName, ERRTYPE);
            returnERRTYPE(node->line);                       
        } else
        {  
            Set *set = make_set();          
            set_add(set, node->left->varName);
            set_add(set, ty->typeName);
            Type *fty = finalType(typeMap, ty->typeName, set);
            if (fty->type == ERR || fty == UNDEFTYPE)  {
                returnERRTYPE(node->line);                       
            } 
            map_put(typeMap, node->left->varName, fty);     
        }
    } else if (ty->type == RECORD)
    {
        Map *recordType = make_map();
        Vector *vec = make_vector();
        findTyfields(node->right->tyfields, recordType, typeMap, vec);  
        ty->recordType = recordType;
        ty->fields = vec;
    } else if (ty->type == ARRAYTYPE)
    {
        Node *right = node->right;
        if (right->nextTy->ntype == KINT)
        {
            ty->eleType = INTTYPE;
        } else if (right->nextTy->ntype == KSTR)
        {
            ty->eleType = STRTYPE;
        } else if (right->nextTy->ntype == VAR)
        {
            Set *set = make_set();      
            set_add(set, right->nextTy->varName);    
            Type *fty = finalType(typeMap, right->nextTy->varName, set);
            if (fty->type == ERR || fty == UNDEFTYPE) {
                returnERRTYPE(node->line);                       
            }                
            ty->eleType = fty;
        }  
    }        
}

void findTyfields(Vector *vec, Map *map, Map *typeMap, Vector *fields) {
    int len = vec_len(vec);
    Set *set = make_set();
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        Node *left = node->left;
        Node *right = node->right;
        vec_push(fields, left->varName);
        if (right->ntype == KINT)
        {
            map_put(map, left->varName, INTTYPE);
        } else if (right->ntype == KSTR)
        {
            map_put(map, left->varName, STRTYPE);
        } else if (right->ntype == VAR)
        {
            set_add(set, right->varName);
            Type *fty = finalType(typeMap, right->varName, set);
            if (fty->type == ERR || fty == UNDEFTYPE) {
                returnERRTYPE(node->line);                       
            }
            map_put(map, left->varName, fty);
        }        
    }
}

Type *checkDecs(Vector *vec, Map *varMap, Map *typeMap) {
    Node *node = NULL;
    int len = vec_len(vec);

    for (int i = 0; i < len; i++)
    {
        node = vec_get(vec, i);
        if (node->ntype == DEC)
        {
            collectDecHead(node, typeMap);
        }              
    }

    Set *set = make_set();
    for (int i = 0; i<len; i++) {
        node = vec_get(vec, i);
        if (node->ntype == DEC)
        {
            findType(node, typeMap);
        }                        
    }

    level++;
    for (int i = 0; i<len; i++) {
        node = vec_get(vec, i);
        if (node->ntype == FUNCTION)
        {
            findFunSign(node, typeMap);
        }        
    }

    for (int i = 0; i<len; i++) {
        node = vec_get(vec, i);
        if (node->ntype == ASSIGN)
        {
            checkVardec(node, varMap, typeMap);
        }
        else if (node->ntype == FUNCTION)
        {
            Map *child = make_parent_map(varMap);
            fieldsToMap(node, child, typeMap);
            Type *ty = checkExp(node->body, child, typeMap);
            Type *funTy = map_get(typeMap, node->funName->varName);
            Type *ty1 = funTy->returnType;
            if (ty != ty1 && !(ty1->type == RECORD && ty ==NILTYPE) && (ty1 != VOIDTYPE)) {
                return returnERRTYPE(node->line);                       
            }
            node->type = funTy;
        }        
    }
    return VOIDTYPE;
}

void fieldsToMap(Node *node, Map *child, Map *typeMap) {
    Type *funTy = map_get(typeMap, node->funName->varName);
    Vector *args = funTy->args;
    Map *argsTy = funTy->argsType;
    int len = args == NULL ? 0 : vec_len(args);
    for (int i = 0; i<len; i++) {
        char *varName = vec_get(args, i);
        Type *ty = map_get(argsTy, varName);
        map_put(child, varName, ty);
    }
}

void checkVardec(Node *node, Map *varMap, Map *typeMap) {
    if (node->left->ntype == VAR)
    {
        Type *ty = checkExp(node->right, varMap, typeMap); 
        node->left->type = ty; 
        if (ty == NILTYPE || ty == VOIDTYPE)
        {
            returnERRTYPE(node->line);                       
        }              
        map_put(varMap, node->left->varName, ty);
    }
    else if (node->left->ntype == COLON)
    {
        Node *colon = node->left;
        Type *ty = checkExp(node->right, varMap, typeMap);
        Type *ty1 = findType1(colon->right, typeMap);

        if ((ty == ty1 && ty1 != VOIDTYPE && ty != VOIDTYPE) || (ty1->type == RECORD && ty == NILTYPE)) { 
            colon->left->type = ty1;
            node->left = colon->left;           
            map_put(varMap, colon->left->varName, ty1);
        } else
        {
            returnERRTYPE(node->line);                       
        }        
    }    
}

Type *findType1(Node *node, Map *typeMap) {
    if (node->ntype == KINT)
    {
        return INTTYPE;
    } else if (node->ntype == KSTR)
    {
        return STRTYPE;   
    } else if (node->ntype == VAR)
    {
        Type *ty = map_get(typeMap, node->varName);
        if (ty == NULL || (ty->type == ALIAS || ty->type == VOID || ty->type == ERR))
        {
            printf("err %d\n", __LINE__);                                   
            return returnERRTYPE(node->line);
        }
        return ty;        
    } else
    {
        printf("err %d\n", __LINE__);
        err = 0; 
        return UNDEFTYPE;
    }         
}

Type *checkExps(Vector *vec, Map *varMap, Map *typeMap) {
    int len = vec_len(vec);
    Type *ty = NULL;
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        ty = checkExp(node, varMap, typeMap);
    }
    return ty;
}

Type *checkLvalue(Node *node, Map *varMap, Map *typeMap) {
    Type *ty = NULL;
    if (node->ntype == VAR)
    {
        ty = map_get(varMap, node->varName); 
    }
    else if (node->ntype == DOT)
    {
        Type *ty1 = checkLvalue(node->left, varMap, typeMap);
        ty = map_get(ty1->recordType, node->right->varName);
    }
    else if (node->ntype == ARROP)
    {
        Type *ty1 = checkLvalue(node->left, varMap, typeMap);
        ty = ty1->eleType;
    }
    if (ty == NULL) {
        return returnERRTYPE(node->line);
    }
    node->type = ty;       

    return ty;
}

Type *checkExp(Node *node, Map *varMap, Map *typeMap) {
    if (node == NULL) {
        printf("err %d\n", __LINE__);
        err = 0; 
        return UNDEFTYPE;
    }

    if (node->ntype == VAR || node->ntype == DOT || node->ntype == ARROP)
    {
        Type *ty = checkLvalue(node, varMap, typeMap);
        node->type = ty;
        return ty;
    }
    else if (node->ntype == PLUS || node->ntype == MINUS 
            || node->ntype == TIMES || node->ntype == DIVIDE 
            || node->ntype == OR || node->ntype == AND 
            || node->ntype == LT || node->ntype == LE
            || node->ntype == GT || node->ntype == GE)
    {
        Type *ty1 = checkExp(node->left, varMap, typeMap);
        Type *ty2 = checkExp(node->right, varMap, typeMap);
        if (ty1 != ty2 || ty1 != INTTYPE || ty2 != INTTYPE) {
            return returnERRTYPE(node->line);
        }
        node->type = ty1;
        return ty1;            
    }
    else if (node->ntype == NINT)
    {
        node->type = INTTYPE;
        return INTTYPE;
    } else if (node->ntype == EQ || node->ntype == NEQ)
    {
        Type *ty1 = checkExp(node->left, varMap, typeMap);
        Type *ty2 = checkExp(node->right, varMap, typeMap);        
        if (ty1 == NILTYPE && ty2 == NILTYPE)
        {
            return returnERRTYPE(node->line);
        }                 
        node->type = INTTYPE;
        return INTTYPE;
    } else if (node->ntype == NSTRING)
    {
        node->type = STRTYPE;
        return STRTYPE;
    } else if (node->ntype == ARRAY)
    {
        Type *ty = map_get(typeMap, node->arrType);
        Type *arrLen = checkExp(node->arrlen, varMap, typeMap);
        Type *initArr = checkExp(node->initArr, varMap, typeMap);                
        if (ty == NULL || ty->type != ARRAYTYPE || arrLen != INTTYPE || initArr != ty->eleType) {
            return returnERRTYPE(node->line);
        }
        node->type = ty;
        return ty;
    } else if (node->ntype == NRECORD)
    {
        Type *ty = map_get(typeMap, node->recodeType);
        if (ty == NULL || ty->type != RECORD) {
            return returnERRTYPE(node->line);
        }
        checkFields(ty, node->tyfields, typeMap, varMap);
        node->type = ty;
        return ty;
    } else if (node->ntype == NIL)
    {
        node->type = NILTYPE;
        return NILTYPE;
    } else if (node->ntype == ASSIGN)
    {
        Type *ty1 = checkExp(node->left, varMap, typeMap);
        Type *ty2 = checkExp(node->right, varMap, typeMap);
        if (ty1 != ty2 || ty1 == VOIDTYPE || ty2 == VOIDTYPE || ty1 == READONLY)
        {
            return returnERRTYPE(node->line);
        }
        node->type = VOIDTYPE;
        return VOIDTYPE;
    } else if (node->ntype == NEGATIVE)
    {
        Type *ty = checkExp(node->unary, varMap, typeMap);
        if (ty != INTTYPE)
        {
            return returnERRTYPE(node->line);
        }
        node->type = ty;
        return ty;        
    } else if (node->ntype == SEQ)
    {
        Vector *vec = node->sequence;
        int len = vec_len(vec);
        for (int i = 0; i<len - 1; i++) {
            checkExp(vec_get(vec, i), varMap, typeMap);
        }
        Type *ty = checkExp(vec_get(vec, len-1), varMap, typeMap);
        node->type = ty;
        return ty;
    } else if (node->ntype == COND)
    {
        Type *condTy = checkExp(node->cond, varMap, typeMap);
        if (condTy != INTTYPE)        
            returnERRTYPE(node->line);        
        
        Type *thenTy = checkExp(node->then, varMap, typeMap);
        if (node->els != NULL)
        {
            Type *elsTy = checkExp(node->els, varMap, typeMap);
            if (thenTy != elsTy && !((thenTy == NILTYPE && elsTy->type == RECORD) || 
                (elsTy == NILTYPE && thenTy->type == RECORD)))            
                return returnERRTYPE(node->line);
            else {
                node->type = thenTy;
                return thenTy; 
            }                         
        } else
        {
            return VOIDTYPE;
        }        
    } else if (node->ntype == WHILE)
    {
        Type *ty = checkExp(node->exp1, varMap, typeMap);
        checkExp(node->exp2, varMap, typeMap);
        if (ty != INTTYPE)
            return returnERRTYPE(node->exp1->line);
        
        return VOIDTYPE;
    } else if (node->ntype == BREAK)
    {
        return VOIDTYPE;
    } else if (node->ntype == FOR)
    {
        node->exp1->left->type = READONLY;
        Type *ty2 = checkExp(node->exp1->right, varMap, typeMap);
        Type *ty3 = checkExp(node->exp2, varMap, typeMap);

        Map *varMap1 = make_parent_map(varMap);
        map_put(varMap1, node->exp1->left->varName, INTTYPE);

        checkExp(node->exp3, varMap1, typeMap);
        if (ty2 != INTTYPE || ty3 != INTTYPE)
            return returnERRTYPE(node->exp1->line);
        return VOIDTYPE;        
    } else if (node->ntype == LET)
    {
        Map *varMap1 = make_parent_map(varMap);
        Map *typeMap1 = make_parent_map(typeMap);

        return checkLet(node, varMap1, typeMap1);
    } else if (node->ntype == CALL)
    {
        Type *ty = map_get(typeMap, node->callName->varName);
        if (ty == NULL) {
            returnERRTYPE(node->line);
            return NULL;
        }
        Vector *real = node->args;
        Vector *form = ty->args;
        int len = real == NULL ? 0 : vec_len(real);
        int flen = form == NULL ? 0 : vec_len(form);

        if (len != flen)
            returnERRTYPE(node->line);

        for (int i = 0; i<len; i++) {
            Type *ty1 = map_get(ty->argsType, vec_get(form, i));
            Type *ty2 = checkExp(vec_get(real, i), varMap, typeMap);
            if (ty1 != ty2)
            {
                returnERRTYPE(node->line);                
            }            
        }
        
        node->type = ty;
        return ty->returnType;
    }    
    else
    {
        printf("err %d\n", __LINE__);
        return returnERRTYPE(node->line);
    }
    
    return NULL;
}

void checkFields(Type *ty, Vector *vec, Map *typeMap, Map *varMap) {
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        Node *left = node->left;
        Node *right = node->right;
        Type *ty1 = map_get(ty->recordType, left->varName);
        Type *ty2 = checkExp(right, varMap, typeMap);
        if (ty1 != ty2 && !(ty1->type == RECORD && ty2 == NILTYPE)) {           
            returnERRTYPE(node->line);
        }
        left->type = ty1;
    }
}
