#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* needed to define exit() */
#include <sys/types.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>



using namespace std;



int STD_IN = 0;
int STD_OUT = 1;
fstream log;


string remove_leading(string str){
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::bind1st(std::not_equal_to<char>(), ' ')));
    return str;
}

string remove_trailing(string str){
    str.erase(std::find_if(str.rbegin(), str.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), str.end());
    return str;
}


//takes in input of "cat -l x.txt"
//"ls"
// "cat x.txt
void executeFunc(string commandWithArgs){
    int argCount = 0;
    istringstream iss;
    iss.str(commandWithArgs);

    for (unsigned i = 0; i <= commandWithArgs.length(); i++)
    {
        if (commandWithArgs[i] == '\0' || commandWithArgs[i] == ' ')
            argCount++;
    }

    string array[argCount+1];
    for (int i = 0; i < argCount; i++)
        iss >> array[i];
    //find the number of args+ 1 command , excluding all empty spaces
    int realCount =0;
    for(int i = 0; i < argCount; i ++){
        if(array[i] != " " && array[i] != "" && array[i].size() != 0){
            //cout << "found : ..." << array[i] << "...\n";
            realCount++;
        }
    }
    //fill the array argv with command and arguments
    char*argv[realCount + 1];
    int count = 0;
    for(int i = 0; i < argCount; i ++){
        if(array[i] != " " && array[i] != "" && array[i].size() != 0){
            argv[count] = const_cast<char*>(array[i].c_str());
            count++;
            //cout << "," << array[i] << "," << endl;
        }
    }


    argv[realCount] = NULL;

    if (strcmp(argv[0], "cd") == 0){
        cout << "cd\n";
        cout << argv[1] << endl;
        chdir(argv[1]);
        char s[100];
        printf("%s\n",getcwd(s,100));
        exit(0);
    } else {
        execvp(argv[0], argv);
    }
    perror("Error: ");

}

void outputRedirection_func(string file_name1) {

  //cout << "outputRedirection()";
    close(1);
    int file = open(file_name1.c_str(), O_RDWR);
    if(file < 0 ){
        cout << "Error: Opening file Failed" << endl;
    }

    //redirect Std input (0) to file
    if(dup2(file,1) < 0)    {
        close(file);
        //return 1; // this can return 0 normally
    }
    //cout << "opened: filename = " << file_name1 << endl;

}


void pipeFunc(bool outputRedirection, vector<string> programs, string fileName) {
  //log.open("log.txt", fstream::app); // for debugging
    int fds[programs.size()][2];


    for(int i = 0; i < programs.size() ; i++){

      //    log << "i = " << i << endl;
        if (pipe(fds[i]) == -1) {
            fprintf(stderr, "Error: Pipe failed");
            exit(1);
        }
        pid_t pid1 = fork();

        if (pid1 == 0) { // child
            if(i > 0){
	      //                log << "in 2nd > cmd  grab input from pipe" << endl;
                close(fds[i -1][1]);
                close(0);
                dup(fds[i - 1][0]);
                close(fds[i-1][0]);
                /*
                close(STD_IN); // redirect input to pipe.
                close(fd[1]);
                dup2( fd[0], STD_IN );
                 */
            }

            if( i == programs.size() - 1) { //i is the last item
	      //log << "last item : " << i << endl;
                if (outputRedirection) {
		  //    log << "--output redirection found " << endl;
                    outputRedirection_func(fileName);
                }
            }else {



	      // log << " redirecting output " << i << "   to pipe" << endl;
                close(fds[i][0]); // redirect output to pipe
                close(1);
                dup(fds[i][1]);
                close(fds[i][1]);

                /*
                close(STD_OUT); // redirecting output to pipe.
                close(fd[0]);
                dup2( fd[1], STD_OUT );
                 */

            }
            //sleep(1);


            executeFunc(programs[i]);
            exit(1);


        }
        close(fds[i][1]);
        //close(fds[i][0]);
        waitpid(pid1,NULL,0);
	// printf("Child 1 exit\n");
    }
    exit(1);
}



void func(int signum) {
  int status;
  // cout << signum << endl;
  waitpid(signum, &status, 0); 
}
/*
void proc_exit() {
  int wstat;
  union wait wstat;
  pid_t pid;
  while(1){
    pid = wait3(&wstat, WNOHANG, (struct rusage *)NULL);
  }
}
*/


int main(int argc, char ** argv) {

    string str = "-n";
    string amperand = "&\n";
    int tokens = 0;
    pid_t pid;
    int status;

    int shell = 1;
    do {

        signal(SIGINT, SIG_IGN);

        if (argc == 1) {
            printf("shell: ");
        }
        else if(argv[1] == str) {
        }

        string buffer;
        getline(cin,buffer);
	//	cout << "passed getline\n";
        if (std::cin.eof()==1) {
	  //wait3(0,WNOHANG, NULL);
	  // signal(SIGCHLD, proc_exit);
	  //waitpid(-1, NULL, WNOHANG);
	  //signal(SIGCHLD, SIG_IGN);
            exit(1);
        }

	buffer = remove_leading(remove_trailing(buffer));
	
        if ((pid = fork()) > 0) { //parent thread
	  if (buffer.length() == 0){
	    waitpid(-1, &status, 0);
	  }
	  else {
            if (buffer.back() == '&') {
                //Fix
                //Needs to kill zombie process!
	      signal(SIGCHLD, func);
            } else {
                waitpid(-1, &status, 0);
                //exit(status);
            }
	  }
        } else {


            vector<string> v;
            vector<string> v1;
            vector<string> v2;
            vector<string> programs;
            int right_index, foundInput = 0;
            string file_name = "";
            string file_name1 = "";
            string program_name = "";

            bool inputIndirection = false;
            bool outputRedirection = false;
            bool pipeFound = false;
            bool singleCommand = false;

	    //cout << "here2\n";
           
	    if (buffer.length() == 0){
	      //    cout << "killing the baby \n" ;
	      exit(1);
	    }
	    //cout << "here\n";

            if (buffer.back() == '&') {
                int temp = buffer.length() - 1;
                buffer = buffer.substr(0, temp);
            }

            for (int i = 0; i < buffer.length(); i++) {
                if (buffer[i] == '<') {
                    inputIndirection = true;
		    //  printf("< found\n");
                    v.push_back(buffer.substr(0, i));   //v[0] has the program name
                    v.push_back(buffer.substr(i + 1, buffer.length() - i - 1));
                    v[0].erase(std::find_if(v[0].rbegin(), v[0].rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), v[0].end());
		    // cout << "left: ..." << v[0] << "... " <<"right: ..." <<  v[1] << "..."  << endl;
                    programs.push_back(v[0]);
                    for (int j = i + 1; j < buffer.length() ; j++) {
                        //cout << buffer[j] << endl;
                        if (buffer[j] == '|' || buffer[j] == '>' || buffer[j] == '&') {
                            //cout << buffer.substr(i + 1, j - 3) << endl;
                            file_name = buffer.substr(i + 1, j - i - 1);
                            file_name = remove_leading(remove_trailing(file_name));
			    //      cout << "-file_name: ..." << file_name << "..." << endl;
                            break;
                        }
                    }

                    if (file_name == "") {
                        file_name = v.back();
                        file_name.erase(file_name.begin(), std::find_if(file_name.begin(), file_name.end(), std::bind1st(std::not_equal_to<char>(), ' ')));
                        file_name.erase(std::find_if(file_name.rbegin(), file_name.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), file_name.end());
			// cout << "-file_name: ..." << file_name << "..." << endl;

                    }
                }

                if (buffer[i] == '|') {
                    pipeFound = true;
		    // printf("| found\n");
		    // cout << buffer << endl;
		    int counter = 0;
                    if(programs.size() == 0) {
		      counter = 2;
		      // cout << "testA\n";
                        v2.push_back(buffer.substr(0, i));
                        v2[0] = remove_leading(remove_trailing(v2[0]));
                        programs.push_back(v2[0]);
			//	cout << "testB\n";

                    //v[0] has the program name
                    v2.push_back(buffer.substr(i + 1, buffer.length() - i - 1));
                    v2[1] = remove_leading(v2[1]);
                    v2[1] = remove_trailing(v2[1]);

                    }else{
		      counter = 1;
		      // cout << "testB\n";

                    //v[0] has the program name
                     v2.push_back(buffer.substr(i + 1, buffer.length() - i - 1));
                     v2[0] = remove_leading(v2[0]);
                     v2[0] = remove_trailing(v2[0]);
		     // cout << "left: ..." << v2[0] << endl;
		    }
                    for (int j = i + 1; j < buffer.length() ; j++) {

                        if (buffer[j] == '|' || buffer[j] == '>' || buffer[j] == '&') {
                            program_name = buffer.substr(i + 1, j - i - 1);
                            program_name = remove_leading(program_name);
                            program_name = remove_trailing(program_name);
			    //     cout << "-program_name: ..." << program_name << "..." << endl;
                            programs.push_back(program_name);
                            break;
                        }
                    }

		    //   cout << "reached\n";
                    if (program_name == "") {
                        program_name = v2.back();
                        program_name = remove_leading(program_name);
                        program_name = remove_trailing(program_name);
			//  cout << "-program_name: ..." << program_name << "..." << endl;
                        programs.push_back(program_name);
                    }
		    // cout << "reached1\n";

		    // cout << "size of v2 = " <<v2.size() << endl;
                    program_name = "";
		    // cout << "reached4\n";
		    // cout << v2.back() << endl;

                    v2.pop_back();
		    if(counter == 2){
		      v2.pop_back();
		    }
		    // cout << "reached5\n";
		    //cout << v2.back() << endl;
                    //v2.pop_back();
                    //cout << "Size of v2 = " << v2.size() << endl;
		    // cout << "reached3\n";

                    //cout<< "resulting vector : " << v << endl;

                }

                if (buffer[i] == '>') {
                    outputRedirection = true;
		    // printf("> found\n");
                    //cout << buffer << endl;
                    v1.push_back(buffer.substr(0, i));
                    v1.push_back(buffer.substr(i + 1, buffer.length() - i - 1));
                    v1[0] = remove_leading(remove_trailing(v1[0]));

                    //add program to program vector if  ls > x.txt
                    if( !inputIndirection && !pipeFound)
                        programs.push_back(v1[0]);
                    v1[1] = remove_leading(v1[1]);
                    v1[1] = remove_trailing(v1[1]);
		    // cout << "left: " << v1[0] << "\n" <<"right: " <<  v1[1] << endl;
                    file_name1 = v1[1];
		    // cout << "-file_name: ..." << file_name1 << "..." << endl;
                    //cout<< "resulting vector : " << v << endl;

                }

                if ((!inputIndirection && !outputRedirection && !pipeFound) && (i == buffer.length()-1)) {
                    singleCommand = true;
		    if(buffer.length() == 0 || buffer == "\n") exit(1);
                    executeFunc(buffer);
                }

            }




            if (inputIndirection){
                close(0);
                int file = open(file_name.c_str(), O_RDWR);
                if(file < 0 ){
                    perror("Error: ");
                    return 1;
                }

                //redirect Std input (0) to file
                if(dup2(file,0) < 0)    {
                    close(file);
                    return 1; // this can return 0 normally
                }
		// cout << "command = " << v[0] << ", filename = " << file_name << endl;

                int status1;
                if (programs.size() == 1){
                    if(outputRedirection) {
                        outputRedirection_func(file_name1);
                    }
                    //cout << "Execa " << v[0] << endl;
                    executeFunc(v[0]);
                }else{
                    pipeFunc(outputRedirection,programs, file_name1);
                }

                close(file);

               

            } else {

                if (!singleCommand){
                    if (programs.size() == 1){
                        if(outputRedirection) {

                            outputRedirection_func(file_name1);
                        }
                        executeFunc(programs[0]);
                    }else{
                        pipeFunc(outputRedirection,programs, file_name1);
                    }
                }

            }
        }
    } while (shell);
}










