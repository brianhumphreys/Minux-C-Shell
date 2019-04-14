
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define INPBUF 100
#define BUFF_RL 64
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIMITER " \t\r\n\a|><"


struct _instr
{
    char * argval[INPBUF];
    int argcount;
};
typedef struct _instr instruction;

// ##########################################################
// HELPER FUNCTIONS
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

// ##########################################################
// BUILT-IN SHELL FUNCTIONS
// This is needed so process does not conflict with process properties
// child processes should inherit properities of parent

// Forward Declarations of built in functions
int s_cd(char **args);
int s_help(char **args);
int s_exit(char **args);

// this is easier than a switch statement because now we can add
// new functinos by adding to the array and implementing their function
// below.
char *builtin_cmd_str[] = {

    "exit",
    "help",
    "cd"
};

// Array of function pointers that takes an array of strings and
// returns an int
int (*builtin_func[]) (char **) = {
    &s_exit,
    &s_help,
    &s_cd
};

int sh_builtin_count() {
    return sizeof(builtin_cmd_str) / sizeof(char *);
};

// Builtin functino implementations
int s_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected an argument following \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int s_help(char **args) {
    printf("Shell made by Brian Humphreys\n");
    printf("Type any of the listed commands and press enter/\n");
    printf("The following commands are built in:\n");

    int i;
    for (i=0; i<sh_builtin_count(); i++) {
        printf(" %s\n", builtin_cmd_str[i]);
    }

    return 1;
}

int s_exit(char **args) {
    return 0;
}




// char *lsh_read_line(void)
// {
//   char *line = NULL;
//   ssize_t bufsize = 0; // have getline allocate a buffer for us
//   getline(&line, &bufsize, stdin);
//   return line;
// }

struct command{
  const char **argv;
};
int
spawn_proc (int in, int out, struct command *cmd){
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
fork_pipes (int n, struct command *cmd)
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



// this is abstracted by a layer from sh_exec() because if we performed
// a process to change directories, it would just change the directory of the
// child and then terminate, leaving the parent untouched.  This is because
// the current directory is a property of a process
int sh_spawn(char **args)
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
        if (execvp(args[0], args) == -1) {
            perror("shell");
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


int sh_exec(char **args) {
    if (args[0] == NULL) {
        // In this case, an empty command was entered.  Do nothing
        return 1;
    }

    int i;
    for (i=0; i<sh_builtin_count(); i++) {
        if (strcmp(args[0], builtin_cmd_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    // if built in function was not called, we will fall back
    // to launching command instead
    return sh_spawn(args);
}


// Reading a line function
// I will use normal C strategy of reading line of user input which is
// allocating an initial block and if the user exeeds this block size, 
// then allocate more memory
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

// Splitting a line into arguments by on tokenizing string based 
// on whitespace, |, >, < delimiters.  We can use strtok to do much of the work
// for us
// We will again, use a buffer and dynamically expand as needed

char **sh_line_split(char *line) {
    int buf_s = TOKEN_BUFSIZE;
    int maxcmds = 10;
    int posidx = 0;
    // int cmdno = 0;
    // int len, i;
    int p_count;
    // char *ptr;
    // char **tkns;
    char **tkns = malloc(maxcmds * sizeof(char*));
    char *tkn;
    
    char *strCopy = strdup(line);
    // // Figure out how many individual commands are present in input
    for(p_count=0; strCopy[p_count]; strCopy[p_count]=='|' ? p_count++ : *strCopy++);

    if (!tkns) {
        fprintf(stderr, "shell error when allocating memory\n");
        exit(EXIT_FAILURE);
    } 

    // get first token
    tkn = strtok(line, TOKEN_DELIMITER);
    
    //walk through other tokens
    while(tkn != NULL) {
        tkns[posidx] = tkn;
        posidx++;
        if (posidx >= buf_s) {
            buf_s += TOKEN_BUFSIZE;
            tkns = realloc(tkns, buf_s * sizeof(char*));
            
            // reallocation failure check
            if (!tkns) {
                fprintf(stderr, "shell error when reallocating memory\n");
                exit(EXIT_FAILURE);
            }
        }
        // get next token
        tkn = strtok(NULL, TOKEN_DELIMITER);
    }

    // put null as last value in tokens
    tkns[posidx] = NULL;
    return tkns;
}


// Interpret Command Loop
// We will:
// 1) Read: Read the command from standard input.
// 2) Parse: Separate the command string into a program and arguments.
// 3) Execute: Run the parsed command.
void intcmd_loop() {
    char status;
    char **args;
    char *line;
    // int argc;

    do {
        // Personal touch: provides location reference before each prompt
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s %s", cwd, "--------------------------------------------------");
        printf("\nShell: ");

        // Read command
        line = sh_read_line();
        // printf("%s", line);

        // Parse command
        args = sh_line_split(line);
        // while ( *args ) printf( "%s\n", *args++ );

        // Run command
        status = sh_exec(args);

    } while (status);
}


// ##########################################################
// BASIC LIFE TIME OF A SHELL
// ##########################################################
// 1) Initialize: In this step, a typical shell would read and execute its 
// configuration files. These change aspects of the shell’s behavior.
// 2) Interpret: Next, the shell reads commands from stdin (which could be 
// interactive, or a file) and executes them.
// 3) Terminate: After its commands are executed, the shell executes any 
// shutdown commands, frees up any memory, and terminates.
int main(int argc, char **argv) {
    //Step 1: load config files
    // while ( *argv ) printf( "%s\n", *argv++ );

    //Step 2: run looping command
    intcmd_loop();

    //Step 3: shut down and cleanup
    return EXIT_SUCCESS;

}