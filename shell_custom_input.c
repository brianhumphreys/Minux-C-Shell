#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define BUFF_RL 64
#define TOKEN_DELIMITER "|"
#define INPUT_MAX 64

int sh_spawn(char **argv)
{
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
        if (execvp(argv[0], argv) == -1) {
            perror("shell\n\n");
        } else {
          printf("executed successfully\n\n");
        }
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process will land here and wait
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // checks for signal to kill or signal that child process is completed
    } else {
        // There was an error forking
        perror("shell");
    }
    return 1;
}

int
spawn_proc2 (int in, int out, char** argv)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0){
    // Then the child process is running
      if (in != 0){
          dup2 (in, 0);
          close (in);
      }
      if (out != 1){
          dup2 (out, 1);
          close (out);
          // printf("yunkkkkkkkkkk\n");
      }
      // printf("One step closer\n");
      if(execvp (*argv, (char * const *)argv) == -1) {
        perror("shell");
        exit(EXIT_FAILURE);
      }
      // exit(EXIT_FAILURE);
  } else if(pid > 0) {
    // parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // checks for signal to kill or signal that child process is completed
  } else {
      // There was an error forking
      perror("shell");
  }
  // printf("yunkkkkkkkkkk\n");
  return 1;
}

// pay attention to the above when figuring out looping
int
spawn_proc1 (int in, int out, char** argv)
{
  pid_t pid;
  if ((pid = fork ()) == 0){
    // Then the child process is running
      if (in != 0){
          dup2 (in, 0);
          close (in);
      }
      if (out != 1){
          dup2 (out, 1);
          close (out);
        }
      printf("One step closer\n");
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
    // if (*args != '\0') *args++ = '\0';
    // if (argv[count] != '\0') {
    //   argv[count] = args;
    //   count++;
    // }
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
fork_pipes2 (char* line)
{
  int i;
  
  // pid_t pid;
  int in, fd [2];
  char** argv;
  char* tkn;
  int n;
  // int status

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
      // printf("== %d\n",ac);
      

      // printPP(ac, argv);

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
    // printPP(n+1, argv);
    /* Last stage of the pipeline - set stdin be the read end of the previous pipe
      and output to the original file descriptor 1. */  
    if (in != 0)
      dup2 (in, 0);

    /* Execute the last stage with the current process. */
    // execvp (*argv, (char * const *)argv);
    // printf("yunkkkkkkkkkk\n");


    // if(execvp (*argv, (char * const *)argv) == -1) {
    //     printf("yunkkkkkkkkkk1\n");
    //     perror("shell");
    // }
    
    // printf("yunkkkkkkkkkk2\n");
    // printf("OUT: %i\n", out);
    // printf("yunkkkkkkkkkk\n");
    return sh_spawn(argv);
  } else {
    int ac;

    argv = parsedargs(tkn, &ac);
    // printf("== %d\n",ac);
    //   for (i = 0; i < ac; i++)
    //     printf("[%s]\n",argv[i]);
    // char **parsedargs(char *args, int *argc)

    // printPP(n+1, argv);
    // printf("here i am\n");


    // execvp (*argv, (char * const *)argv);
    // if(execvp (*argv, (char * const *)argv) == -1) {
    //     perror("shell");
    // }
    // printf("here i am2\n\n");
    // printf("OUT: %i\n", out);

    return sh_spawn(argv);;
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
        printf("TESTING");
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
    int status;
    // char **args;
    // char *line;
    // int argc;

    do {
        char line[INPUT_MAX];
        // // Print path to current working directory
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("\n%s %s", cwd, "--------------------------------------------------");
        printf("\nShell: ");

        // Read command
        // line = sh_read_line();
        fgets(line, INPUT_MAX, stdin);
        // scanf("%s", line);
        // gets(line);
        // printf("yunk\\n");
        // printf("LINE: %s\n", line);
        // fork and pipe (if more than 1 command)
        status = fork_pipes2(line);

        // printf("is wackkkk\n\n");


    } while (status);
}

int
main ()
{
  intcmd_loop();
  // printf("ya boiiiiiii\n\n");
  
  return 1;
}

