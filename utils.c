#include "tiger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *copyString(char *str) {
    int len = strlen(str);
    char *s = calloc(1, (len + 1) * sizeof(char));
    strcpy(s, str);
    return s;
}