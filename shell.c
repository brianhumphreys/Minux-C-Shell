#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define BUFF_RL 64
#define TOKEN_DELIMITER "|"
#define INPUT_MAX 64

int single_spawn(char **argv, int bg)
{
    if (!strcmp(argv[0], "exit"))
    {
        exit(0);
    }


    pid_t pid, wpid;
    int status;

    // fork cuase a duplicate process to be made
    // after this system call, the processes will run concurrently,
    // using same program counter (for asm instr), same CPU and same
    // and same files.
    // They will run together until the parent uses waitpid to wait for the child process to 
    // finish.  
    
    pid = fork();
    if (pid == 0) {
        // Then the child process is running
        if (execvp(*argv, (char * const *)argv) == -1) {
            perror("shell\n\n");
        } else {
          printf("executed successfully\n\n");
        }
        exit(EXIT_FAILURE);
    } else if (pid > 0 && !bg) {
        // Parent process will land here and wait
        do {
            
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // checks for signal to kill or signal that child process is completed
    } else if(pid<0) {
        // There was an error forking
        perror("Error while forking");
    }
    return 1;
}



int
pipe_spawn (int in, int out, char** argv, bool last, int bg)
{
    if (!strcmp(argv[0], "exit"))
    {
        exit(0);
    }
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0){
        if (in != 0){
            if(last) {
                dup2 (in, 0);
            }
            else {
                dup2 (in, 0);
                close (in);
            } 
      }
      if (out != 1){
          dup2 (out, 1);
          close (out);
      }
      if(execvp (*argv, (char * const *)argv) == -1) {
        perror("shell");
        exit(EXIT_FAILURE);
      }
  } else if(pid > 0 && !bg) {
    // parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // checks for signal to kill or signal that child process is completed
    printf("I have waited for child\n");
  } else if(pid<0) {
      // There was an error forking
      perror("Error while forking");
  }
  return 1;
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

void printPP(int argc, char ** argv) 
{
  int i;
  for (i = 0; i < argc; i++)
        printf("[%s]\n",argv[i]);
  printf("______________\n");
}

int
fork_pipes (char* line, bool bg)
{
  int i;
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

      pipe (fd);

      // f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      pipe_spawn(in, fd [1], argv, false, bg);

      // No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      // Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];

      tkn = strtok(NULL, TOKEN_DELIMITER);
    }
    int ac;
    argv = parsedargs(tkn, &ac);
    /* Last stage of the pipeline - set stdin be the read end of the previous pipe
      and output to the original file descriptor 1. */  
    pipe_spawn(in, fd [1], argv, true, bg);
    return 1;
  } else {
    int ac;

    argv = parsedargs(tkn, &ac);

    return single_spawn(argv, bg);
  }
  
}

void interp_cmd_loop() {
    int status;
    

    do {
        int bg=0;
        char line[INPUT_MAX];
        // // Print path to current working directory
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("\n%s %s", cwd, "--------------------------------------------------");
        printf("\nShell: ");

        // Read command
        fgets(line, INPUT_MAX, stdin);
        if(line[strlen(line)-2]=='&')
            {
              bg = 1;
              line[strlen(line) - 2] = '\0';
            }
        // fork and pipe (if more than 1 command)
        status = fork_pipes(line, bg);


    } while (status);
}

int
main ()
{
  interp_cmd_loop();
  
  return 1;
}

