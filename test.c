#include<stdio.h>
#include<assert.h>
#include "tiger.h"

void testTydec(Node *node);
void testExp(Node *node);
void printOp(Node *node, char *op);

void testTyfields(Vector *vec) {    
    int len = vec_len(vec);    
    for(int i = 0; i<len; i++) {
        Node *node = (Node *)vec_get(vec, i);
        Node *right = node->right;
        if (node->left->ntype == VAR) {
            printf("%s : ", node->left->varName);
        } else
        {
            printf("err %d\n", __LINE__);
        }

        if (right->ntype == KINT)
        {
            printf("int, ");
        }
        else if (right->ntype == KSTR)
        {
            printf("string, ");
        } else if (right->ntype == VAR)
        {
            testExp(right);
            printf(", ");
        }
    }
}

void testTydec(Node *node) {
    Node *left = node->left;
    Node *right = node->right;
    if (left->ntype == VAR)
    {
        printf(" %s = ", left->varName);
    }
    if (right->ntype == VAR)
    {
        printf(" %s\n", right->varName);
    } else if (right->ntype == KINT)
    {
        printf(" int\n");
    } else if (right->ntype == KSTR)
    {
        printf(" string\n");
    } else if (right->ntype == NRECORD)
    {
        printf(" { ");
        testTyfields(right->tyfields);
        printf(" } \n");
    } else if (right->ntype == ARRAY)
    {
        if (right->nextTy->ntype == VAR)
        {
            printf("array of %s\n", right->nextTy->varName);
        } else if (right->nextTy->ntype == KINT)
        {
            printf("array of int\n");            
        } else if (right->nextTy->ntype == KSTR)
        {
            printf("array of string\n");          
        } else
        {
            printf("err %d\n", __LINE__);
        }   
    } else
    {
            printf("err %d\n", __LINE__);        
    }   
}

void testTyid(Node *node) {
    if (node->ntype == KINT)
    {
        printf("int ");
    }
    else if (node->ntype == KSTR)
    {
        printf("string ");
    }
    else if (node->ntype == VAR)
    {
        printf("%s ", node->varName);
    }    
}

void testId(Node *node) {
    if (node->ntype == VAR)
    {
        printf("%s ", node->varName);
    }    
}
void testExp1(Node *node) {
    if (node->ntype == SEQ)
    {
        printf("exp \n");
    }    
}
void testVardec(Node *node) {
    if (node->ntype == ASSIGN)
    {
        if (node->left->ntype == VAR)
        {
            testId(node->left);
            printf(":= ");
            testExp(node->right);
        }
        else if (node->left->ntype == COLON)
        {
            Node *colon = node->left;
            testId(colon->left);
            printf(": ");
            testTyid(colon->right);
            printf(":= ");
            testExp(node->right);
        }
    }
    printf("\n");
}

void testDecs(Vector *vec) {
    Node *node = NULL;
    int len = vec_len(vec);
    for (int i = 0; i<len; i++) {        
        node = (Node *)vec_get(vec, i);
        if (node->ntype == DEC)
        {
            printf("type ");
            testTydec(node);
        } else if (node->ntype == ASSIGN)
        {
            printf("var ");
            testVardec(node);
        } else if (node->ntype == FUNCTION)
        {
            printf("function ");
            testExp(node->funName);
            printf("(");
            if (node->fargs != NULL)
            {
                testTyfields(node->fargs);
            }
            printf(") ");
            if (node->returnType != NULL)
            {
                printf(": ");
                testTyid(node->returnType);
                printf(" ");
            }
            printf("= ");
            testExp(node->body); 
        } else
        {
            printf("error decs");
        }
    }
}

Node *testVar(Node *node) {
    assert(node->ntype == VAR);
    return node;
}

void testLvalue(Node *node) {
    if (node->ntype == VAR)
    {
        printf("%s", node->varName);
    }
    else if (node->ntype == DOT)
    {
        testExp(node->left);
        printf(".");
        testExp(node->right);
    }
    else if (node->ntype == ARROP)
    {
        testExp(node->left);
        printf("[");
        testExp(node->right);
        printf("]");
    }    
}

void printOp(Node *node, char *op) {
    printf("(%s ", op);
    testExp(node->left);
    printf(" ");
    testExp(node->right);
    printf(")");
}

void testExp(Node *node) {
    if (node == NULL)
        return;
    
    if (node->ntype == OR)
    {
        printOp(node, "|");
    } else if (node->ntype == VAR || node->ntype == DOT || node->ntype == ARROP)
    {
        testLvalue(node);
    } else if (node->ntype == AND)
    {
        printOp(node, "&");    
    } else if (node->ntype == EQ)
    {
        printOp(node, "=");
    } else if (node->ntype == NEQ)
    {
        printOp(node, "<>");
    } else if (node->ntype == LT)
    {
        printOp(node, "<");
    } else if (node->ntype == LE)
    {
        printOp(node, "<=");
    } else if (node->ntype == GT)
    {
        printOp(node, ">");
    } else if (node->ntype == GE)
    {
        printOp(node, ">=");
    } else if (node->ntype == PLUS)
    {
        printOp(node, "+");
    } else if (node->ntype == MINUS)
    {
        printOp(node, "-");
    } else if (node->ntype == TIMES)
    {
        printOp(node, "*");
    } else if (node->ntype == DIVIDE)
    {
        printOp(node, "/");
    } else if (node->ntype == NINT)
    {
        printf("%d", node->ival);
    } else if (node->ntype == NSTRING)
    {
        printf("\"%s\"", node->strVal);
    } else if (node->ntype == ARRAY)
    {
        printf("%s", node->arrType);
        printf("[");
        testExp(node->arrlen);
        printf("]");
        printf(" of ");
        testExp(node->initArr);
    } else if (node->ntype == NRECORD)
    {
        printf("%s", node->recodeType);
        printf("{ ");
        testTyfields(node->tyfields);
        printf("} ");
    } else if (node->ntype == NEGATIVE)
    {
        printf("-");
        testExp(node->unary);
    } else if (node->ntype == SEQ)
    {
        Vector *vec = node->sequence;
        int len = vec_len(vec);
        printf("(");
        for (int i = 0; i<len-1; i++) {
            testExp(vec_get(vec, i));
            printf("; ");
        }
        testExp(vec_get(vec, len-1));
        printf(")");
    } else if (node->ntype == NIL)
    {
        printf("nil");
    } else if (node->ntype == ASSIGN)
    {
        printOp(node, ":=");        
    } else if (node->ntype == COND)
    {
        printf("if ");
        testExp(node->cond);
        printf(" then \n");
        testExp(node->then);
        if (node->els != NULL)
        {
            printf("\nelse\n");
            testExp(node->els);
        }        
    } else if (node->ntype == WHILE)
    {
        printf("while ");
        testExp(node->exp1);
        printf("\ndo ");
        testExp(node->exp2);
    } else if (node->ntype == BREAK)
    {
        printf("break");
    } else if (node->ntype == FOR)
    {
        printf("for ");
        testExp(node->exp1);
        printf(" to ");
        testExp(node->exp2);
        printf("\ndo ");
        testExp(node->exp3);
    }else if (node->ntype == LET)
    {
        testLet(node);
    } else if (node->ntype == CALL)
    {
        testExp(node->callName);
        printf("(");
        if (node->args != NULL)
        {
            Vector *vec = node->args;
            int len = vec_len(vec);
            for (int i = 0; i < len; i++)
            {
                testExp(vec_get(vec, i));
                printf(", ");
            }             
        }
        printf(")");
    }    
    else
    {
        printf("\nerror exp\n");
    }   
}

void testExps(Vector *vec) {
    int len = vec_len(vec);
    for (int i = 0; i < len; i++) {
        Node *node = vec_get(vec, i);                   
        testExp(node);
        printf("\n");         
    }
}

void testLet(Node *node) {
    if (node->ntype == LET)
    {
        printf("let\n");
        testDecs(node->decs);
        printf("\nin\n");
        testExps(node->expseq);

        printf("\nend\n");        
    } else
    {
        printf("err %d\n", __LINE__);
    }
    
    
}
