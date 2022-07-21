#include "byos.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>  //Bools
#include <signal.h>
// Find out what other #include's you need! (E.g., see man pages.)
int interp(const struct cmd *c)
{
    int returno = 0;
    if (c->type == LIST) //check if it is list
    {
        bool filecheck = false; //file is not opened
        int tempfile = -1;
        int saved_stdout = -1;
        for (int i=0; i<c->data.list.n; i++) //itterate over the number of items in list
        {
            if(c->redir_stdout != NULL) //if there is a redirection
            {
                if(!filecheck) //if file has not been opened
                {
                    saved_stdout = dup(STDOUT_FILENO); //open the file and store the STDOUT_FILENO
                    tempfile = open(c->redir_stdout,O_CREAT|O_WRONLY|O_TRUNC,0666);
                    if(tempfile<0){return 1;}
                    dup2(tempfile, STDOUT_FILENO);
                    filecheck = true; //tell the file is opened
                    close(tempfile);;
                }
            }
            returno = interp(&c->data.list.cmds[i]); //get the returns and recursive
            if(returno == 128+SIGINT) //if there is a error
                return returno;
            if(i+1 == c->data.list.n)
            {
                dup2(saved_stdout, STDOUT_FILENO); //at the end restore the STDOUT
            }
        }
        return returno;
    }
    else if (c->type == ECHO) //if there are echo commands
    {
        if(c->redir_stdout == NULL) //if there is no redirection
        {
            write(STDOUT_FILENO,c->data.echo.arg,strlen(c->data.echo.arg)); //do the simple printf
        }
        else //if there is a redirection
        {
            int Echofile = creat(c->redir_stdout, 0666); //open the file
            if(Echofile<0){return 1;} //error check
            write(Echofile,c->data.echo.arg,strlen(c->data.echo.arg)); //write on the redirectin
            close(Echofile); //close the file at end
        }
        return 0;
    }
    else if(c->type == FORX) //if there is a fork/exec xommand
    {
        bool nowcheck = false; //file is yet closed
        int saved_stdout2;
        int tempfile2;
        if(c->redir_stdout != NULL) //if the there is a redirect change STDOUT
        {
            if(!nowcheck)
            {
                saved_stdout2 = dup(STDOUT_FILENO);
                tempfile2 = open(c->redir_stdout,O_CREAT|O_WRONLY|O_TRUNC,0666);
                if(tempfile2<0){return 1;}
                dup2(tempfile2, STDOUT_FILENO); //open the file and save the og STDOUT
                nowcheck = true;
                close(tempfile2);;
            }
        }
        pid_t pid = fork(); //do the fork
        if(pid == 0) //if is the child
        {
            if((execvp(c->data.forx.pathname,c->data.forx.argv)) == -1) //do the exec also error check
            {
                perror("Error at Exec"); //if error tell the person
                exit(127);
            }
        }
        else
        {
            int status;
            if (waitpid(pid, &status, 0) != -1) //do the wait for child
            {
                if (WIFSIGNALED(status)) //if there is a signal given then exit the code
                {
                    int signum = WTERMSIG(status);
                    dup2(0, STDOUT_FILENO);
                    return (128+signum);
                }
            }
            if(nowcheck) //restore the old STDOUT_FILENO
            {
                dup2(saved_stdout2, STDOUT_FILENO);
            }
            if (WIFEXITED(status)) //send the child children code
            {
                return WEXITSTATUS(status);
            }
        }
    }
    return 0;
}

