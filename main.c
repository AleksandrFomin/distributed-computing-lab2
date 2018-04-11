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
	MessageHeader header;
	Message* msg = (Message*)malloc(sizeof(Message));
	SourceProc* sp;
	AllHistory* all_history;

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
				close_pipes(fds, N, proc_id);

				first_phase(fds, proc_id, N, log_fd, balance);
				second_phase(fds, proc_id, N, log_fd,
							 &balance, balance_history);
				third_phase(fds, proc_id, N, log_fd, balance);

				sp = prepare_source_proc(fds, proc_id, N, log_fd);
				header = prepare_message_header(balance_history->s_history_len,
												BALANCE_HISTORY);
				balance_history->s_id = proc_id;
	    		msg = prepare_message(header, (char*)balance_history);
				send(sp, PARENT_ID, msg);

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

	all_history = (AllHistory*)malloc(sizeof(AllHistory));

	for(i = 0; i < N; i++){
		receive(sp, i + 1, msg);
		printf("%d\n", ((BalanceHistory*)(msg->s_payload))->s_history_len);
		all_history->s_history[i] = *((BalanceHistory*)(msg->s_payload));
	}
	all_history->s_history_len = N;

	for(i = 0; i < N; i++){
		wait(NULL);
	}

	print_history(all_history);

	return 0;
}
