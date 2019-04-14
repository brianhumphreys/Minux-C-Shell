#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define INPBUF 100
#define BUFF_RL 64
#define TOKEN_BUFSIZE 64
// #define TOKEN_DELIMITER " \t\r\n\a"
#define TOKEN_DELIMITER "|"

struct command{
  
  int length;
  char **argv;
};

struct command2{
  

  char **argv;
};
int
spawn_proc (int in, int out, struct command2 *cmd){
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
      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }
  return pid;
}
int
spawn_proc2 (int in, int out, char** argv){
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

int
fork_pipes (int n, struct command2 *cmd)
{
  int i;
  pid_t pid;
  int in, fd [2];

  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  for (i = 0; i < n - 1; ++i)
    {


      pipe (fd);

      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      spawn_proc (in, fd [1], cmd + i);

      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      /* Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];
    }

  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */  
  if (in != 0)
    dup2 (in, 0);

  /* Execute the last stage with the current process. */
  return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv);
}


// struct command* getNextCommand(char *line) {

//   return struct command cmd;
// }

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

void printPP(int argc, char ** args) {
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
  // struct command * cmd;
  char** argv;
  char* tkn;
  int n;

  // copy input line
  char* inputptr = strdup(line);
  char* strCopy = strdup(line);

  // printf("This the string yo: %s\n", line);

  for(n=0; strCopy[n]; strCopy[n]=='|' ? n++ : *strCopy++);

  // printf("Length of string: %i\n", n+1);



  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  tkn = strtok(inputptr, TOKEN_DELIMITER);
  
  // printf("token 1: %s\n", tkn);
  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  if(n>0) {
    for (i = 0; i < n; ++i)
    {

      int ac;

      // printf("THIS IS THE LINE: %s\n", tkn);
      // cmd = getNextCommand(inputptr);
      argv = parsedargs(tkn, &ac);

      printPP(n+1, argv);

      pipe (fd);

      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      spawn_proc2(in, fd [1], argv);

      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      /* Keep the read end of the pipe, the next child will read from there.  */
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

    // printf("THIS IS THE LINE: %s\n", tkn);
    // cmd = getNextCommand(inputptr);
    argv = parsedargs(tkn, &ac);

    printPP(n+1, argv);

    return execvp (*argv, (char * const *)argv);
  }
  
}

void printCommands(int cmdCount, struct command* cmds) {
  int i;
  // printf("yunk");

  printf("yunk: %s %s\n", cmds[0].argv[0], cmds[0].argv[1]);
  
  printf("yunk: %s %s\n", cmds[1].argv[0], cmds[1].argv[1]);
  
  printf("yunk: %s %s\n", cmds[2].argv[0], cmds[2].argv[1]);
  

  for(i=0; i<cmdCount; i++) {
    // printf("yunk");
    printf("___________________\n");
    

    char length = cmds[i].length;
    printf("length: %i\n", length);
    printf("command: %i\n", i);
    int j;
    printf("Next Command\n");
    for(j=0; j<length; j++) {
      // printf("yunk");
      printf("this tkn: %i\n", j);
      printf("%s \n", cmds[i].argv[j]);
    }
    printf("done with this command\n");
    // char* ptr;
    // int j = 0;
    // printf("next command: %s\n", tkn);
    // while(*ptr != '\0')
    // {
    //     printf("%s", *(tkns+j));
    //     ptr++;
    //     j++;
    // }
    // while(tkn != NULL) {
    //   printf("%s ", tkn);
    //   tkn++;
    // }
  }
  printf("uuhhhhhhhh\n");
}

void printCommands2(int cmdCount, struct command cmds[]) {
  int i;
  printf("PRINT COMMAND\n");
  for(i=0; i<cmdCount; i++) {
    char** oneCommand;
    int size = 1;

    for (oneCommand = cmds[i].argv; *oneCommand != '\0'; oneCommand++) {
        size++;
        printf("%s ", *oneCommand);
    }
    
    printf("\nSIZE: %i\n", size);

  }
    
}

int sh_line_split2(char *line) {
    int buf_s = TOKEN_BUFSIZE;
    int maxcmds = 10;
    int posidx = 0;
    int cmdno = 0;
    int p_count;
    
    char **tkns;
    char **og = tkns;
    char *tkn;

    int length = 0;

    char *strCopy = strdup(line);
    // // Figure out how many individual commands are present in input
    for(p_count=0; strCopy[p_count]; strCopy[p_count]=='|' ? p_count++ : *strCopy++);
    struct command cmd[p_count+1];

    if (!tkns) {
        fprintf(stderr, "shell error when allocating memory\n");
        exit(EXIT_FAILURE);
    } 

    // get first token
    tkn = strtok(line, TOKEN_DELIMITER);
    
    //walk through other tokens
    while(tkn != NULL) {
        if(*tkn=='|') {
          
        }
        else {
          strcpy(*tkns , tkn);
          tkns++;


          if (posidx >= buf_s) {
              // printf("reallocation\n");
              // buf_s += TOKEN_BUFSIZE;
              // *tkns = realloc(*tkns, buf_s * sizeof(char*));
              
              // reallocation failure check
              if (!tkns) {
                  fprintf(stderr, "shell error when reallocating memory\n");
                  exit(EXIT_FAILURE);
              }
          }
          // get next token
          
        }
        tkn = strtok(NULL, TOKEN_DELIMITER);
    }
    // printf("printed test\n");
    // // printf("yunk");
    // // put null as last value in tokens
    // tkns[posidx] = 0;
    // // tkns[posidx] = 0;
    // cmd[cmdno].argv = tkns;
    // cmd[cmdno].length = length;
    // tkns = og;
    // // printf("yunk");
    // // printCommands(p_count+1, cmd);

    printCommands2(p_count+1, cmd);

    printf("printed test\n");
    // return tkns;
    free(tkn);

    return 1;
}

// int sh_line_split(char *line) {
//     int buf_s = TOKEN_BUFSIZE;
//     int maxcmds = 10;
//     int posidx = 0;
//     int cmdno = 0;
//     // int len, i;
//     int p_count;
//     // char *ptr;
//     // char **tkns;
//     // char *tkns[] = malloc(maxcmds * sizeof(char*));
    
//     // char *tkns[sizeof(char*) * 548]; // = malloc(sizeof(char*) * 548);
//     // char **cmdptr = tkns;
//     char *tkn;

//     int length = 0;

    
    
//     char *strCopy = strdup(line);
//     // // Figure out how many individual commands are present in input
//     for(p_count=0; strCopy[p_count]; strCopy[p_count]=='|' ? p_count++ : *strCopy++);

//     struct command cmd[sizeof(struct command*) * (p_count+1)];



//     // if (!tkns) {
//     //     fprintf(stderr, "shell error when allocating memory\n");
//     //     exit(EXIT_FAILURE);
//     // } 

//     // get first token
//     printf("last test: %s\n", line);
    
    
//     //walk through other tokens
//     tkn = strtok(line, TOKEN_DELIMITER);
//     while(tkn != NULL) {

//         char *tkns[sizeof(char*) * 548];
//         char **cmdptr = tkns;
//         printf("new one: %s\n", tkn);
        

//         while(*tkn != '|'){
//           printf("new one: %s\n", tkn);
//           // printf("loop begin %s\n", tkn);
//           printf("there\n");
//           tkns[posidx] = tkn;
//           printf("TESTING: %s\n", tkns[0]);
//           // strcpy(tkns[posidx], tkn);
//           posidx++;
//           length++;
//           if (posidx >= buf_s) {
//               printf("reallocation\n");
//               buf_s += TOKEN_BUFSIZE;
//               *tkns = realloc(*tkns, buf_s * sizeof(char*));
              
//           }
//             // get next token
            
          
//           tkn = strtok(NULL, TOKEN_DELIMITER);
//           printf("does: %d\n", tkn==NULL);
//           // printf("new one: %s\n", tkn);
//           printf("yunk\n");
//           printf("yunk\n");
//           printf("yunk\n");
//         }
        
//         tkns[posidx] = NULL;
//         cmd[cmdno].argv = tkns;
//         cmd[cmdno].length = length;
//         printf("other test: %s\n", cmd[0].argv[0]);
//         length = 0;
//         posidx = 0;
//         cmdno++;

//         tkn = strtok(NULL, TOKEN_DELIMITER);
        
//     }
//     printf("printed test\n");
//     // printf("yunk");
//     // put null as last value in tokens
//     // tkns[posidx] = 0;
//     // // tkns[posidx] = 0;
//     // cmd[cmdno].argv = tkns;
//     // cmd[cmdno].length = length;
//     // tkns = og;
//     // printf("yunk");
//     // printCommands(p_count+1, cmd);

//     // printCommands2(p_count+1, cmd);

//     printf("printed test\n");
//     // return tkns;
//     free(tkn);

//     return 1;
// }

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

        // printf("%s\n", line);
        fork_pipes2(line);

        // Parse command
        // int test = sh_line_split(line);
        // printf("done %i\n", test);
        // printf("arg: %s\n", args[0]);
        // while ( *args ) printf( "%s\n", *args++ );

        // Run command
        // status = sh_exec(args);

    } while (status);
}

int
main ()
{
  int p_count;

  char *ls[] = { "ls", "-l",0 };
  char *ls2[] = { "sort",0 };
  char *awk[] = { "awk", "{print $9}",0 };
  char *sort[] = { "sort", 0 };
  char *uniq[] = { "uniq", "-u", 0 };

  const char *cmd2 = "ls -l\n";

  // printf("cmd: %c", ls[1][0]);
  // char* line = sh_read_line();

  // for(p_count=0; strCopy[p_count]; strCopy[p_count]=='|' ? p_count++ : *strCopy++);

  // printf("cmd: %s", line);

  intcmd_loop();

  // struct command2 cmd[] = { {ls}, {awk}, {sort}, {uniq} };
  struct command2 cmd[] = { {ls}, {ls2} };
  // printf("cmd %s", cmd[0].argv[0]);

  // return 0;
  // return fork_pipes2 (line);
  // return fork_pipes (4, cmd);
  return 1;

}

