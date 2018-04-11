#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>

#include "common.h"
#include "ipc.h"
#include "pa1.h"
#include "lab1.h"
#include "lab2.h"
#include "banking.h"

//export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:~/Документы/ИТМО/3курс(весна)/Распределенные вычисления/pa2";
//LD_PRELOAD="~/Документы/ИТМО/3курс(весна)/Распределенные вычисления/pa2/lib64"


int main(int argc, char* argv[])
{
	pid_t pid;
	int i, N, log_fd;
	int*** fds;
	local_id proc_id;
	balance_t balance;
	SourceProc* sp;
	Options* opts = (Options*)malloc(sizeof(Options));

	opts = get_key_value(argc, argv);
	N = opts->N;
	// for(i=0;i<N;i++){
	// 	printf("%d\n", opts->values[i]);
	// }
	fds = create_matrix(N);

	if(log_pipes(fds, N) == -1){
		printf("Error writing to log file");
	}

	if((log_fd = open(events_log, O_WRONLY|O_CREAT|O_TRUNC)) == -1){
		return -1;
	}
	
	for(i = 0; i < N; i++){
		switch(pid = fork()){
			case -1:
				perror("fork");
				break;
			case 0:
				proc_id = i + 1;
				balance = opts->values[i];
				if(close_pipes(fds, N, proc_id) < 0){
					printf("Error closing pipe\n");
				}
				if(first_phase(fds, proc_id, N, log_fd, balance) < 0){
					printf("First phase failed\n");
				}
				if(second_phase(fds, proc_id, N, log_fd, &balance) < 0){
					printf("Second phase failed\n");
				}
				if(third_phase(fds, proc_id, N, log_fd, balance) < 0){
					printf("Third phase failed\n");
				}
				exit(0);
				break;
			default:
				break;
		}
	}

	if(close_pipes(fds, N, PARENT_ID) < 0){
		printf("Error closing pipe");
	}
	if(get_message(fds, PARENT_ID, N, log_fd, STARTED) < 0){
		return -1;
	}

	sp = prepare_source_proc(fds, PARENT_ID, N, log_fd);

	bank_robbery(sp, N);
	send_message(fds, proc_id, N, log_fd, STOP, balance);

	if(get_message(fds, PARENT_ID, N, log_fd, DONE) < 0){
		return -1;
	}

	for(i = 0; i < N; i++){
		wait(NULL);
	}

	return 0;
}
