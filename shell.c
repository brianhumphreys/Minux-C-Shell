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
        if (execvp(*argv, (char * const *)argv) == -1) {
            // printf("yunk1\n");
            perror("shell\n\n");
        } else {
          printf("executed successfully\n\n");
        }
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // printf("I am the parent\n");
        // Parent process will land here and wait
        do {
            
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // checks for signal to kill or signal that child process is completed
    } else {
        // There was an error forking
        // printf("yunk2\n");
        perror("shell");
    }
    return 1;
}

int sh_spawn_loop(char **argv)
{
    pid_t pid, wpid;
    int status=0;
    // printf("we get here!\n");
    // fork cuase a duplicate process to be made
    // after this system call, the processes will run concurrently,
    // using same program counter (for asm instr), same CPU and same
    // and same files.
    // They will run together until the parent uses waitpid to wait for the child process to 
    // finish.  
    pid = fork();
    if (pid == 0) {
        // Then the child process is running
        // if (execvp(argv[0], argv) == -1) {
        //     perror("shell\n\n");
        // } else {
        //   printf("executed successfully\n\n");
        // }
        // exit(EXIT_FAILURE);
        // printf("last one\n");
        int ret = execv (*argv, (char * const *)argv);
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
    // } 
    // wait(&status);
    // if(status < 0)
    //     perror("Abnormal exit of program ls");
    // else
    //     printf("Exit Status of ls is %d",status);
    // return 1;

}

int
spawn_proc2 (int in, int out, char** argv, bool last)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0){
    // Then the child process is running
    // printf("I am a child\n");
    //     if (last) {
    //         if (in != 0)
    //             dup2 (in, 0);
    //     }
    //   if (in != 0){
    //       dup2 (in, 0);
    //       close (in);
    //   }
        if (in != 0){
            // printf("in doesnt equal 0\n");
            if(last) {
                // printf("LAST\n");
                dup2 (in, 0);
            }
            else {
                // printf("NOT LAST\n");
                dup2 (in, 0);
                close (in);
            } 
      }
      if (out != 1){
        //   printf("in doesnt equal 1\n");
          dup2 (out, 1);
          close (out);
      }
    //   printf("LAST?: %d\n", last);
      
      // printf("One step closer\n");
      if(execvp (*argv, (char * const *)argv) == -1) {
        // printf("yunk3");
        perror("shell");
        exit(EXIT_FAILURE);
      }
      // exit(EXIT_FAILURE);
  } else if(pid > 0) {
    // parent process
    // printf("I am a parent\n");
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // checks for signal to kill or signal that child process is completed
    // printf("I have waited for child\n");
  } else {
      // There was an error forking
    //   printf("yun4");
      perror("shell");
  }
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
    //   printf("One step closer\n");
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

void printPP(int argc, char ** argv) 
{
  int i;
  for (i = 0; i < argc; i++)
        printf("[%s]\n",argv[i]);
  printf("______________\n");
}

int
fork_pipes3 (char* line)
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
//   printf("LENGTH : %i\n", n);

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
      spawn_proc2(in, fd [1], argv, i==n);

      // No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      // Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];

      tkn = strtok(NULL, TOKEN_DELIMITER);
    }
    // return 1;
    int ac;
    argv = parsedargs(tkn, &ac);
    /* Last stage of the pipeline - set stdin be the read end of the previous pipe
      and output to the original file descriptor 1. */  
    // if (in != 0)
    //   dup2 (in, 0);
    spawn_proc2(in, fd [1], argv, true);
    // printf("We are out mf\n");
    return 1;
  } else {
    int ac;

    argv = parsedargs(tkn, &ac);

    return sh_spawn(argv);
  }
  
}


// int
// fork_pipes2 (char* line)
// {
//   int i;
  
//   // pid_t pid;
//   int in, fd [2];
//   char** argv;
//   char* tkn;
//   int n;
//   // int status

//   // copy input line
//   char* inputptr = strdup(line);
//   char* strCopy = strdup(line);

//   for(n=0; strCopy[n]; strCopy[n]=='|' ? n++ : *strCopy++);

//   /* The first process should get its input from the original file descriptor 0.  */
//   in = 0;

//   tkn = strtok(inputptr, TOKEN_DELIMITER);
  
//   /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  
//     for (i = 0; i <= n; ++i)
//     {
//       int ac;
//       argv = parsedargs(tkn, &ac);

//       pipe (fd);

//       // f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
//       spawn_proc2(in, fd [1], argv);

//       // No need for the write end of the pipe, the child will write here.  */
//       close (fd [1]);

//       // Keep the read end of the pipe, the next child will read from there.  */
//       in = fd [0];

//       tkn = strtok(NULL, TOKEN_DELIMITER);
//     }

//     int ac;
//     argv = parsedargs(tkn, &ac);
//     /* Last stage of the pipeline - set stdin be the read end of the previous pipe
//       and output to the original file descriptor 1. */  
//     if (in != 0)
//       dup2 (in, 0);
//     return sh_spawn(argv);
  
  
// }

void intcmd_loop() {
    int status;

    do {
        char line[INPUT_MAX];
        // // Print path to current working directory
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("\n%s %s", cwd, "--------------------------------------------------");
        printf("\nShell: ");

        // Read command
        fgets(line, INPUT_MAX, stdin);
        // fork and pipe (if more than 1 command)
        status = fork_pipes3(line);


    } while (status);
}

int
main ()
{
  intcmd_loop();
  
  return 1;
}

