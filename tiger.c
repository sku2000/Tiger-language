#include<stdio.h>
#include "tiger.h"
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2)
    {
        printf("argc less than two");
        exit(1);
    }
    initToken(argv[1]);
    
    Node *node = parser();
    if (node != NULL)
    {
        if (checkTiger(node)) {
            Frame *fm = frame(node);
            genAsm(fm);
        }     
    }    
    
    fflush(stdin);
    fflush(stdin);
    getchar();
}

