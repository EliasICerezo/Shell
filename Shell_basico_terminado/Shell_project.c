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
int idx=0;	//numero de elementos de la lista de jobs
struct termios conf;
struct termios new_conf;

void handler_SIGCHLD(int sig){
	job *aux=l;

	int status=0;
	int sign;
	enum status stat;
	int info;
	char path[1000];

	/*Recorro la estructura de la lista para ir procesandola en funcion de las señales que hayan ido recibiendo*/
	while(aux!=NULL){

		sign=waitpid(aux->pgid,&status,WNOHANG|WUNTRACED|WCONTINUED);
		//si continua
		if(sign==aux->pgid){
			if(WIFCONTINUED(status)){
				aux->state=BACKGROUND;

			}else{
				stat=analyze_status(status,&info);

									//printf("\n");
									//Si el estado es suspendido
									if(strcmp(status_strings[stat],"Suspended")==0 && aux->state==BACKGROUND && sign==aux->pgid){
										printf("He parado al proceso %d, %s\n",aux->pgid, aux->command);
										printf("%s COMMAND->",getcwd(path,sizeof(path)));
										fflush(stdout);

										aux->state=STOPPED;

									//SI ha terminado
									}else if(strcmp(status_strings[stat],"Exited")==0){
										printf("\nEl proceso %d, %s ha acabado\n",aux->pgid, aux->command);
										delete_job(l,aux);
										idx--;
										//Si lo hemos matado con una señal
									}else if(strcmp(status_strings[stat],"Signaled")==0 ){
										printf("\nHe matado con una señal al proceso %d, %s\n",aux->pgid, aux->command);
										delete_job(l,aux);
										idx--;
									}


			}

		}
		aux=aux->next;
	}
}


//BACKGROUND
void bg(char** args, job*l, int index){
						job *aux;
						int bg; //numero de proceso

						//si no le paso ningun numero de proceso
						if(args[1]==NULL){
							aux=l;
							while(aux->next!=NULL){
							aux=aux->next;

							}
						}else{
							bg=atoi(args[1]);
							if(bg>index || bg <=0 ){
								printf("\nError, escoge un numero valido\n");
								return;
							}else{
								


							aux=l;
							while(bg>0){
								aux=aux->next;
								bg--;
							}
						
							
							}
						}
						printf("\nrunning... %s\n",aux->command);
						
						restore_terminal_signals();


							//si ya esta en bg no le mandamos la señal sigcont
							if(aux->state!=BACKGROUND){
								killpg(aux->pgid,SIGCONT);
							}
							aux->state=BACKGROUND;
						

							ignore_terminal_signals();

						
//STDIN_FILEN0
}

void fg(char** args, job*l, int index,int pid_padre){
	job *aux; //job para recorrer la lista
	int fg; //numero de proceso
	int status; //status del wait
	enum status stat; // return de analyze_status
	int info; //info del analyze_status
	int sign; //return del waitpid
	//si no me dan ningun numero me voy al ultimo proceso qhe añadido en la lista.
	if(args[1]==NULL){
		aux=l;
		while(aux->next!=NULL){
			aux=aux->next;

		}

	}else{
		//comprueb que el numero sea valido
		fg=atoi(args[1]);
		if(fg>index || fg <=0 ){
			printf("\nError, escoge un numero valido\n");
			return;
		}else{


			aux=l;
			//busco el proceso
			while(fg>0){
				aux=aux->next;
				fg--;
			}
		}

	}
//HACEMOS LA MISMA BUSQUEDA QUE EN BG PERO SI NO NOS DAN NUMERO NOS VMAOS AL ULTIMO QUE HEMOS AÑADIDO

	//le damos el terminal
		printf("Hago fg al comando: %s\n",aux->command);
		tcgetattr(STDIN_FILENO,&conf);
		set_terminal(aux->pgid);
		tcsetattr(STDIN_FILENO,TCSADRAIN,&(aux->atrib));
		//tcgetattr(STDIN_FILENO,&aux->atrib);
		
//si no esta en bg le mandamos sigcont
		if(aux->state!=BACKGROUND){
			killpg(aux->pgid,SIGCONT);
		}

//lo esperamos y luego le damos el terminal al padre
		waitpid(aux->pgid,&status,WUNTRACED);
		tcgetattr(STDIN_FILENO,&new_conf);
		aux->atrib = new_conf;
		
		set_terminal(pid_padre);
		tcsetattr(STDIN_FILENO,TCSANOW,&conf);
		//analizamos el status en el que ha terminado
		stat=analyze_status(status,&info);



							if(strcmp(status_strings[stat],"Suspended")==0 ){
								printf("He suspendido al proceso %d, %s",aux->pgid, aux->command);
								aux->state=STOPPED;


							}else if(strcmp(status_strings[stat],"Exited")==0){
								printf("El proceso %d, %s ha acabado\n", aux->pgid, aux->command);
								delete_job(l,aux);
								idx--;
							}else if(strcmp(status_strings[stat],"Signaled")==0 ){
								printf("\nHe matado a %d,%s con una señal\n",aux->pgid,aux->command);
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
	int grup; //Grupo del proceso
	//job auxiliar para usar los atributos del terminal

	char path[1000]; //cadena para el path
	inicialista(&l); //funcion que mete un nodo en la lista llamado joblist y con su siguiente apuntando a null
	//signal(SIGCHLD,handler_SIGCHLD0);
	ignore_terminal_signals(); //desde aqui ignoramos las señales del term
	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{

		printf("\n");
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
			if(empty_list(l)){
				printf("Lista Vacia\n");
				
			}
			else{
				print_job_list(l);
			}

			//si no es comando interno
		}else{

		pid_fork=fork();

		if(pid_fork==0){ //SI es el hijo
				//Activamos las señales del terminal
				new_process_group(getpid()); //le damos un nuevo grupo al proceso hijo
				restore_terminal_signals();

				//si es foreground, le damos el termina
				if(background==0){

					set_terminal(getpid());
					
				}
				//ejecutamos el comando que se nos pasa
				execvp(*args,args);

				printf("Error: command not found: %s \n",inputBuffer);
				exit(-1);


		}else{ //si el padre

			//si es foreground, le damos el terminal al hijo y lo esperamos
			if(background==0){
				tcgetattr(STDIN_FILENO,&conf);
				set_terminal(pid_fork);
				//esperamos hasta que acaba o le mandamos una señal
				
				waitpid(pid_fork,&status,WUNTRACED);
				//devolvemos los atributos del terminal del padre
				tcgetattr(STDIN_FILENO,&new_conf);
				
				//devolvemos el terminal al padre
				set_terminal(getpid());
			    tcsetattr(STDIN_FILENO,TCSANOW,&conf);

				//analizamos como ha acabado, y si lo suspendemos lo metemos en la lista
				status_res=analyze_status(status,&info);



				if(strcmp(status_strings[status_res],"Suspended")==0){
					job * t0 = new_job(pid_fork,args[0],STOPPED);
					t0->atrib = new_conf;
					add_job(&l,t0);
					idx++;
				}
			}



			//imprimimos la linea como se nos pide en la tarea 1
			if(background==0){
				printf("\nForeground ");
				printf("pid: %d, command: %s, %s, info: %d",pid_fork,inputBuffer,status_strings[status_res],info);
			}else{
				printf("\nBackground job running... pid: %d, command: %s",pid_fork,inputBuffer);

				//si eol proceso esta en BACKGROUND lo metemos en la lista tambien
				tcgetattr(STDIN_FILENO,&conf);
				job * t1 = new_job(pid_fork,args[0],BACKGROUND);
				t1->atrib = conf;
				add_job(&l,t1);
				idx++;



			}

		}
		//llamamos al handler
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
