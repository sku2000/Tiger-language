#include <stdio.h>
#include <stdarg.h>

void errormsg(const char *msg, ... ) {
    va_list valist;

    va_start(valist, msg);
    vfprintf(stdout, msg, valist);
    fprintf(stdout, "\n");
    va_end(valist);
}