#include <stdio.h>
// #include <stdlib.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include <string.h>



int main(int argc, char **argv) {
    int i;
    char *s = "cd test | cd .. | piping";
    for(i=0; s[i]; s[i]=='|' ? i++ : *s++);
    printf("%i", i);
}