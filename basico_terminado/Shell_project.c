/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell
	(then type ^D to exit program)

**/

#include "job_control.h"   // remember to compile with module job_control.c
#include <string.h>
#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN
// -----------------------------------------------------------------------
job *l=NULL; // lista de jobs
int idx=0;

void handler_SIGCHLD(int sig){
	job *aux=l;

	int status=0;
	int sign;
	enum status stat;
	int info;


	while(aux!=NULL){

		sign=waitpid(aux->pgid,&status,WNOHANG|WUNTRACED|WCONTINUED);

		if(sign==aux->pgid){
			if(WIFCONTINUED(status)){
				aux->state=BACKGROUND;
				//aux=aux->next;
			}else{
				stat=analyze_status(status,&info);



									if(strcmp(status_strings[stat],"Suspended")==0 && aux->state==BACKGROUND && sign==aux->pgid){
										//printf("\nEstoy parando a %d\n",aux->pgid);
										aux->state=STOPPED;


									}else if(strcmp(status_strings[stat],"Exited")==0){
										delete_job(l,aux);
										idx--;
									}else if(strcmp(status_strings[stat],"Signaled")==0 ){
										printf("\nESTOY MATANDO A %d\n",aux->pgid);
										delete_job(l,aux);
										idx--;
									}


			}

		}
		aux=aux->next;
	}
}



void bg(char** args, job*l, int index){
						job *aux;
						int bg;

						if(args[1]==NULL){
							// aux=l;
							// while(aux->next!=NULL){
							// 	aux=aux->next;
							//
							// }
							printf("Error, tiene que escoger un numero valido de proceso\n");
						}else{
							bg=atoi(args[1]);
							if(bg>index || bg <0 ){
								printf("\nError, escoge un numero valido\n");
							}


							aux=l;
							while(bg>0){
								aux=aux->next;
								bg--;
							}

						printf("\nrunning... %s\n",aux->command);

						restore_terminal_signals();


							//set_terminal(aux->pgid);
							if(aux->state!=BACKGROUND){
								killpg(aux->pgid,SIGCONT);
							}

							//signal(SIGCHLD,handler_SIGCHLD);
							ignore_terminal_signals();

						}

}

void fg(char** args, job*l, int index,int pid_padre){
	job *aux;
	int fg;
	int status;
	int stat;
	int info;
	int sign;
	if(args[1]==NULL){
		aux=l;
		while(aux->next!=NULL){
			aux=aux->next;

		}
		//printf("Error, tiene que escoger un numero valido de proceso\n");
	}else{
		fg=atoi(args[1]);
		if(fg>index || fg <0 ){
			printf("\nError, escoge un numero valido\n");
		}


		aux=l;
		while(fg>0){
			aux=aux->next;
			fg--;
		}

	}


		set_terminal(aux->pgid);
		//set_terminal(aux->pgid);

		killpg(aux->pgid,SIGCONT);


		waitpid(aux->pgid,&status,WUNTRACED);

		set_terminal(pid_padre);

		//signal(SIGCHLD,handler_SIGCHLD);
		stat=analyze_status(status,&info);



							if(strcmp(status_strings[stat],"Suspended")==0 ){
								//printf("\nEstoy parando a %d\n",aux->pgid);
								aux->state=STOPPED;


							}else if(strcmp(status_strings[stat],"Exited")==0){
								delete_job(l,aux);
								idx--;
							}else if(strcmp(status_strings[stat],"Signaled")==0 ){
								printf("\nESTOY MATANDO A %d\n",aux->pgid);
								delete_job(l,aux);
								idx--;
							}






}

//--------------------------------------------------------------
int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */
	int grup; //Grupo del proceso.


	int nele=0;


	char path[1000];
	inicialista(&l);
	//signal(SIGCHLD,handler_SIGCHLD0);
	ignore_terminal_signals();
	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{

		printf("%s COMMAND->",getcwd(path,sizeof(path)));
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

		if(args[0]==NULL){
			continue;
		}   // if empty command

		//SI ESCRIBIMOS EXIT SALIMOS
		if(strcmp(args[0],"^d")==0 || strcmp(args[0], "exit")==0 ){
				printf("Bye\n");
				return 0;
		}

				//Si es fg
				if(strcmp(args[0],"fg")==0){
						fg(args,l,idx,getpid());

						//Si es bg
			}else if(strcmp(args[0],"bg")==0){
					bg(args,l,idx);

				//Si es cd
			}else if(strcmp(args[0],"cd")==0){

			if(args[1]!=NULL){
					chdir(args[1]);
				}
				//Si es jobs
		}else if(strcmp(args[0],"jobs")==0){

			print_job_list(l);

			//si no es comando interno
		}else{

		pid_fork=fork();

		if(pid_fork==0){ //SI es el hijo
				//Activamos las señales del terminal
				new_process_group(getpid());
				restore_terminal_signals();
				//Le damos el terminal

				if(background==0){

					set_terminal(getpid());
				}
				execvp(*args,args);

				printf("Error: command not found: %s \n",inputBuffer);
				exit(-1);


		}else{ //si el padre


			if(background==0){
				set_terminal(pid_fork);

				waitpid(pid_fork,&status,WUNTRACED);

				set_terminal(getpid());

				//ignore_terminal_signals();
				status_res=analyze_status(status,&info);



				if(strcmp(status_strings[status_res],"Suspended")==0){
					add_job(&l,new_job(pid_fork,args[0],STOPPED));
					idx++;
				}
			}




			if(background==0){
				printf("\nForeground ");
				printf("pid: %d, command: %s, %s, info: %d",pid_fork,inputBuffer,status_strings[status_res],info);
			}else{
				printf("\nBackground job running... pid: %d, command: %s",pid_fork,inputBuffer);


				add_job(&l,new_job(pid_fork,args[0],BACKGROUND));
				idx++;



			}

		}

			block_SIGCHLD();

			signal(SIGCHLD,handler_SIGCHLD);
			unblock_SIGCHLD();
			printf("\n\n");

		}



		/* the steps are:
			 (1) fork a child process using fork()
			 (2) the child process will invoke execvp()
			 (3) if background == 0, the parent will wait, otherwise continue
			 (4) Shell shows a status message for processed command
			 (5) loop returns to get_commnad() function
		*/

	} // end while
}
