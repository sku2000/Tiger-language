#ifndef TIGER_H
#define TIGER_H

enum {
    T_ID, T_STRING, T_INT, 
    T_ERROR,
    //, : ; ( ) [ ] { } . + - * / = <> < <= > >= & | :=
    T_COMMA, T_COLON, T_SEMICOLON, T_LPAREN, T_RPAREN, T_LBRACK, T_RBRACK, T_LBRACE, 
    T_RBRACE, T_DOT, T_PLUS, T_MINUS, T_TIMES, T_DIVIDE, T_EQ, T_NEQ, T_LT, T_LE, T_GT,
    T_GE, T_AND, T_OR, T_ASSIGN,
    T_WHILE, T_FOR, T_TO, T_BREAK, T_LET, T_IN, T_END, T_FUNCTION, T_VAR, T_TYPE, T_ARRAY,
    T_IF, T_THEN, T_ELSE, T_DO, T_OF, T_NIL, T_NUL, T_KINT, T_KSTR
};

typedef struct Token
{
    int ttype;
    union 
    {
        int num;
        char *str;
    };
    int line;
    char *filename;    
} Token;

typedef struct Map
{
    struct Map *parent;
    char **key;
    void **val;
    int size;
    int nelem;
    int nused;

    unsigned int (*hash)(char *key);
    int (*eq)(const char *, const char *);

    int frame;
} Map;

typedef struct Vector {
    void **val;
    int nelem;
    int size;
} Vector;

typedef struct Set
{
    Map *map;    
} Set;

enum {
    LET, DEC, EXP, VAR, ARRAY, ARROP, NRECORD, FUNCTION, CALL, NIL, SEQ, COND, WHILE, FOR, BREAK,
    //, : ; ( ) [ ] { } . + - * / = <> < <= > >= & | :=
    COMMA, COLON, SEMICOLON, LPAREN, RPAREN, LBRACK, RBRACK, LBRACE, 
    RBRACE, DOT, PLUS, MINUS, TIMES, DIVIDE, EQ, NEQ, LT, LE, GT,
    GE, AND, OR, ASSIGN, NEGATIVE,
    KINT, KSTR, NINT, NSTRING, 
};

enum {
    INT, STRING, ARRAYTYPE, RECORD,FUNTYPE, ALIAS, VOID, ERR, READ_ONLY
};

typedef struct Type Type;
struct Type
{
   int type;
   char *typeName;
   int mark;
   union 
   {
       //array type
       struct {
           Type *eleType;
       };
       //record type;
       struct {
            Vector *fields;
            Map *recordType;
            Map *offsetMap;
            int len;
       };
        //function type;
        struct {
            Vector *args;
            Map *argsType;
            Type *returnType;
            int level;
        };
   };
};

extern Type *INTTYPE;
extern Type *STRTYPE;
extern Type *VOIDTYPE;
extern Type *NILTYPE;
extern Type *READONLY;

typedef struct Node Node;
struct Node {
    int line;
    int ntype;
    Type *type;
    
    union 
    {   
        //INT
        int ival;
        //STRING
        struct {
            char *strVal;
            int strLen;
            int strLabel;
        };
        //VAR
        struct {
            char *varName;  
            Node *varNode; 
            int level; 
            int offset;        
        };
        //RECORD
        struct {
            char *recodeType;
            //key: Node, val: Node
            Vector *tyfields;
            Map *recode;
        };
        //ARRAY
        struct {
            char *arrType;
            Node *nextTy;
            Vector *arr;           
            Node *arrlen;
            Node *initArr;                       
        };
        //FUNCTION
        struct {
            Node *funName;
            Vector *fargs;
            
            Node *body;
            //return type
            Node *returnType;
            //Map
            Map *context;
            int loopVar;
        };
        //CALL
        struct {
            Node *callName;
            Vector *args;
        };
        //SEQ
        struct {
            Vector *sequence;
        };
        //COND
        struct {
            Node *cond;
            Node *then;
            Node *els;
        };      
        //FOR or WHILE
        struct 
        {
            Node *exp1;
            Node *exp2;
            Node *exp3;
            char *lbreak;
        };
        //op
        struct 
        {
            Node *left;
            Node *right;
        };    
        //Unary operator
        struct 
        {
            Node *unary;
        };
        //decs and expseq
        struct 
        {
            int localVarLen;
            Vector *decs;
            Vector *expseq;
        };      
        //label
        char *label;
    };    
};

extern Token *TNUL;

typedef struct Frame Frame;
struct Frame {
    Vector *stVec;
    Node *node;
};

enum {
    IR_GRAPH = 1,
    IR_BLOCK,
    CONST, OP_VAR, ADD, SUB, MUL, DIV, LSHIFT, RSHIFT, OP_AND, OP_OR, OP_NOT,    
    CMP_LT, CMP_LE, CMP_EQ, CMP_GE, CMP_GT, CMP_NE, CBR,
    COMP, CBR_LT, CBR_LE, CBR_EQ, CBR_GE, CBR_GT, CBR_NE, 
    JUMP, 
    // CALL arg0, arg1 ...
    //RET returnVal
    //RECEIVE par0, par1 ...
    OP_CALL, RET, RECEIVE,
    PHI,
    LOAD, STORE, MOV, ALLOC
};

enum {
    LABEL = 1, IMM, BYTE, DWORD, QWORD
};
typedef struct ir_node ir_node;
struct ir_node {
    int op;
    int type;

    union 
    {
        //int const
        int ival;

        //string const
        char *addr;

        //graph
        struct {
            char *funName;
            Vector *fargs;
            ir_node *start;
            ir_node *end;
            ir_node *graph;
        };

        //basicblock
        struct {
            Vector *insts;
        };

        //label: if, loop, goto
        struct {
            char *label;
        };

        //binop
        struct {
            ir_node *left;
            ir_node *right;
        };

        //unop, jump, return
        struct {
            ir_node *opd;
        };

        //if
        struct {
            ir_node *cond;
            ir_node *then;
            ir_node *els;
        };

        //call
        struct {
            char *callName;
            ir_node *function;
            Vector *args;
        };

        //phi
        struct {
            ir_node *phi;
            Vector *phis;
        };
    };
    ir_node *prev, *next;
};

typedef struct IR IR;
struct IR {
    Vector *strVec;
    ir_node *ir;
};

//errormsg.c
void errormsg(const char *msg, ... );

//lex.c
void initToken(char *filename);
Token *getToken();
void freeToken(Token *token);
Token *make_op_token(Token *token, int type);
void make_id_type(Token *token);
void ugetToken(Token *token);
 
//map.c
Map *make_map();
Map *make_parent_map(Map *parent);
void *map_get(Map *map, void *key);
void map_put(Map *map, void *key, void *val);
void map_remove(Map *map, char *key);
int map_len(Map *map);
int map_updata(Map *map, void *key, void *val);
void *map_get1(Map *map, char *key);
int map_contaion1(Map *map, char *key);
int map_contation(Map *map, char *key);

//Vector
Vector *make_vector();
Vector *make_vector1(void *val);
Vector *make_vectorLen(int i);
Vector *vec_copy(Vector *src);
void vec_push(Vector *vec, void *val);
void vec_append(Vector *a, Vector *src);
void *vec_pop(Vector *vec);
void *vec_get(Vector *vec, int index);
void vec_set(Vector *vec, int index, void *val);
void *vec_head(Vector *vec);
void *vec_tail(Vector *vec);
Vector *vec_reverse(Vector *vec);
void *vec_body(Vector *vec);
int vec_len(Vector *vec);
void vec_insert(Vector *vec, int index, void *val);

//set.c
Set *make_set();
void set_add(Set *set, char *key);
int set_contaion(Set *set, char *key);
void set_remove(Set *set, char *key);
int set_len(Set *set);

//parser.c
Node *parser();
char *make_label();

//semantic.c
int checkTiger(Node *node);

//semanticTest.c
void testTiger(Node *node);

//string.c
char *copyString(char *str);

//frame.c
Frame *frame(Node *node);

//asm.c
void genAsm(Frame *fm);

#endif