#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>
#define BLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"
#define MAXLINE 1024
#define MAXARGS 128

// Tahyr Bayryyev


// declaring the volatile variable
static volatile sig_atomic_t interuppted = 1;


// the signal handler function to handle ^C signal
void intHandler(int signal) {
    interuppted = 0;    
    
}



// our main function
int main() {

char cmdline[MAXLINE]; /* command line */


// signal action initilization
struct sigaction signalAction;
// reset memory to get no errors
memset(&signalAction, 0, sizeof(signalAction));
// set our handler to our own handler function
signalAction.sa_handler = intHandler;

// make the sigaction call to check for SIGINT signal and check if it fails 
if(sigaction(SIGINT, &signalAction, NULL)==-1){
    fprintf(stderr,"Error: Cannot register signal handler. %s.\n",strerror(errno));
    return EXIT_FAILURE;

}



// the while loops which keeps looping while there is something to read 
while (1) {
    

    char* temp = getcwd(NULL, 0); // get current directory


    // check if we cannot get the current working directory
    if(temp == NULL){
        fprintf(stderr,"Error: Cannot get current working directory. %s.\n",strerror(errno));
        free(temp);
        return EXIT_FAILURE;

    }
 
  
    //print the current directory
    printf("[");
    printf("%s%s", BLUE,temp);
    printf("%s]",DEFAULT);
    free(temp);
    printf("%s> ",DEFAULT);

    // get the input from stdin and put into cmdline variable
    char *s = fgets(cmdline, MAXLINE, stdin);

    // check if the signal was interuppted by ^C and continue
    if(interuppted == 0){
        interuppted = 1;
        printf("\n");
        continue;
    }

    // check if we fail to read from stdin
    if(s == NULL){
        fprintf(stderr,"Error: Failed to read from stdin. %s.\n",strerror(errno));
        return EXIT_FAILURE;

    }
  

  

// get the command only of the command line not the arguements
    int i = 0;

    int size_of_function = 0;

    // find the size of the command
    while(cmdline[i] != '\n'){
        if(cmdline[i]== ' '){
            break;
        }
        i++;
        size_of_function++;
    }

    // initialize the command string and copy the command only from the command line 
    char command[size_of_function+1];
    memcpy(command,&cmdline[0],size_of_function);
    command[size_of_function]='\0';

    // finding the length of arguments string
    int arguments_length = strlen(cmdline)-2-size_of_function;
    char arguments[arguments_length+1];


    int num_arguments =0;
// get the arguments of the command line if there are any
    if(cmdline[i]!= '\n'){
            
            // if there are copy the arguments part of the commandline into the arguments string
            memcpy(arguments,&cmdline[size_of_function+1],arguments_length);

            // always null terminate
            arguments[arguments_length] = '\0';
            int z = 0; //index
            // count the number of arguments in the arguments string
            while(arguments[z] != '\0'){
                if(arguments[z]==' '){
                    num_arguments++;
                }
                z++;
            }

            // account for the space after the command and before the first argument
            num_arguments++;

    }


    if(strcmp(cmdline,"\n")==0){
        continue;
    }

    // CD command
    if(strcmp(command,"cd")==0){

        // home directory case
        if(strcmp(cmdline,"cd\n")==0 || strcmp(cmdline, "cd ~\n") == 0){
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);

        // error for getpwuid function
        if(!pw){
            fprintf(stderr,"Error: Cannot get passwd entry. %s.\n",strerror(errno));
            continue;

        }


        if(chdir(pw->pw_dir)!=0){
            fprintf(stderr,"Error: Cannot change directory to '%s'. %s.\n",pw->pw_dir,strerror(errno));
        }

        
        }else{
            // index after cd and space 
            int i = 3;
            int truth = 0;

            // check if there are too many arguments passed to cd function
            while(cmdline[i]!= '\n'){
                if(cmdline[i]== ' '){
                    fprintf(stderr,"Error: Too many arguments to cd.\n");
                    truth = 1;
                    break;
                }
                i++;
            }

            // if there are just continue and don't change directory
            if(truth == 1){
                continue;
            }

            // otherwise get the directory name from the command line passed in to stdin
            int num_characters = strlen(cmdline) - 3;
            char dir_name[num_characters];
            memcpy(dir_name,&cmdline[3],num_characters-1);
            dir_name[num_characters-1]='\0';
            // check if we cannot change the directory
            if(chdir(dir_name)!=0){
                fprintf(stderr,"Error: Cannot change directory to '%s'. %s.\n",dir_name,strerror(errno));
             
                


            }
         
        }
        
        
    // exit command;
    }else if (strcmp(cmdline, "exit\n") == 0){
        return EXIT_SUCCESS;
        

    }else{
        pid_t pid;
        int stat;
        int pid_1;

        // building our own argv for the execv command
        // length of argv = one for the null terminator, one for command, and the rest for the number of arguments
        int argv_elements = num_arguments+2;
        char* argv_mine[argv_elements]; 
        argv_mine[0] = command;
        argv_mine[argv_elements-1] = NULL;


        // case for when there are no arguments we hae to add them manually use strtok
        if(num_arguments!= 0 ){
            char* argument = strtok(arguments, " ");  // seperate strings by a space
            int i = 1;

            // go through all the arguments
            while( argument != NULL ) {
                argv_mine[i] = argument;    // add the argument to our argv array

                
                argument = strtok(NULL, " ");  // get the next argument
                i++;  // increase the index
            }
        }
            
        




        // Both child and parent will now start execution from here.
        if((pid=fork()) < 0) {
        //child was not created successfully
            fprintf(stderr,"Error: fork() failed. %s.\n",strerror(errno));
            exit(1);
            
        }
        else if(pid == 0) {

        // This is the child process
        // Child process code goes here
            if (execvp(command, argv_mine) < 0) {
                    fprintf(stderr,"Error: exec() failed. %s.\n",strerror(errno));
                    exit(1);
                }
        }else {
            pid_1 = waitpid(pid,&stat,0);

            // signal was not interuppted thus there was just error with wait function
            if(pid_1==-1 && interuppted!=0){
                fprintf(stderr,"Error: wait() failed. %s.\n",strerror(errno));
                exit(1);

                
            }

            // here signal was interuppted thus it was not just error with wait set interuppted back to initial state
            if(pid_1==-1 && interuppted == 0){
                interuppted = 1;
                printf("\n");
                

            }
        }
    }
}
}
