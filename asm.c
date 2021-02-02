#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tiger.h"

void genAsmTiger(Node *node);
void genLet(Node *node);
void genExps(Vector *vec);
Type *genExp(Node *node);
void genDecs(Vector *vec);
void genFun(Vector *vec);
void asmStr(Vector *vec);
void genFields(Node *node);

static int funName = 0;

Type *genExp(Node *node) {
    if (node->ntype == NINT)
    {
        printf("\tmov   $%d, %%eax\n", node->ival);
        return node->type;
    } else if (node->ntype == NEGATIVE)
    {
        genExp(node->unary);
        printf("\tmov   %%eax, %%ebx\n");
        printf("\tmov   $0, %%eax\n");
        printf("\tsub   %%ebx, %%eax\n");
    }
    else if (node->ntype == NSTRING)
    {
        printf("\tleaq   .LC%d(%%rip), %%rax\n", node->strLabel);
        return node->type;
    } else if (node->ntype == PLUS || node->ntype == MINUS || node->ntype == TIMES || 
            node->ntype == DIVIDE || node->ntype == LT || node->ntype == LE || 
            node->ntype == GT || node->ntype == GE || node->ntype == OR || node->ntype == AND)
    {
        genExp(node->left);
        printf("\tpush  %%rax\n");
        genExp(node->right);
        printf("\tmov   %%eax, %%ebx\n");
        printf("\tpop   %%rax\n");
        //eax op ebx
        if (node->ntype == PLUS)        
            printf("\tadd   %%ebx, %%eax\n");
        else if (node->ntype == MINUS) {
            printf("\tsub   %%ebx, %%eax\n");
        } else if (node->ntype == TIMES) 
            printf("\timull %%ebx, %%eax\n");
        else if (node->ntype == DIVIDE)
        {
            printf("\tcltd\n");
            printf("\tidivl %%ebx\n");
        } else if (node->ntype == LT)
        {
            printf("\tcmp   %%ebx, %%eax\n");
            printf("\tsetl  %%al\n");
            printf("\tmovzbl    %%al, %%eax\n");
        } else if (node->ntype == LE)
        {
            printf("\tcmp   %%ebx, %%eax\n");
            printf("\tsetle  %%al\n");
            printf("\tmovzbl    %%al, %%eax\n");
        } else if (node->ntype == GT)
        {
            printf("\tcmp   %%ebx, %%eax\n");
            printf("\tsetg  %%al\n");
            printf("\tmovzbl    %%al, %%eax\n");
        } else if (node->ntype == GE)
        {
            printf("\tcmp   %%ebx, %%eax\n");
            printf("\tsetge  %%al\n");
            printf("\tmovzbl    %%al, %%eax\n");
        } else if (node->ntype == AND)
        {
            char *L1 = make_label();
            char *L2 = make_label();
            printf("\tcmp   $0, %%ebx\n");
            printf("\tje    %s\n", L1);
            printf("\tcmp   $0, %%eax\n");
            printf("\tje    %s\n", L1);
            printf("\tmov   $1, %%eax\n");
            printf("\tjmp   %s\n", L2);
            printf("%s:\n", L1);
            printf("\tmov   $0, %%eax\n");
            printf("%s:\n", L2);
        } else if (node->ntype == OR)
        {
            char *L1 = make_label();
            char *L2 = make_label();
            char *L3 = make_label();
            printf("\tcmp   $0, %%ebx\n");
            printf("\tjne    %s\n", L1);
            printf("\tcmp   $0, %%eax\n");
            printf("\tje    %s\n", L2);
            printf("%s:\n", L1);
            printf("\tmov   $1, %%eax\n");
            printf("\tjmp   %s\n", L3);
            printf("%s:\n", L2);
            printf("\tmov   $0, %%eax\n");
            printf("%s:\n", L3);
        }             
        return INTTYPE;    
    } else if (node->ntype == EQ || node->ntype == NEQ)
    {
        Type *ty1 = genExp(node->left);
        printf("\tpush  %%rax\n");
        Type *ty2 = genExp(node->right);
        if (node->ntype == EQ) {
            if (ty1 == INTTYPE && ty2 == INTTYPE) {
                printf("\tmov   %%eax, %%ebx\n");
                printf("\tpop   %%rax\n");
                printf("\tcmp   %%ebx, %%eax\n");
            } else if (ty1 == STRTYPE && ty2 == STRTYPE)
            {
                printf("\tmov   %%rax, %%rdx\n");
                printf("\tpop   %%rcx\n");
                printf("\tcall  strcmp\n");
                printf("\tcmp   $0, %%eax\n");
            } else
            {
                printf("\tmov   %%rax, %%rbx\n");
                printf("\tpop   %%rax\n");
                printf("\tcmp   %%rbx, %%rax\n");
            }
            printf("\tsete  %%al\n");
            printf("\tmovzbl    %%al, %%eax\n");
                         
        } else if (node->ntype == NEQ)
        {
            if (ty1 == INTTYPE && ty2 == INTTYPE) {
                printf("\tmov   %%eax, %%ebx\n");
                printf("\tpop   %%rax\n");
                printf("\tcmp   %%ebx, %%eax\n");
            } else if (ty1 == STRTYPE && ty2 == STRTYPE)
            {
                printf("\tmov   %%rax, %%rdx\n");
                printf("\tpop   %%rcx\n");
                printf("\tcall  strcmp\n");
                printf("\tcmp   $0, %%eax\n");
            } else
            {
                printf("\tmov   %%rax, %%rbx\n");
                printf("\tpop   %%rax\n");
                printf("\tcmp   %%rbx, %%rax\n");
            }
            printf("\tsetne  %%al\n");
            printf("\tmovzbl    %%al, %%eax\n");
        }
    } else if (node->ntype == VAR)
    {
        if (node->level == 0)
        {
            if (node->offset >= 0)
                printf("\tmov   %d(%%rbp), %%rax\n", node->offset);
            else
                printf("\tmov   %d(%%rbp), %%rax\n", node->offset);
        } else
        {
            printf("\tmov   16(%%rbp), %%rax\n");
            for (int i = node->level - 1; i>0; i--) {
                printf("\tmov   16(%%rax), %%rax\n");
            }
            if (node->offset >= 0)
                printf("\tmov   %d(%%rax), %%rax\n", node->offset);
            else
                printf("\tmov   %d(%%rax), %%rax\n", node->offset);
        }
        return node->type; 
    } else if (node->ntype == LET)
    {
        printf("\tpush  %%rbp\n");
        genLet(node);
        printf("\taddq  $8, %%rsp\n");            
    } else if (node->ntype == ASSIGN)
    {
        genExp(node->right);
        if (node->left->ntype == VAR)
        {
            if (node->left->level == 0) {
                if (node->left->offset >= 0)                
                    printf("\tmov   %%rax, %d(%%rbp)\n", node->left->offset);                    
                else
                    printf("\tmov   %%rax, %d(%%rbp)\n", node->left->offset);                    
            }            
            else
            {
                printf("\tmov   %%rax, %%rbx\n");
                printf("\tmov   16(%%rbp), %%rax\n");
                for (int i = node->left->level - 1; i>0; i--) {
                    printf("\tmov   16(%%rax), %%rax\n");
                }
                if (node->left->offset >= 0)               
                    printf("\tmov   %%rbx, %d(%%rax)\n", node->left->offset);
                else
                    printf("\tmov   %%rbx, %d(%%rax)\n", node->left->offset);
            }            
        } else if (node->left->ntype == ARROP)
        {
            printf("\tpush  %%rax\n");
            genExp(node->left->left);
            printf("\tpush  %%rax\n");
            genExp(node->left->right);
            printf("\tpop   %%rbx\n");
            printf("\tpop   %%r8\n");
            if (node->left->type == INTTYPE) 
                printf("\tmov   %%r8d, (%%rbx, %%rax, 4)\n");            
            else 
                printf("\tmov   %%r8, (%%rbx, %%rax, 8)\n");              
        } else if (node->left->ntype == DOT)
        {
            printf("\tpush  %%rax\n");
            genExp(node->left->left);
            printf("\tpop   %%rbx\n");
            if (node->left->type == INTTYPE)
                printf("\tmov   %%ebx, %d(%%rax)\n", node->right->offset);
            else
                printf("\tmov   %%rbx, %d(%%rax)\n", node->right->offset);
        }
                  
    } else if (node->ntype == CALL)
    {
        Vector *real = node->args;
        int len = real == NULL ? 0 : vec_len(real);

        for (int i = 0; i<len; i++) {
            Node *node = vec_get(real, i);
            genExp(node);
            printf("\tpush  %%rax\n");
        }
        char *fname = node->callName->varName;

        if (!strcmp(fname, "flush"))
        {
            printf("\tmovl	$1, %%ecx\n");
            printf("\tmovq	__imp___acrt_iob_func(%%rip), %%rax\n");
            printf("\tcall	*%%rax\n");
            printf("\tmovq	%%rax, %%rcx\n");
            printf("\tcall	fflush\n");
        } else if (!strcmp(fname, "getchar"))
        {
            printf("\tcall  tgetchar\n");
            if (len > 0)
                printf("\taddq  $%d, %%rsp\n", len * 8); 
        } else if (!strcmp(fname, "substring"))
        {
            printf("\tpop   %%r8\n");
            printf("\tpop   %%rdx\n");
            printf("\tpop   %%rcx\n");
            printf("\tcall  substring\n");
        }  else if (!strcmp(fname, "not"))
        {
            printf("\tpop   %%rax\n");
            printf("\ttestl %%eax, %%eax\n");
            printf("\tsete %%al\n");
            printf("\tmovzbl %%al, %%eax\n");
        } else if (!strcmp(fname, "exit"))
        {
            printf("\tpop   %%rcx\n");
            printf("\tcall  exit\n");
        } else {
            if (!strcmp(fname, "print") || !strcmp(fname, "ord") || !strcmp(fname, "ord") || 
                !strcmp(fname, "chr") || !strcmp(fname, "size") || !strcmp(fname, "concat")) {
                printf("\tcall  %s\n", node->callName->varName);

                if (len > 0)
                    printf("\taddq  $%d, %%rsp\n", len * 8);
            } else {
                Type *ty = node->type;
                if (ty->level == funName)
                    printf("\tpush  16(%%rbp)\n");
                else
                    printf("\tpush  %%rbp\n");
                printf("\tcall  %s\n", node->callName->varName);
                printf("\taddq  $%d, %%rsp\n", len * 8 + 8);
            } 
        }
    } else if (node->ntype == COND)
    {
        char *L1 = make_label();        
        genExp(node->cond);
        printf("\tcmp   $1, %%eax\n");
        printf("\tjne    %s\n", L1);
        genExp(node->then);
        if (node->els != NULL) {
            char *L2 = make_label();
            printf("\tjmp   %s\n", L2);
            printf("%s:\n", L1);
            genExp(node->els);
            printf("%s:\n", L2);
        } else
        {
            printf("%s:\n", L1);            
        }
    } else if (node->ntype == WHILE)
    {
        char *L1 = make_label();
        char *L2 = make_label();
        printf("\tjmp   %s\n", L1);
        printf("%s:\n", L2);
        genExp(node->exp2);
        printf("%s:\n", L1);
        genExp(node->exp1);
        printf("\tcmp   $1, %%eax\n");
        printf("\tje    %s\n", L2);
        printf("%s:\n", node->lbreak);
    } else if (node->ntype == BREAK)
    {
        printf("\tjmp   %s\n", node->label);        
    } else if (node->ntype == FOR)
    {
        //rbx-rax
        char *L1 = make_label();
        char *L2 = make_label();
        genExp(node->exp1->right);
        printf("\tmov   %%rax, %d(%%rbp)\n", node->exp1->left->offset);
        genExp(node->exp2);
        printf("\tpush  %%rax\n");
        printf("\tjmp   %s\n", L1);
        printf("%s:\n", L2);
        genExp(node->exp3);
        printf("\tmov   %d(%%rbp), %%rax\n", node->exp1->left->offset);
        printf("\tadd   $1, %%eax\n");
        printf("\tmov   %%rax, %d(%%rbp)\n", node->exp1->left->offset);
        printf("%s:\n", L1);
        printf("\tpop   %%rax\n");
        printf("\tpush  %%rax\n");
        printf("\tcmpl  %d(%%rbp), %%eax\n", node->exp1->left->offset);
        printf("\tjge   %s\n", L2);
        printf("\tpop   %%rax\n");
        printf("%s:\n", node->lbreak);
    } else if (node->ntype == SEQ)
    {
        Vector *vec = node->sequence;
        int len = vec_len(vec);
        for (int i = 0; i<len; i++)
            genExp(vec_get(vec, i));
    } else if (node->ntype == NIL)
    {
        printf("\tmov   $0, %%rax\n");
    } else if (node->ntype == ARRAY)
    {
        char *L1 = make_label();
        char *L2 = make_label();

        genExp(node->arrlen);
        printf("\tpush  %%rax\n");
        if (node->type->eleType == INTTYPE)
            printf("\tmov   $4, %%edx\n"); 
        else
            printf("\tmov   $8, %%edx\n");
        printf("\tmov   %%eax, %%ecx\n"); 
        printf("\tcall  calloc\n");
        printf("\tmov   %%rax, %%rbx\n");  
        genExp(node->initArr);              
        printf("\tmov   $0, %%r8\n");      
        printf("\tpop   %%rcx\n");
        printf("\tjmp   %s\n", L1);
        printf("%s:\n", L2);
        if (node->type->eleType == INTTYPE) 
            printf("\tmov   %%eax, (%%rbx, %%r8, 4)\n");
        else 
            printf("\tmov   %%rax, (%%rbx, %%r8, 8)\n");
        printf("\tadd   $1, %%r8d\n");
        printf("%s:\n", L1);
        printf("\tcmp   %%ecx, %%r8d\n");
        printf("\tjl    %s\n", L2);
        printf("\tmov   %%rbx, %%rax\n");
    } else if (node->ntype == ARROP)
    {
        genExp(node->left);
        printf("\tpush  %%rax\n");
        genExp(node->right);
        printf("\tpop   %%rbx\n");
        if (node->type == INTTYPE)
            printf("\tmov   (%%rbx, %%rax, 4), %%eax\n");
        else   
            printf("\tmov   (%%rbx, %%rax, 8), %%rax\n");   
    } else if (node->ntype == NIL)
    {
        printf("\tmov   $0, %%rax\n");
    }
    else if (node->ntype == NRECORD)
    {
        genFields(node);
    } else if (node->ntype == DOT)
    {
        genExp(node->left);
        if (node->type == INTTYPE)
            printf("\tmov   %d(%%rax), %%eax\n", node->right->offset);
        else
            printf("\tmov   %d(%%rax), %%rax\n", node->right->offset);
    }
}
void genFields(Node *node) {
    Vector *vec = node->tyfields;
    int len = vec_len(vec);
    Map *tymap = node->type->offsetMap;
    printf("\tmov   $%d, %%edx\n", node->type->len);
    printf("\tmov   $1, %%ecx\n");
    printf("\tcall  calloc\n");
    printf("\tpush  %%rax\n");
    for (int i = 0; i<len; i++) {
        Node *n = vec_get(vec, i);
        genExp(n->right);
        printf("\tmov   %%rax, %%rbx\n");
        printf("\tpop   %%rax\n");
        int off = *((int *)map_get(tymap, n->left->varName));
        if (n->left->type == INTTYPE)
            printf("\tmov   %%ebx, %d(%%rax)\n", off);
        else
            printf("\tmov   %%rbx, %d(%%rax)\n", off);
        printf("\tpush  %%rax\n");  
    }
    printf("\tpop   %%rax\n");
}
void genExps(Vector *vec) {
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        genExp(node);
    }
}

void genDecs(Vector *vec) {
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        if (node->ntype == ASSIGN)
        {
            genExp(node->right);  
            printf("\tmov   %%rax, %d(%%rbp)\n", node->left->offset);
        }
    }
}

void genLet(Node *node) {
    printf("\tpushq %%rbp\n");
    printf("\tmovq  %%rsp, %%rbp\n");
    printf("\tsubq  $%d, %%rsp\n", node->localVarLen + 32);
    genDecs(node->decs);
    genExps(node->expseq);
    printf("\taddq  $%d, %%rsp\n", node->localVarLen + 32);
    printf("\tpopq  %%rbp\n");
}

void genAsmTiger(Node *node) {
    printf("\t.globl main\n");
    printf("main:\n");
    funName = 0;
    genLet(node);
    printf("\tret\n\n");

    genFun(node->decs);
}

void genFun(Vector *vec) {
    if (vec == NULL)
        return;
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        if (node->ntype == FUNCTION) {
            printf("%s:\n", node->funName->varName);
            funName = node->type->level;
            if (node->body->ntype == LET) {
                genLet(node->body);
                printf("\tret\n\n");
                genFun(node->body->decs);
            }
            else {
                printf("\tpushq %%rbp\n");
                printf("\tmovq  %%rsp, %%rbp\n");
                printf("\tsubq  $%d, %%rsp\n", node->loopVar + 32);
                genExp(node->body);
                printf("\taddq  $%d, %%rsp\n", node->loopVar + 32);
                printf("\tpopq  %%rbp\n");
                printf("\tret\n\n");
            }
        }
    }
}

void asmStr(Vector *vec) {
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {
        Node *node = vec_get(vec, i);
        printf(".LC%d:\n", node->strLabel);
        printf("\t.ascii \"");
        char *s = node->strVal;
        while (*s != '\0')
        {
            if (*s == '\n')
                printf("\\12");
            else if (*s == '\t')
                printf("\\11");
            else if (*s == '\\')
                printf("\\\\");
            else if (*s == '\"')
                printf("\\\"");
            else
                putchar(*s);
            s++;
        }
        printf("\\0\"\n");        
    }
}

void genAsm(Frame *fm) {
    asmStr(fm->stVec);
    printf("\t.text\n");
    genAsmTiger(fm->node);

}

