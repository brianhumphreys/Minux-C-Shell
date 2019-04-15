#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define BUFF_RL 64
#define TOKEN_DELIMITER "|"
#define INPUT_MAX 64

void printPP(int argc, char ** argv) 
{
  int i;
  for (i = 0; i < argc; i++)
        printf("[%s]\n",argv[i]);
  printf("______________\n");
}

bool parse_input_redir(char** argv) {
    int fd0,j,in=0;
    char input[64];

    for(j=0;argv[j]!='\0';j++)
    {
        // printf("this: %d\n", strcmp(argv[j],"<"));
        if(strcmp(argv[j],"<")==0)
        {        
            // printf("we got in put character\n");
            argv[j]=NULL;
            // printf("yunk1\n");
            strcpy(input,argv[j+1]);
            // printf("yunk2\n");
            in=1;       
            // printf("yunk3\n");  
            continue;  
        }               
            
    }
    // printPP(3,argv);
    //if '<' char was found in string inputted by user
    if(in)
    {   
        // printf("we got in stuff\n");
        // fdo is file-descriptor
        int fd0;
        if ((fd0 = open(input, O_RDONLY, 0)) < 0) {
            // perror("Couldn't open input file");
            exit(0);
        }           
        // dup2() copies content of fdo in input of preceeding file
        dup2(fd0, 0); // STDIN_FILENO here can be replaced by 0 

        close(fd0); // necessary
        return true;
    }
    return false;

}

bool parse_output_redir(char** argv) {
    int fd1,j,out=0;
    char output[64];

    for(j=0;argv[j]!='\0';j++)
    {          
        // printf("yunk4\n");
        if(strcmp(argv[j],">")==0)
        {      
            
            argv[j]=NULL;
            strcpy(output,argv[j+1]);
            out=2;
        }
        // printf("end iter\n");      
    }
    // printPP(3,argv);
    //if '>' char was found in string inputted by user 
    if (out)
    {
        // printf("we got out stuff\n");
        int fd1 ;
        if ((fd1 = creat(output , 0644)) < 0) {
            // perror("Couldn't open the output file");
            exit(0);
        }           

        dup2(fd1, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
        close(fd1);
        return true;
    }
    return false;
}

void parse_redirection(char** argv) {
    int fd0,fd1,j,in=0,out=0;
    char input[64],output[64];
    // printPP(3, argv);
    for(j=0;argv[j]!='\0';j++)
    {
        // printf("this: %d\n", strcmp(argv[j],"<"));
        if(strcmp(argv[j],"<")==0)
        {        
            // printf("we got in put character\n");
            argv[j]=NULL;
            // printf("yunk1\n");
            strcpy(input,argv[j+1]);
            // printf("yunk2\n");
            in=1;       
            // printf("yunk3\n");  
            continue;  
        }               
        // printf("yunk4\n");
        if(strcmp(argv[j],">")==0)
        {      
            
            argv[j]=NULL;
            strcpy(output,argv[j+1]);
            out=2;
        }
        // printf("end iter\n");      
    }


    // printf("this is it\n");
    //if '<' char was found in string inputted by user
    if(in)
    {   
        // printf("we got in stuff\n");
        // fdo is file-descriptor
        int fd0;
        if ((fd0 = open(input, O_RDONLY, 0)) < 0) {
            // perror("Couldn't open input file");
            exit(0);
        }           
        // dup2() copies content of fdo in input of preceeding file
        dup2(fd0, 0); // STDIN_FILENO here can be replaced by 0 

        close(fd0); // necessary
    }

    //if '>' char was found in string inputted by user 
    if (out)
    {
        // printf("we got out stuff\n");
        int fd1 ;
        if ((fd1 = creat(output , 0644)) < 0) {
            // perror("Couldn't open the output file");
            exit(0);
        }           

        dup2(fd1, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
        close(fd1);
    }
}

int proc_spawn(char **argv, int bg)
{
    if (!strcmp(argv[0], "exit"))
    {
        exit(0);
    }

    // FILE *fp;

    // fp = fopen("/tmp/test.txt", "w+");

    pid_t pid, wpid;
    int status;

    // fork causes a duplicate process to be made
    // after this system call, the processes will run concurrently,
    // using same program counter (for asm instr), same CPU and same
    // and same files.
    // They will run together until the parent uses waitpid to wait for the child process to 
    // finish.  
    // int file = open("Result", O_CREAT|O_WRONLY, S_IRWXU);
    
    // dup2(file, 1);
    // close(file);
    pid = fork();
    if (pid == 0) {

        parse_redirection(argv);
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
pipe_proc_spawn1 (int in, int out, char** argv, bool last, bool first, int bg)
{
    if (!strcmp(argv[0], "exit"))
    {
        exit(0);
    }
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0){
        // printf("in child");
        // bool is_infile = parse_input_redir(argv);
        // if(first && is_infile) {
        //     printf("first command has redirection\n");
        //     //nothing to be done descriptors already dupped
        //     // if there are input redirections, we will open those file descriptors
            
        // } else if(!first && is_infile) {
        //     //throw an error
        //     printf("throw error for input redirection at invalid position");
        // } else if(first && !is_infile) {
        //     //in remains 0
        // } else if 
        if(first) {

            bool isinfile = parse_input_redir(argv);
            // bool is_infile = parse_input_redir(argv);
            // if(!is_infile) {
            if (out != 1){
                dup2 (out, 1);
                close (out);
            }
            // }

        } else if(last) {
            dup2 (in, 0);
            bool is_outfile = parse_output_redir(argv);
            // bool is_infile = parse_input_redir(argv);
            // if(!is_outfile) {
            //     dup2 (in, 0);
            // }
        } else {
            if (in != 0){
                if(last) {
                    dup2 (in, 0);
                    
                }
                else {
                    dup2 (in, 0);
                    close (in);
                } 
            }
            // intermidiary piping output
            if (out != 1){
                dup2 (out, 1);
                close (out);
            }
        }
        
        // else if (){
        //     //intermediary piping input
        //     if (in != 0){
        //         if(last) {
        //             dup2 (in, 0);
                    
        //         }
        //         else {
        //             dup2 (in, 0);
        //             close (in);
        //         } 
        //     }
        // }
       

        // if(last) {
        //     printf("last\n");
        //     parse_output_redir(argv);
        // } else {
            
        //     // intermidiary piping output
        //     if (out != 1){
        //         dup2 (out, 1);
        //         close (out);
        //     }
        //     if(execvp (*argv, (char * const *)argv) == -1) {
        //         perror("shell");
        //         exit(EXIT_FAILURE);
        //     }
        // }
        if(execvp (*argv, (char * const *)argv) == -1) {

            
            perror("ERROR");
            exit(EXIT_FAILURE);
        } 
    
    } else if(pid > 0 && !bg) {
        // parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // checks for signal to kill or signal that child process is completed
        // printf("I have waited for child\n");
    } else if(pid<0) {
        // There was an error forking
        perror("Error while forking");
    }
  return 1;
}

int
pipe_proc_spawn (int in, int out, char** argv, bool last, bool first, int bg)
{
    if (!strcmp(argv[0], "exit"))
    {
        exit(0);
    }
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0){
        
        //intermediary piping input
        if (in != 0){
            if(last) {
                dup2 (in, 0);
                
            }
            else {
                dup2 (in, 0);
                close (in);
            } 
        }
        // intermidiary piping output
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
        // printf("I have waited for child\n");
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



// char* getTime(){
//     char buff[20];
//     struct tm *sTm;

//     time_t now = time (0);
//     sTm = gmtime (&now);

//     strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
//     // printf ("%s %s\n", buff, "Event occurred now");
//     return buff;
// }

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
        pipe_proc_spawn1(in, fd [1], argv, false, i==0, bg);

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
        
        pipe_proc_spawn1(in, fd [1], argv, true, false, bg);
        return 1;
    } else {
        int ac;

        argv = parsedargs(tkn, &ac);

        return proc_spawn(argv, bg);
    }
    
}

void interp_cmd_loop() {
    int status=1;
    

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
        if(*line=='\n') {
            continue;
        }
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
    // printf("What uout \\n do \'you\' expect?");
  interp_cmd_loop();
  
  return 1;
}

