#include <stdio.h>

    static int setargs(char *args, char **argv)
    {
       int count = 0;

       while (isspace(*args)) ++args;
       while (*args) {
         if (argv) argv[count] = args;
         while (*args && !isspace(*args)) ++args;
         if (argv && *args) *args++ = '\0';
         while (isspace(*args)) ++args;
         count++;
       }
       return count;
    }

    char **parsedargs(char *args, int *argc)
    {
       char **argv = NULL;
       int    argn = 0;

       if (args && *args
        && (args = strdup(args))
        && (argn = setargs(args,NULL))
        && (argv = malloc((argn+1) * sizeof(char *)))) {
          *argv++ = args;
          argn = setargs(args,argv);
       }

       if (args && !argv) free(args);

       *argc = argn;
       return argv;
    }

    void freeparsedargs(char **argv)
    {
      if (argv) {
        free(argv[-1]);
        free(argv-1);
      } 
    }

    int main(int argc, char *argv[])
    {
      int i;
      char **av;
      int ac;
      char *as = NULL;

      if (argc > 1) as = argv[1];

      av = parsedargs(as,&ac);
      printf("== %d\n",ac);
      for (i = 0; i < ac; i++)
        printf("[%s]\n",av[i]);

      freeparsedargs(av);
      exit(0);
    }