#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define BUFF_RL 64
#define TOKEN_DELIMITER "|"


int
spawn_proc2 (int in, int out, char** argv)
{
  pid_t pid;
  if ((pid = fork ()) == 0){
      if (in != 0){
          dup2 (in, 0);
          close (in);
      }
      if (out != 1){
          dup2 (out, 1);
          close (out);
        }
      return execvp (*argv, (char * const *)argv);
    }
  return pid;
}



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

void printPP(int argc, char ** args) 
{
  int i;
  printf("Printing single command\n");
  for(i=0; i<argc; i++) {
    printf("%s ", *args);
    args++;
  }
  printf("\nend\n\n\n");

}

int
fork_pipes2 (char* line)
{
  int i;
  
  pid_t pid;
  int in, fd [2];
  char** argv;
  char* tkn;
  int n;

  // copy input line
  char* inputptr = strdup(line);
  char* strCopy = strdup(line);

  for(n=0; strCopy[n]; strCopy[n]=='|' ? n++ : *strCopy++);

  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  tkn = strtok(inputptr, TOKEN_DELIMITER);
  
  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  if(n>0) {
    for (i = 0; i < n; ++i)
    {
      int ac;
      argv = parsedargs(tkn, &ac);

      printPP(n+1, argv);

      pipe (fd);

      // f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      spawn_proc2(in, fd [1], argv);

      // No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      // Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];

      tkn = strtok(NULL, TOKEN_DELIMITER);
      // printf("token %i: %s\n", i+1, tkn);
    }

    int ac;
    argv = parsedargs(tkn, &ac);
    printPP(n+1, argv);
    /* Last stage of the pipeline - set stdin be the read end of the previous pipe
      and output to the original file descriptor 1. */  
    if (in != 0)
      dup2 (in, 0);

    /* Execute the last stage with the current process. */
    return execvp (*argv, (char * const *)argv);
  } else {
    int ac;

    argv = parsedargs(tkn, &ac);

    printPP(n+1, argv);

    return execvp (*argv, (char * const *)argv);
  }
  
}


char *sh_read_line(void) {
    int posidx = 0;
    int buf_s = BUFF_RL;
    // initialize block to hold the amount of memory it takes to hold
    // a single char X the number of chars we are initially accepting
    char *buf = malloc(sizeof(char) * buf_s);
    int c;

    // catch any situation where intialization does not occur
    if (!buf) {
        // Format output stream
        fprintf(stderr, "shell error when allocating memory\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // We will read character by character
        c = getchar();

        // Check for EOF and replace EOF with null character and return
        if(c == EOF || c == '\n') {
            buf[posidx] = '\0';
            return buf;
        } else {
            buf[posidx] = c;
        }
        posidx++;

        // Once iterating to next possition, we much check if we are out of the
        // bounds of the char array.  If we are, then we must reallocate
        if(posidx >= buf_s) {
            buf_s += BUFF_RL;
            buf = realloc(buf, buf_s);
            // check once more for allocation error
            if(!buf) {
                fprintf(stderr, "shell error when reallocating memory\n");
                exit(EXIT_FAILURE);
            }
        }
    }

}

void intcmd_loop() {
    char status;
    char **args;
    char *line;
    // int argc;

    do {
        // Print path to current working directory
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s %s", cwd, "--------------------------------------------------");
        printf("\nShell: ");

        // Read command
        line = sh_read_line();

        // fork and pipe (if more than 1 command)
        fork_pipes2(line);

        printf("is wackkkk\n\n");


    } while (status);
}

int
main ()
{
  intcmd_loop();
  printf("ya boiiiiiii\n\n");
  
  return 1;
}

