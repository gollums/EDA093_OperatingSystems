/* 
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
      $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);

void run_command(Command *);
void run_pgm_recursive(Pgm *, int);
void safe_pipe(int *);

void sigint_handler(int);
void sigchld_handler(int);

/* When non-zero, this global means the user is done using this program. */
int done = 0;

/**/
char pwd[4096];
/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  getcwd(pwd, sizeof(pwd));
  signal(SIGINT,sigint_handler);
  signal(SIGCHLD,sigchld_handler);

  while (!done) {

    char *line;
    /*
    printf("%s", pwd);
    */
    line = readline("> ");

    if (!line) {
      /* Encountered EOF at top level */
      done = 1;
    }
    else {
      /*
       * Remove leading and trailing whitespace from the line
       * Then, if there is anything left, add it to the history list
       * and execute it.
       */
      stripwhite(line);

      if(*line) {
        add_history(line);
        /* execute it */
        parse(line, &cmd);
        run_command(&cmd);
      }
    }
    
    if(line) {
      free(line);
    }
  }
  return 0;
}

/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (whitespace( string[i] )) {
    i++;
  }
  
  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && whitespace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}

void run_command(Command *cmd)
{
  Pgm *p = cmd->pgm;
  int bg = cmd->bakground;
  char **pl = p->pgmlist;
  char *infile = cmd->rstdin;
  char *outfile = cmd->rstdout;
  int infd = STDIN_FILENO;
  int outfd = STDOUT_FILENO;
  pid_t pid;

  if (0 == strcmp("exit", *pl)) {
    exit(EXIT_SUCCESS);
  } else if (0 == strcmp("cd", *pl)) {
    pl++;
    if (-1 == chdir(*pl)) perror(NULL);
    getcwd(pwd, sizeof(pwd));
  } else {
      
      if (outfile) {
	outfd = creat(outfile, S_IWUSR | S_IRUSR);
      }
      if (infile) {
	infd = open(infile, O_RDONLY);
      }

      pid = fork();
      if (-1 == pid) {
	perror(NULL);
      }
      if (0 == pid) {
	if (bg) signal(SIGINT, SIG_IGN);
	dup2(infd,STDIN_FILENO);
	run_pgm_recursive(p,outfd);
	exit(EXIT_SUCCESS);
      } else {
        if (!bg) waitpid(pid, NULL, 0);
      }
    }
}

void run_pgm_recursive(Pgm *p, int outfd){
  char **pl = p->pgmlist;
  pid_t pid;
  int pipefd[2];


  if((p->next) == NULL) {
    dup2(outfd,STDOUT_FILENO);
    if (-1 == execvp(*pl, pl)){
      perror(NULL);
      exit(EXIT_FAILURE);
    }
  }
  else {
    safe_pipe(pipefd);
    pid = fork();
    if (-1 == pid){
      perror(NULL);
    }
    if (0 == pid) {
      /*CHILD*/
      close(pipefd[0]);
      run_pgm_recursive(p->next, pipefd[1]);
      exit(EXIT_SUCCESS);
    } else{
      /*PARENT*/
      close(pipefd[1]);
      waitpid(pid,NULL,0);
      dup2(outfd,STDOUT_FILENO);
      dup2(pipefd[0],STDIN_FILENO);
      if (-1 == execvp(*pl,pl)) {
	perror(NULL);
	exit(EXIT_FAILURE);
      }
    }
  }
	    
}

void safe_pipe(int *pipefd){
  if (-1 == pipe(pipefd)){
    perror(NULL);
  }
  return;
}

void sigint_handler(int signo){
  if (signo == SIGINT) {
    printf("\n%s> ", pwd);
  }
}

void sigchld_handler(int signo){
  if (signo == SIGCHLD) {
    waitpid(-1, NULL, WNOHANG);
  }
}
