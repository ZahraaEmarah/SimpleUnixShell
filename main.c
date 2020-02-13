#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAX 50

char *args[25];
int flag=0;    /// flag to check whether background process or not. background = 1, foreground =0;
int cd_flag=0; /// flag to detect cd command

void Clearfile() /// function to clear the log file
{
    FILE *filePointer ;
    filePointer =fopen("logfile.txt","w");
    fputs("********* LOG FILE *********\n",filePointer);
    fclose(filePointer);
}

void Signal_Handler(int signum)
{
    int status;
    pid_t pid;

    pid = waitpid(-1, &status, WNOHANG);
    if(pid > 0)
        Log_file(pid);
}

void Log_file(int PID) /// process ID is the parameter for the write to file function
{

    FILE *filePointer ;
    filePointer =fopen("logfile.txt","a");

    if ( filePointer == NULL )
    {
        printf( "logfile failed to open." ) ;
    }
    else
    {
        fprintf(filePointer,"Child process %d was terminated",PID);
        fputs("\n", filePointer) ;
        fclose(filePointer) ;
    }

}

void strParse(char *line)
{
    args[0] = strtok(line," "); /// takes the first word of the input line

    int length=1; /// variable to check whether there's arguments in the command or not
    int i=0;
    while(args[i] != NULL)
    {
        args[++i] = strtok(NULL," "); /// load args with the rest of the arguments
        length ++;
    }

    if(strcmp(args[0],"exit") == 0) /// if command is exit
    {
        exit(0);
    }

    if(strcmp(args[0],"cd") == 0) /// if command is cd
    {
        if(length == 2 || strcmp(args[1],"~") == 0) /// if cd is without an argument or has '~' as argument
        {
            args[1] = ".."; /// set the argument to the default directory
        }
        chdir(args[1]); /// execute cd command
        cd_flag = 1; /// set flag to 1 to prevent execvp() from executing cd command
    }

    if(!strcmp(args[i-1],"&")) /// if ampersand is the last character
    {
        flag =1 ; /// set background flag
        args[i-1] = '\0'; /// remove ampersand from the input string
    }
}

void execute_foreground()
{
    pid_t ChildPID = fork();
    if(ChildPID < 0)
    {
        perror("fork Error\n");
        return -1;
    }
    if(ChildPID != 0)
    {
        wait(NULL); /// wait untill termination
        Log_file(ChildPID); /// write process to file
    }
    else
    {
        //printf("I'm the Child %d, my parent is %d\n",getpid(),getppid());
        if(execvp(args[0],args) < 0)  /// execute
        {
            printf("Sorry, this command doesn't exist\n\n");
        }
    }
}

void execute_background()
{
    pid_t ChildPID = fork();
    if(ChildPID < 0)
    {
        perror("Error");
        return -1;
    }

    if(ChildPID != 0)
    {
    }
    else
    {
        //printf("I'm the Child %d, my parent is %d\n",getpid(),getppid());
        if(execvp(args[0],args) < 0)
        {
            printf("Sorry, this command doesn't exist\n\n");
        }

    }
}

int main()
{
    Clearfile();
    signal(SIGCHLD, Signal_Handler); /// track terminated children

    while(1)
    {
        char line[MAX];
        sleep(1);
        do
        {
            printf("Shell --> ");
            fflush(stdout);
            gets(line,MAX,stdin);  /// input
            fflush(stdout);
        }
        while(line[0] == NULL);
        fflush(stdout);

        strParse(line); /// parse input

        if(cd_flag == 0 && flag == 0)
            execute_foreground();
        else
        {
            if(cd_flag == 0)
                execute_background();
            flag = 0;
        }
        cd_flag = 0;

    }
    return 0;
}

