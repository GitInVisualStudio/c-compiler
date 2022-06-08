#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header/gen.h"

int main(int argc, char** args) {
    if (argc != 3) return -1;
    compile(args[1], args[2]);
    return 0;
}