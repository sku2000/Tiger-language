#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "tiger.h"

#define FILELEN 1024 * 1024

char *fname;
int line = 1;

char buf[FILELEN];
int pointer = 0;
Token *TNUL = &(Token){
    .ttype = T_NUL,   
};

Token *t = NULL;

void initToken(char *filename)
{
    int num = 0;
    fname = filename;

    memset(buf, '\0', FILELEN);
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("file open error");
        exit(1);
    }

    num = fread(buf, sizeof(char), FILELEN, file);

    if (!feof(file) || num == FILELEN)
    {
        printf("file too large");
        exit(1);
    }
    fclose(file);
}

int isSpace(char c)
{
    return (c == '\n' || c == '\r' || c == '\t' || c == ' ');
}

void ugetToken(Token *token) {
    t = token;
}

Token *getToken()
{
    if (t != NULL) {
        Token *token = t;
        t = NULL;
        return token;
    }
    
    Token *token = (Token *)malloc(sizeof(Token));

    while (isSpace(buf[pointer]))
    {
        if (buf[pointer] == '\n')
        {
            line++;
        }
        pointer++;
    }
    if (buf[pointer] >= '0' && buf[pointer] <= '9')
    {
        token->ttype = T_INT;
        token->line = line;
        token->filename = fname;
        token->num = 0;
        while (buf[pointer] >= '0' && buf[pointer] <= '9')
        {
            token->num = token->num * 10 + buf[pointer] - '0';
            pointer++;
            if (buf[pointer] == '\0')
            {
                return token;
            }
        }
        return token;
    }
    else if (isalpha(buf[pointer]))
    {
        char bf[128];
        int i = 0;

        token->ttype = T_ID;
        token->line = line;
        token->filename = fname;
        while (isalnum(buf[pointer]) || buf[pointer] == '_')
        {
            bf[i++] = buf[pointer++];
            if (i >= 128)
            {
                errormsg("Variable names are too long %d\n", line);
                exit(1);
            }
        }
        bf[i] = '\0';
        token->str = (char *)malloc(sizeof(i + 1));
        memset(token->str, '\0', i + 1);
        strcpy(token->str, bf);
        make_id_type(token);
        return token;
    }
    else if (buf[pointer] == '/' && buf[pointer + 1] == '*')
    {
        pointer = pointer + 2;
        int flag = 1;
        while (1)
        {
            if (buf[pointer] == '\0')
            {
                errormsg("The lack of annotation %d\n", line);
                break;
            }
            if (buf[pointer] == '\n')
                line++;
            if (buf[pointer] == '*' && buf[pointer + 1] == '/')
            {
                flag--;
                pointer = pointer + 2;
            }
            if (buf[pointer] == '/' && buf[pointer + 1] == '*')
            {
                flag++;
                pointer += 2;
            }

            if (flag == 0)
                break;
            pointer++;
        }
        return getToken();
    }
    else if (buf[pointer] == '\"')
    {
        pointer++;
        char bf[1024];
        int i = 0;
        while (1)
        {
            if (buf[pointer] == '\\')
            {
                pointer++;
                if (buf[pointer] == 'n')
                {
                    bf[i++] = '\n';
                    pointer++;
                }
                else if (buf[pointer] == 't')
                {
                    bf[i++] = '\t';
                    pointer++;
                }
                else if (isdigit(buf[pointer]))
                {
                    int ascii = buf[pointer] - '0';
                    pointer++;
                    if (isdigit(buf[pointer]))
                    {
                        ascii = ascii * 10 + buf[pointer] - '0';
                        pointer++;
                        if (isdigit(buf[pointer]))
                        {
                            ascii = ascii * 10 + buf[pointer] - '0';
                            pointer++;
                        }
                    }
                    bf[i++] = ascii;
                }
                else if (buf[pointer] == '\\')
                {
                    bf[i++] = '\\';
                    pointer++;
                }
                else if (buf[pointer] == '\"')
                {
                    bf[i++] = '\"';
                    pointer++;
                }
                else if (buf[pointer] == 'f')
                {
                    pointer++;
                    while (!(buf[pointer] == 'f' && buf[pointer + 1] == '\\'))
                    {
                        if (buf[pointer] == '\0')
                        {
                            errormsg("/f does not end %d\n", line);
                            TNUL->line = line;
                            return TNUL;
                        }

                        if (buf[pointer] == '\n')
                        {
                            line++;
                        }
                        pointer++;
                    }
                    pointer += 2;
                }
                else
                {
                    errormsg("escape character \\%c string error %d\n", buf[pointer], line);
                    pointer++;
                    return make_op_token(token, T_ERROR);
                }
            }
            if (buf[pointer] == '\"')
            {
                break;
            }
            if (buf[pointer] == '\0')
            {
                errormsg("string does not end %d\n", line);
                TNUL->line = line;
                return TNUL;
            }
            if (i > 1024)
            {
                errormsg("Strings cannot be longer than 1024 %d\n", line);
                exit(1);
            }
            bf[i++] = buf[pointer++];
        }
        pointer++;

        bf[i] = '\0';
        token->ttype = T_STRING;
        token->line = line;
        token->filename = fname;

        token->str = (char *)malloc(i + 1);
        memset(token->str, '\0', i + 1);
        strcpy(token->str, bf);

        return token;
    }
    else
    {
        if (buf[pointer] == ',')
        {
            pointer++;
            return make_op_token(token, T_COMMA);
        }
        else if (buf[pointer] == ':')
        {
            pointer++;
            if (buf[pointer] == '=')
            {
                pointer++;
                return make_op_token(token, T_ASSIGN);
            }
            else
            {
                return make_op_token(token, T_COLON);
            }
        }
        else if (buf[pointer] == ';')
        {
            pointer++;
            return make_op_token(token, T_SEMICOLON);
        }
        else if (buf[pointer] == '(')
        {
            pointer++;
            return make_op_token(token, T_LPAREN);
        }
        else if (buf[pointer] == ')')
        {
            pointer++;
            return make_op_token(token, T_RPAREN);
        }
        else if (buf[pointer] == '[')
        {
            pointer++;
            return make_op_token(token, T_LBRACK);
        }
        else if (buf[pointer] == ']')
        {
            pointer++;
            return make_op_token(token, T_RBRACK);
        }
        else if (buf[pointer] == '{')
        {
            pointer++;
            return make_op_token(token, T_LBRACE);
        }
        else if (buf[pointer] == '}')
        {
            pointer++;
            return make_op_token(token, T_RBRACE);
        }
        else if (buf[pointer] == '.')
        {
            pointer++;
            return make_op_token(token, T_DOT);
        }
        else if (buf[pointer] == '+')
        {
            pointer++;
            return make_op_token(token, T_PLUS);
        }
        else if (buf[pointer] == '-')
        {
            pointer++;
            return make_op_token(token, T_MINUS);
        }
        else if (buf[pointer] == '*')
        {
            pointer++;
            return make_op_token(token, T_TIMES);
        }
        else if (buf[pointer] == '/')
        {
            pointer++;
            return make_op_token(token, T_DIVIDE);
        }
        else if (buf[pointer] == '=')
        {
            pointer++;
            return make_op_token(token, T_EQ);
        }
        else if (buf[pointer] == '<')
        {
            pointer++;
            if (buf[pointer] == '>')
            {
                pointer++;
                return make_op_token(token, T_NEQ);
            }
            else if (buf[pointer] == '=')
            {
                pointer++;
                return make_op_token(token, T_LE);
            }
            else
            {
                return make_op_token(token, T_LT);
            }
        }
        else if (buf[pointer] == '>')
        {
            pointer++;
            if (buf[pointer] == '=')
            {
                pointer++;
                return make_op_token(token, T_GE);
            }
            else
            {
                return make_op_token(token, T_GT);
            }
        }
        else if (buf[pointer] == '&')
        {
            pointer++;
            return make_op_token(token, T_AND);
        }
        else if (buf[pointer] == '|')
        {
            pointer++;
            return make_op_token(token, T_OR);
        }
        else if (buf[pointer] == '\0')
        {
            TNUL->line = line;
            return TNUL;
        }
        else
        {
            errormsg("Invalid operator %c %d\n", buf[pointer], line);
            pointer++;
            return make_op_token(token, T_ERROR);
        }
    }
    TNUL->line = line;
    return TNUL;
}

Token *make_op_token(Token *token, int type)
{
    token->ttype = type;
    token->line = line;
    token->filename = fname;
    return token;
}

void freeToken(Token *token)
{
    if (token == NULL)
        return;
    free(token);
    token = NULL;
}

void make_id_type(Token *token)
{
    if (!strcmp(token->str, "while"))
    {
        token->ttype = T_WHILE;
    }
    else if (!strcmp(token->str, "for"))
    {
        token->ttype = T_FOR;
    }
    else if (!strcmp(token->str, "to"))
    {
        token->ttype = T_TO;
    }
    else if (!strcmp(token->str, "break"))
    {
        token->ttype = T_BREAK;
    }
    else if (!strcmp(token->str, "let"))
    {
        token->ttype = T_LET;
    }
    else if (!strcmp(token->str, "in"))
    {
        token->ttype = T_IN;
    }
    else if (!strcmp(token->str, "end"))
    {
        token->ttype = T_END;
    }
    else if (!strcmp(token->str, "function"))
    {
        token->ttype = T_FUNCTION;
    }
    else if (!strcmp(token->str, "var"))
    {
        token->ttype = T_VAR;
    }
    else if (!strcmp(token->str, "type"))
    {
        token->ttype = T_TYPE;
    }
    else if (!strcmp(token->str, "array"))
    {
        token->ttype = T_ARRAY;
    }
    else if (!strcmp(token->str, "if"))
    {
        token->ttype = T_IF;
    }
    else if (!strcmp(token->str, "then"))
    {
        token->ttype = T_THEN;
    }
    else if (!strcmp(token->str, "else"))
    {
        token->ttype = T_ELSE;
    }
    else if (!strcmp(token->str, "do"))
    {
        token->ttype = T_DO;
    }
    else if (!strcmp(token->str, "of"))
    {
        token->ttype = T_OF;
    }
    else if (!strcmp(token->str, "nil"))
    {
        token->ttype = T_NIL;
    }
    else if (!strcmp(token->str, "int"))
    {
        token->ttype = T_KINT;
    }
    else if (!strcmp(token->str, "string"))
    {
        token->ttype = T_KSTR;
    }
}