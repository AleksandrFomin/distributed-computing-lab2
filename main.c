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
	balance_t balance = 0;
	// MessageHeader header;
	// Message* msg = (Message*)malloc(sizeof(Message));
	SourceProc* sp;
	AllHistory* all_history;
	timestamp_t time;
	BalanceState* balanceState = (BalanceState*)malloc(sizeof(BalanceState));

	BalanceHistory* balance_history = (BalanceHistory*)malloc(
									sizeof(BalanceHistory));
	Options* opts = (Options*)malloc(sizeof(Options));

	opts = get_key_value(argc, argv);
	N = opts->N;
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

				balanceState -> s_balance = balance;
				// printf("%d\n",balanceState -> s_balance );
				time = get_physical_time();
				// if(time%2==0){time/=2;}
				// printf("%d\n", time);
				balanceState -> s_time = time;
				balance_history->s_id = proc_id;
				balance_history->s_history[time] = (*balanceState);

				close_pipes(fds, N, proc_id);

				first_phase(fds, proc_id, N, log_fd, balance);
				second_phase(fds, proc_id, N, log_fd,
							 &balance, balance_history);
				third_phase(fds, proc_id, N, log_fd, balance);

				send_history(fds, proc_id, N, log_fd, balance_history);

				exit(0);
				break;
			default:
				break;
		}
	}

	close_pipes(fds, N, PARENT_ID);

	get_message(fds, PARENT_ID, N, log_fd, STARTED);

	sp = prepare_source_proc(fds, PARENT_ID, N, log_fd);
	bank_robbery(sp, N);
	send_message(fds, PARENT_ID, N, log_fd, STOP, 0);

	get_message(fds, PARENT_ID, N, log_fd, DONE);

	all_history = receive_all_history(fds, PARENT_ID, N, log_fd);

	

	for(i = 0; i < N; i++){
		wait(NULL);
	}

	return 0;
}
