#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiger.h"

static int err = 0;

Node *parserTyid();
Node *parserId();
Vector *parserTyfields();
Node *parserTiger();
void paserDecs(Vector *vec);
Node *parserVardec();
Node *parserTydec();
Node *paserTy();
Node *parserAssignOp();
void parserExps(Vector *vec);
Node *parserDot();
Node *parserLbrack();
Node *parserLvalue();
Node *parserInt();
Node *parserArt();
Node *parserCompare();
Node *parserOr();
Node *parserPlusMinus();
Node *parserAnd();
Node *parserAndOp();
Node *parserComOp();
Node *parserPM();
Node *parserTimDiv();
Node *parserFactor();
Node *parserTD();
Node *parserinitArray(Node *tyNode, Node *arrLen);
Node *parserInitRecode(Node *node);
Node *parserExp();
Node *parserBranch();
Node *parserBreak();
Node *parserFundec();
Node *parserCall(Node *callName); 

void printErr(int line);

static char *lbreak = NULL;
static int strLabel = 1;

char *make_label() {
    static int c = 0;
    char *a = calloc(32, sizeof(char));
    sprintf(a, ".L%d", c++);
    return a;
}

Node *parser()
{
    Node *node = parserTiger();
    if (err == 0)    
        return node;
     
    return NULL;
};

void printErr(int line) {
    err = 1;
    errormsg("syntax error %d\n", line);
    Token *token = getToken();
    while (token->line == line && !(token->ttype == T_THEN || token->ttype == T_DO 
            || token->ttype == T_TO || token->ttype == T_SEMICOLON) && token != TNUL)
        token = getToken();
    ugetToken(token);     
}

Node *parserId() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_ID)
    {
        node->ntype = VAR;
        node->varName = token->str;
        node->line = token->line;
    } else
    {
        printErr(token->line);
    }
    return node;    
}

Vector *parserTyfields() {
    Token *token = getToken();
    Vector *vec = NULL;
    if (token->ttype == T_ID)
    {
        vec = make_vector();
        Node *node = NULL;
        while (token != TNUL && token->ttype != T_RBRACE && token->ttype != T_RPAREN)
        {
            if (token->ttype != T_ID) {
                printErr(token->line);              
                while (token != TNUL && token->ttype != T_RBRACE && token->ttype != T_RPAREN)
                    token = getToken();
                break;
            }
                
            ugetToken(token);
            Node *left = parserId();
            token = getToken();
            if (token->ttype == T_COLON)
            {
                Node *right = parserTyid();

                node = calloc(1, sizeof(Node));            
                node->ntype = COLON;
                node->left = left;
                node->right = right;
                node->line = token->line;
                vec_push(vec, node);                
            } else
            {
                printErr(token->line);
                return vec;
            }
            token = getToken();
            if (token->ttype == T_RBRACE || token->ttype == T_RPAREN)
            {
                ugetToken(token);
                return vec;
            } else if (token->ttype == T_COMMA)
            {
                token = getToken();
            } else
                printErr(token->line);               
        }
    } else if (token->ttype == T_RBRACE || token->ttype == T_RPAREN)
    {
        ugetToken(token);
        return NULL;
    } else
        printErr(token->line);
    return NULL;
}

Node *paserTy() {
    Token *token = getToken();
    Node *node = NULL;
    if (token->ttype == T_ID || token->ttype == T_KINT || token->ttype == T_KSTR)
    {
        ugetToken(token);
        node = parserTyid();
    }
    else if (token->ttype == T_LBRACE)
    {
        node = calloc(1, sizeof(Node));
        Vector *vec = parserTyfields();
        node->ntype = NRECORD;
        node->tyfields = vec;
        node->line = token->line;
        token = getToken();
        if (token->ttype != T_RBRACE)
            printErr(token->line);
    }
    else if (token->ttype == T_ARRAY)
    {
        token = getToken();
        if (token->ttype == T_OF)
        {
            Node *nextNode = parserTyid();
            node = calloc(1, sizeof(Node));            
            node->ntype = ARRAY;
            node->nextTy = nextNode;
            node->line = token->line;
        } 
    } else
    {
        printErr(token->line);
    }
    
    return node;
}

Node *parserTyid() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_ID)
    {
        node->ntype = VAR;
        node->varName = token->str;
        node->line = token->line;
    } else if (token->ttype == T_KINT)
    {
        node->ntype = KINT;
        node->line = token->line;
    } else if (token->ttype == T_KSTR)
    {
        node->ntype = KSTR;
        node->line = token->line;
    } else 
        printErr(token->line);

    return node;    
}

Node *parserTydec() {     
    Node *left = parserId();   
    Token *token = getToken();
    if (token->ttype != T_EQ) {
        printErr(token->line);
        return NULL;
    }        
    
    Node *right = paserTy();

    Node *node = calloc(1, sizeof(Node));
    node->ntype = DEC;
    node->left = left;
    node->right = right;
    node->line = token->line;
    
    return node;
}

Node *parserAssignOp() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_ASSIGN) {
        node->ntype = ASSIGN;
        node->line = token->line;
    } else
    {
        printErr(token->line);
    }
    return node;
}

Node *parserColon() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_COLON)
    {
        node->ntype = COLON;
        node->line = token->line;
    }
    else
    {
        printErr(token->line);
    }
    return node;    
}

Node *parserVardec() {
    Token *token = NULL;
    Node *node = NULL;

    Node *left = parserId();
    token = getToken();
    if (token->ttype == T_ASSIGN)
    {
        ugetToken(token);
        node = parserAssignOp();
        Node *right = parserArt();
        node->left = left;
        node->right = right;
    }
    else if (token->ttype == T_COLON)
    {
        ugetToken(token);
        Node *colon = parserColon();
        Node *tid = parserTyid();
        node = parserAssignOp();
        Node *right = parserArt();

        node->right = right;
        node->left = colon;
        colon->left = left;
        colon->right = tid;
    } else
    {
        printErr(token->line);
    }
        
    return node;
}

Node *parserDot() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_DOT)
    {
        node->ntype = DOT;
        node->line = token->line;
    }
    else
    {
        printErr(token->line);
    }
    return node;
}

Node *parserLbrack() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_LBRACK)
    {
        node->ntype = ARROP;
        node->line = token->line;
    }
    else
    {
        printErr(token->line);
    }
    return node;
}

Node *parserinitArray(Node *tyNode, Node *arrlen) {
    Node *val = parserArt();

    Node *node = calloc(1, sizeof(Node));
    node->arrType = tyNode->varName;
    node->line = tyNode->line;
    node->ntype = ARRAY;

    node->arrlen = arrlen;
    node->initArr = val;
    return node;
}

Node *parserLvalue() {
    Node *node = parserId();
    Token *token = getToken();
    while (token->ttype == T_DOT || token->ttype == T_LBRACK)
    {
        Node *left = node;
        if (token->ttype == T_DOT)
        {
            ugetToken(token);
            node = parserDot();
            Node *right = parserId();
            node->left = left;
            node->right = right;
        }
        else if (token->ttype == T_LBRACK)
        {
            ugetToken(token);
            node = parserLbrack();
            Node *right = parserArt();
            token = getToken();
            if (token->ttype != T_RBRACK) {
                printErr(token->line);
                return NULL;
            }
            token = getToken();
            if (token->ttype == T_OF)
            {
                return parserinitArray(left, right);
            }
            ugetToken(token);            
            node->left = left;
            node->right = right;
        }
        token = getToken();     
    }
    ugetToken(token);
    return node;
}

Node *parserInt() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_INT) {
        node->ntype = NINT;
        node->ival = token->num;
        node->line = token->line;
    } else
    {
        printErr(token->line);
    }
    return node;
    
}

Node *parserOr() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_OR)
    {
        node->ntype = OR;
        node->line = token->line;
    }
    else
    {
        printErr(token->line);
    }
    return node;
}

Node *parserPM() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_PLUS)
    {
        node->ntype = PLUS;
    }
    else if (token->ttype == T_MINUS)
    {
        node->ntype = MINUS;
    } else
    {
        printErr(token->line);
    }
    node->line = token->line;
    return node;    
}

Node *parserTD() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_TIMES)
    {
        node->ntype = TIMES;
    }
    else if (token->ttype == T_DIVIDE)
    {
        node->ntype = DIVIDE;
    }
    else
    {
        printErr(token->line);
    }
    node->line = token->line;
    return node;
    
}

Node *parserInitRecode(Node *vnode) {
    Node *node = calloc(1, sizeof(Node));

    node->recodeType = vnode->varName;
    node->ntype = NRECORD;
    node->line = vnode->line;
    Vector *vec = make_vector();
    Token *token = NULL;
    while (token != TNUL)
    {
        Node *id = parserId();
        token = getToken();
        if (token->ttype == T_EQ)
        {
            Node *exp = parserArt();
            Node *assign = calloc(1, sizeof(Node));
            assign->ntype = ASSIGN;
            assign->line = token->line;
            
            assign->left = id;
            assign->right = exp;
            vec_push(vec, assign);
        } else
        {
            printErr(token->line);
        }
        token = getToken();
        if (token->ttype != T_COMMA)
        {
            ugetToken(token);
            break;
        }
    }
    node->tyfields = vec;

    return node;
}

Node *parserBreak() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_BREAK)
    {
        node->ntype = BREAK;
        node->line = token->line;
        node->label = lbreak;
    } else 
        printErr(token->line);
    return node;
}

Node *parserCall(Node *callName) {    
    Node *node = calloc(1, sizeof(Node));
    node->ntype = CALL;
    node->line = callName->line;
    node->callName = callName;
    Vector *vec = NULL;
    
    Token *token = getToken();
    if (token->ttype != T_RPAREN)
    {
        ugetToken(token);
        vec = make_vector();
        while (1)
        {
            Node *arg = parserExp();
            vec_push(vec, arg);
            token = getToken();
            if (token->ttype == T_RPAREN)
                break;
            else if (token->ttype == T_COMMA)
            {} else {
                printErr(token->line); 
                break;
            }   
        } 
    }  
    
    node->args = vec;
    ugetToken(token);
    return node;    
}

Node *parserFactor() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_INT)
    {
        node->ntype = NINT;
        node->ival = token->num;
        node->line = token->line;
    }
    else if (token->ttype == T_MINUS)
    {
        token = getToken();
        if (token->ttype == T_INT)
        {
            node->ntype = NINT;
            node->ival = -token->num;
            node->line = token->line;
        }
        else if (token->ttype == T_ID)
        {
            ugetToken(token);
            node->ntype = NEGATIVE;
            node->unary = parserLvalue();
        } else
        {
            printErr(token->line);
        }                
    }
    else if (token->ttype == T_ID)
    {
        ugetToken(token);
        node = parserLvalue();        
        token = getToken();
        if (node->ntype == VAR && token->ttype == T_LBRACE)
        {
            node = parserInitRecode(node);
            token = getToken();
            if (token->ttype != T_RBRACE)            
                printErr(token->line);
        } else if (node->ntype == VAR && token->ttype == T_LPAREN)
        {
            node = parserCall(node);
            token = getToken();
            if (token->ttype != T_RPAREN)
                printErr(token->line);            
        }
         else
            ugetToken(token);                
    } else if (token->ttype == T_STRING)
    {
        node->ntype = NSTRING;
        node->strVal = token->str;
        node->strLen = strlen(token->str);
        node->line = token->line;
        node->strLabel = strLabel;
        strLabel++;
    } else if (token->ttype == T_NIL)
    {
        node->ntype = NIL;
        node->line = token->line;
    } else if (token->ttype == T_LPAREN)
    {
        node = parserExp();
        token = getToken();
        if (token->ttype != T_RPAREN)
        {
            ugetToken(token);
            Node *n = node;
            node = calloc(1, sizeof(Node));
            node->ntype = SEQ;
            node->line = token->line;
            node->sequence = make_vector();
            vec_push(node->sequence, n);
            while (token->ttype != T_RPAREN && token != TNUL)
            {
                ugetToken(token);
                n = parserExp();
                vec_push(node->sequence, n);
                token = getToken();
            }  
        }
        if (token->ttype != T_RPAREN)
            printErr(token->line);
    } else if (token->ttype == T_BREAK)
    {
        ugetToken(token);
        if (lbreak != NULL)
        {
            node = parserBreak();
        } else
            printErr(token->line);   
    } else 
        printErr(token->line);

    return node;
}

Node *parserTimDiv() {
    Node *node = parserFactor();
    Token *token = getToken();

    while (token->ttype == T_TIMES || token->ttype == T_DIVIDE)
    {
        ugetToken(token);
        Node *left = node;
        node = parserTD();
        node->left = left;
        node->right = parserFactor();
        token = getToken();
    }
    ugetToken(token);
    return node;
}

Node *parserPlusMinus() {
    Node *node = parserTimDiv();
    Token *token = getToken();

    while (token->ttype == T_PLUS || token->ttype == T_MINUS)
    {
        ugetToken(token);
        Node *left = node;
        node = parserPM();
        node->left = left;
        node->right = parserTimDiv();
        token = getToken();
    }
    ugetToken(token);
    return node;
}

Node *parserComOp() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));

    if (token->ttype == T_EQ)    
        node->ntype = EQ;    
    else if (token->ttype == T_NEQ)    
        node->ntype = NEQ;    
    else if (token->ttype == T_LT)
        node->ntype = LT;
    else if (token->ttype == T_LE)
        node->ntype = LE;
    else if (token->ttype == T_GT)
        node->ntype = GT;
    else if (token->ttype == T_GE)
        node->ntype = GE;
    else
        printErr(token->line);
    node->line = token->line;
    
    return node;
}

Node *parserCompare() {
    Node *node = parserPlusMinus();
    Token *token = getToken();

    if (token->ttype == T_EQ || token->ttype == T_NEQ || token->ttype == T_LT ||
        token->ttype == T_LE || token->ttype == T_GT || token->ttype == T_GE)
    {
        ugetToken(token);
        Node *left = node;
        node = parserComOp();
        node->left = left;
        node->right = parserPlusMinus();
    } else
        ugetToken(token);
    
    
    return node;
}

Node *parserAndOp() {
    Token *token = getToken();
    Node *node = calloc(1, sizeof(Node));
    if (token->ttype == T_AND)
    {
        node->ntype = AND;
        node->line = token->line;
    }
    else
    {
        printErr(token->line);   
    }
    return node;
}

Node *parserAnd() {
    Node *node = parserCompare();
    Token *token = getToken();

    while (token->ttype == T_AND)
    {
        ugetToken(token);
        Node *left = node;
        node = parserAndOp();
        node->left = left;
        node->right = parserCompare();

        token = getToken();
    }
    ugetToken(token);
    return node;
}

Node *parserArt() {
    Node *node = parserAnd();

    Token *token = getToken();

    while (token->ttype == T_OR)
    {  
        ugetToken(token);
        Node *left = node;
        node = parserOr();
        node->left = left;
        node->right = parserAnd();

        token = getToken();
    }
    ugetToken(token);
    return node;    
}

Node *parserAssigen() {
    Node *node = parserArt();

    Token *token = getToken();
    if (token->ttype == T_ASSIGN)
    {   
        ugetToken(token);
        Node *left = node;
        node = parserAssignOp();
        node->left = left;
        node->right = parserArt();
    } else
        ugetToken(token);
    
    return node;    
}

Node *parserBranch() {
    Token *token = getToken();
    Node *node = NULL;
    if (token->ttype == T_LET)
    {
        ugetToken(token);
        node = parserTiger();
    } else
    {
        ugetToken(token);
        node = parserExp();
    }
    return node;
}

Node *parserExp() {
    Token *token = getToken();
    Node *node = NULL;

    if (token->ttype == T_IF)
    {
        node = calloc(1, sizeof(Node));
        node->ntype = COND;
        node->line = token->line;
        Node *cond = parserArt();
        token = getToken();
        Node *then = NULL;
        Node *els = NULL;
        if (token->ttype == T_THEN)
        {
            then = parserBranch();
            token = getToken();
            if (token->ttype == T_ELSE)
            {
                els = parserBranch();
            } else
                ugetToken(token);                        
        }else
            printErr(token->line);        
        
        node->cond = cond;
        node->then = then;
        node->els = els;       
    }
    else if (token->ttype == T_WHILE)
    {
        node = calloc(1, sizeof(Node));
        node->ntype = WHILE;
        node->line = token->line;       

        Node *cond = parserArt();
        Node *then = NULL;
        token = getToken();
        char *obreak = lbreak;
        lbreak = make_label();
        if (token->ttype == T_DO)
        {
            then = parserBranch();
        } else
            printErr(token->line);
        node->lbreak = lbreak;
        lbreak = obreak;

        node->exp1 = cond;
        node->exp2 = then;
    }
    else if (token->ttype == T_FOR)
    {
        node = calloc(1, sizeof(Node));
        node->ntype = FOR;
        node->line = token->line;

        Node *exp1 = parserAssigen();

        if (exp1->ntype != ASSIGN)        
            printErr(exp1->line);
        
        token = getToken();
        if (token->ttype != T_TO)
            printErr(token->line);
        Node *exp2 = parserArt();

        token = getToken();
        if (token->ttype != T_DO)
            printErr(token->line);

        char *obreak = lbreak;
        lbreak = make_label();
        Node *exp3 = parserBranch();
        node->lbreak = lbreak;
        lbreak = obreak;
        
        node->exp1 = exp1;
        node->exp2 = exp2;
        node->exp3 = exp3;
    } else if (token->ttype == T_LET)
    {
        ugetToken(token);
        node = parserTiger();
    } else
    {      
        ugetToken(token);      
        node = parserAssigen();
        token = getToken();            
        
        if (token->ttype != T_SEMICOLON) {
            ugetToken(token);               
        } 
    }
    return node;
}

void parserExps(Vector *vec) {
    Token *token = getToken();
    Node *node = NULL;
    while (token != TNUL && token->ttype != T_END)
    {
        ugetToken(token);
        node = parserExp();
        vec_push(vec, node);  
        token = getToken();      
    }
    ugetToken(token);
}

Node *parserFundec() {
    Node *node = calloc(1, sizeof(Node));
    Node *id = parserId();

    Token *token = getToken();
    if (token->ttype != T_LPAREN)
        printErr(token->line);
    
    Vector *vec = parserTyfields();

    token = getToken();
    if (token->ttype != T_RPAREN)
        printErr(token->line);

    token = getToken();
    Node *exp = NULL;
    Node *reType = NULL;
    if (token->ttype == T_COLON)
    {
        reType = parserTyid(); 
        token = getToken();       
    } 
    
    if (token->ttype == T_EQ)
    {
        exp = parserBranch();
    } else
        printErr(token->line);

    node->ntype = FUNCTION;
    node->line = id->line;
    node->funName = id;
    node->fargs = vec;
    node->body = exp;
    node->returnType = reType;

    return node;
}

void paserDecs(Vector *vec) {
    Token *token = getToken();
    Node *node = NULL;

    if (token->ttype == T_TYPE || token->ttype == T_VAR || token->ttype == T_FUNCTION)
    {
        while (token != TNUL && token->ttype != T_IN)
        {
            if (token->ttype == T_TYPE)
            {
                node = parserTydec();                
            }
            else if (token->ttype == T_VAR)
            {
                node = parserVardec();
            }
            else if(token->ttype == T_FUNCTION)
            {
                char *nbreak = lbreak;
                lbreak = NULL;
                node = parserFundec();
                lbreak = nbreak;
            } else
            {
                printErr(token->line);
            }
            if (node != NULL)
                vec_push(vec, node);   
            token = getToken();         
        }        
    } else
        if(token->ttype != T_IN)
            printErr(token->line);
    ugetToken(token);
}

Node *parserTiger()
{
    Node *node = calloc(1, sizeof(Node));
    node->ntype = LET;
    Token *token = getToken();
    Vector *vec = make_vector();
    if (token->ttype == T_LET)
    {
        paserDecs(vec);
        node->decs = vec;
        token = getToken();
        if (token->ttype == T_IN)
        {
            vec = make_vector();
            parserExps(vec);
            node->expseq = vec;
            token = getToken();
            if (token->ttype != T_END)
                printErr(token->line);           
        } else {
            printErr(token->line);           
        }
    } else {
        printErr(token->line);          
    }
    return node;
}