/***************************************************************************
 *  Title: Runtime environment 
 * -------------------------------------------------------------------------
 *    Purpose: Runs commands
 *    Author: Stefan Birrer
 *    Version: $Revision: 1.3 $
 *    Last Modification: $Date: 2009/10/12 20:50:12 $
 *    File: $RCSfile: runtime.c,v $
 *    Copyright: (C) 2002 by Stefan Birrer
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    $Log: runtime.c,v $
 *    Revision 1.3  2009/10/12 20:50:12  jot836
 *    Commented tsh C files
 *
 *    Revision 1.2  2009/10/11 04:45:50  npb853
 *    Changing the identation of the project to be GNU.
 *
 *    Revision 1.1  2005/10/13 05:24:59  sbirrer
 *    - added the skeleton files
 *
 *    Revision 1.6  2002/10/24 21:32:47  sempi
 *    final release
 *
 *    Revision 1.5  2002/10/23 21:54:27  sempi
 *    beta release
 *
 *    Revision 1.4  2002/10/21 04:49:35  sempi
 *    minor correction
 *
 *    Revision 1.3  2002/10/21 04:47:05  sempi
 *    Milestone 2 beta
 *
 *    Revision 1.2  2002/10/15 20:37:26  sempi
 *    Comments updated
 *
 *    Revision 1.1  2002/10/15 20:20:56  sempi
 *    Milestone 1
 *
 ***************************************************************************/
#define __RUNTIME_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/************Private include**********************************************/
#include "runtime.h"
#include "io.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

/************Global Variables*********************************************/
int PATHSIZE = 512;

#define NBUILTINCOMMANDS (sizeof BuiltInCommands / sizeof(char*))

typedef struct bgjob_l
{
  pid_t pid;
  struct bgjob_l* next;
} bgjobL;

/* the pids of the background processes */
bgjobL *bgjobs = NULL;


/*Set current running process to non-existent*/
//crpid=-1;
/************Function Prototypes******************************************/
/* run command */
static void
RunCmdFork(commandT*, bool);
/* runs an external program command after some checks */
static void
RunExternalCmd(commandT*, bool);
/* resolves the path and checks for exutable flag */
static bool
ResolveExternalCmd(commandT*, char*);
/* forks and runs a external program */
static void
Exec(char*, commandT*, bool);
/* runs a builtin command */
static void
RunBuiltInCmd(commandT*);
/* checks whether a command is a builtin command */
static bool
IsBuiltIn(char*);
/************External Declaration*****************************************/

/**************Implementation***********************************************/


/*
 * RunCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: none
 *
 * Runs the given command.
 */
void
RunCmd(commandT* cmd)
{
  RunCmdFork(cmd, TRUE);
} /* RunCmd */


/*
 * RunCmdFork
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   bool fork: whether to fork
 *
 * returns: none
 *
 * Runs a command, switching between built-in and external mode
 * depending on cmd->argv[0].
 */
void
RunCmdFork(commandT* cmd, bool fork)
{
  if (cmd->argc <= 0)
    return;
  if (IsBuiltIn(cmd->argv[0]))
    {
      RunBuiltInCmd(cmd);
    }
  else
    {
      RunExternalCmd(cmd, fork);
    }
} /* RunCmdFork */


/*
 * RunCmdBg
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: none
 *
 * Runs a command in the background.
 */
void
RunCmdBg(commandT* cmd)
{
  // TODO
} /* RunCmdBg */


/*
 * RunCmdPipe
 *
 * arguments:
 *   commandT *cmd1: the commandT struct for the left hand side of the pipe
 *   commandT *cmd2: the commandT struct for the right hand side of the pipe
 *
 * returns: none
 *
 * Runs two commands, redirecting standard output from the first to
 * standard input on the second.
 */
void
RunCmdPipe(commandT* cmd1, commandT* cmd2)
{
} /* RunCmdPipe */


/*
 * RunCmdRedirOut
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   char *file: the file to be used for standard output
 *
 * returns: none
 *
 * Runs a command, redirecting standard output to a file.
 */
void
RunCmdRedirOut(commandT* cmd, char* file)
{
} /* RunCmdRedirOut */


/*
 * RunCmdRedirIn
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   char *file: the file to be used for standard input
 *
 * returns: none
 *
 * Runs a command, redirecting a file to standard input.
 */
void
RunCmdRedirIn(commandT* cmd, char* file)
{
}  /* RunCmdRedirIn */


/*
 * RunExternalCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   bool fork: whether to fork
 *
 * returns: none
 *
 * Tries to run an external command.
 */
static void
RunExternalCmd(commandT* cmd, bool fork)
{
  char* path = (char*)malloc(sizeof(char)*PATHSIZE); 
  
  if(ResolveExternalCmd(cmd, path)){
    Exec(path, cmd, fork);
  }

  free(path);
}  /* RunExternalCmd */


/*
 * ResolveExternalCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: bool: whether the given command exists
 *
 * Determines whether the command to be run actually exists.
 */
static bool
ResolveExternalCmd(commandT* cmd, char* path)
{
  //Check to see if in home directory:
  if(*(cmd->argv[0])=='.'){
    
    char* cwd = getCurrentWorkingDir();
    //char* cwd;
    //getCurrentWorkingDir(cwd);
    sprintf(path,"%s/%s",cwd,cmd->name);
    free(cwd);
    return TRUE;

  }

  char** memLocs = getPath();
 
  char dest[500];
  int i=0;
  struct stat buf;


 /*If already absolute path*/
 if(stat(cmd->name,&buf)==0){
   
   /*Set path = to entered absolute path*/
   strcpy(path,cmd->name);
   freePath(memLocs);
   return TRUE;

  }

  while(memLocs[i]!=NULL){
   
    //Concatanate Paths with cmd->name:
    int size = (snprintf( dest, 499,"%s/%s",memLocs[i],cmd->name)+1)*sizeof(char);
    char* exeName =  (char*)malloc(size);
    sprintf(exeName,"%s/%s",memLocs[i],cmd->name);
  
    //Check to see if exists and executable:
    if(stat(exeName,&buf)==0){
      
      if(S_ISREG(buf.st_mode) && buf.st_mode & 0111){
         
         strncpy(path,exeName,size);
         
         freePath(memLocs);
         free(exeName);
         return TRUE;
      }
      
      freePath(memLocs);
      free(exeName);
      return FALSE;
    } 
    
    i++;
    free(exeName);
  }
  
  freePath(memLocs);
  return FALSE;
} /* ResolveExternalCmd */


/*
 * Exec
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *   bool forceFork: whether to fork
 *
 * returns: none
 *
 * Executes a command.
 */
static void
Exec(char* path, commandT* cmd, bool forceFork)
{
   pid_t child_pid;
   int childStat;
   sigset_t mask;
   
   sigemptyset(&mask);
   sigaddset(&mask, SIGCHLD);

   sigprocmask(SIG_BLOCK, &mask, NULL);

   if (!forceFork) {
	execv(path, cmd->argv);
	/*TODO maybe get rid of */ exit(0);	
   }
 
   child_pid=fork();

   /*Fork Worked?*/
   if(child_pid >= 0){
	
      /*If child process*/
      if(child_pid==0){
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
        setpgid(0,0); 
	if(execv(path,cmd->argv)==-1)   
  	   printf("Error: %s\n", strerror(errno));
	 
      }
      /*In Parent Process*/	
      else{
	crpid = child_pid;
        sigprocmask(SIG_UNBLOCK, &mask, NULL); 
	waitpid(-1, &childStat, 0);   
         
      }
      
   }
} /* Exec */


/*
 * IsBuiltIn
 *
 * arguments:
 *   char *cmd: a command string (e.g. the first token of the command line)
 *
 * returns: bool: TRUE if the command string corresponds to a built-in
 *                command, else FALSE.
 *
 * Checks whether the given string corresponds to a supported built-in
 * command.
 */
static bool
IsBuiltIn(char* cmd)
{
  return FALSE;
} /* IsBuiltIn */


/*
 * RunBuiltInCmd
 *
 * arguments:
 *   commandT *cmd: the command to be run
 *
 * returns: none
 *
 * Runs a built-in command.
 */
static void
RunBuiltInCmd(commandT* cmd)
{
} /* RunBuiltInCmd */


/*
 * CheckJobs
 *
 * arguments: none
 *
 * returns: none
 *
 * Checks the status of running jobs.
 */
void
CheckJobs()
{
} /* CheckJobs */

/*
 * getCurrentWorkingDir (NO ARGS)
 *
 * arguments: none
 *
 * returns: outputs current directory
 *
 * Print Working Directory.
 */
char*
getCurrentWorkingDir(){
  
   long size;
   char *buf;
   char *ptr =0;

   size = pathconf(".", _PC_PATH_MAX);

   if((buf = (char*)malloc((size_t)size)) != NULL)
      ptr = getcwd(buf,(size_t)size);
   
   return ptr;
}/* getCurrentWorkingDir*/


/*
 * changeWorkingDir
 *
 * arguments: none
 *
 * returns: outputs current directory
 *
 * Print Working Directory.
 */
void 
changeWorkingDir(char* path){
   
   /*Change directory to given path and set environment "PWD" variable to match*/

   if(path==NULL){
     char* home = getenv("HOME");
     chdir(home);
   }
   else
     chdir(path);
	
  //if(chdir(path)!=0)
  //   printf("Error: %s\n", strerror(errno));

   char *cwd = getCurrentWorkingDir();
   setenv("PWD", cwd, 1);
   free(cwd);
 
      
}/*changeWorkingDir*/

/*
 * getPath
 *
 * arguments: none
 *
 * returns: Path  
 *
 * Search directories according to PATH.
 */
char** 
getPath(){
   
   /*Get copy of pointer to "PATH" string.*/
   char* mPath;
   mPath=strdup(getenv("PATH"));
   
   /*Make pointer pointing to the string pointer*/
   char* ptr = mPath;
   
   /*Create a char* [] dynamically*/
   char** subPath = (char**)malloc(sizeof(char*) * PATHSIZE); 
   
   /*Index for token loop*/
   int i =0;
  
   /****************strsep method*********************/
   //char* tempx = temp;

   //while(subPath[i] =strsep(&mainPath,":")){
      
      //subPath[i] = (char*)malloc(sizeof(char)*100);
      //strcpy(subPath[i],temp2);
      //printf("%s\n",subPath[i]); 
     // i++; 
   //}

   /**************strtok method**********************/

   subPath[i] = strtok_r(mPath,":",&mPath);

   i=1;
   while((subPath[i]=strtok_r(NULL,":",&mPath)) != NULL){
    
       i++;
   }
 
   /*Free ptr to string and return subPath. To be freed later*/
   free(ptr);
   return subPath;

}

void 
freePath(char** path){
  
  free(path);

}



