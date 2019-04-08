// /* CELEBG03

//    This example determines the working directory.

//  */
// #define _POSIX_SOURCE
// #include <unistd.h>
// #undef _POSIX_SOURCE
// #include <stdio.h>

// main() {
//   char cwd[256];

//   if (chdir("/tmp") != 0)
//     perror("chdir() error()");
//   else {
//     if (getcwd(cwd, sizeof(cwd)) == NULL)
//       perror("getcwd() error");
//     else
//       printf("current working directory is: %s\n", cwd);
//   }
// }

#include <unistd.h>
#include <stdio.h>

int main() {
    char cwd[1024];
    // chdir("/path/to/change/directory/to");
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);
}