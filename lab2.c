#include <stdlib.h>
#include <stdio.h>

#include "banking.h"
#include "lab1.h"
#include "ipc.h"
#include "lab2.h"

void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount){
    MessageHeader header;
    TransferOrder* trOrd;
    Message* msg;

    msg = (Message*)malloc(sizeof(Message));
    trOrd = (TransferOrder*)malloc(sizeof(TransferOrder));

    trOrd->s_src = src;
    trOrd->s_dst = dst;
    trOrd->s_amount = amount;

    header = prepare_message_header(sizeof(trOrd), TRANSFER);
    msg = prepare_message(header, (char*)trOrd);

    if(((SourceProc*)parent_data)->proc_id != 0){
    	((SourceProc*)parent_data)->proc_id = src;
    	send(parent_data, dst, msg);
	}
	if(((SourceProc*)parent_data)->proc_id == 0){
    	// send(parent_data, dst, msg);
		send(parent_data, src, msg);
		while(receive(parent_data, dst, msg)==-1);
	}
}

int second_phase(int*** matrix, int proc_id, int N,
	int fd, balance_t* balance, BalanceHistory* balance_history){

	SourceProc* sp;
	MessageHeader header;
	Message* msg;
	local_id from;
	local_id to;
	balance_t amount;
	TransferOrder* trOrd;
	timestamp_t time;
	int i;
	BalanceState* balance_state = (BalanceState*)malloc(sizeof(BalanceState));

	msg = (Message*)malloc(sizeof(Message));
	trOrd = (TransferOrder*)malloc(sizeof(TransferOrder));
	
	sp = prepare_source_proc(matrix, proc_id, N, fd);

	while(1){
		i=0;
		while(1){
			if(i==proc_id){
				i++;
				if(i>N){
					i=0;
				}
				continue;
			}
			if(receive(sp, i, msg)!=-1){
				break;
			}
			i++;
			if(i>N){
				i=0;
			}
		}
		trOrd = (TransferOrder*)(msg->s_payload);
		from = trOrd->s_src;
		to = trOrd->s_dst;
		amount = trOrd->s_amount;
		if(msg->s_header.s_type == TRANSFER){
			if(proc_id == from){
				(*balance) -= amount;
				balance_state->s_balance = *balance;
				time = get_physical_time();
				balance_state->s_time = time;
				balance_state->s_balance_pending_in = 0;
				balance_history->s_history[time] = *balance_state;

				transfer(sp, from, to, amount);
			}

			if(proc_id == to){
				// receive(sp, from, msg);
				amount = ((TransferOrder*)(msg->s_payload))->s_amount;
				(*balance) += amount;

				balance_state->s_balance = *balance;
				time = get_physical_time();
				balance_state->s_time = time;
				balance_state->s_balance_pending_in = 0;
				balance_history->s_history[time] = *balance_state;

				header = prepare_message_header(4, ACK);
	    		msg = prepare_message(header, "NULL");
				send(sp, PARENT_ID, msg);
			}
		}
		if(msg->s_header.s_type == STOP){
			break;
		}
	}
	balance_history->s_id = proc_id;
	balance_history->s_history_len=time+1;
	// complete_history(balance_history);
	return 0;
}

void complete_history(BalanceHistory* balanceHistory){
	int i=0;
	// BalanceState* prev;
	balance_t prev_balance;

	for(i = 1; i <= balanceHistory->s_history_len; i++){
		prev_balance = balanceHistory->s_history[i-1].s_balance;
		if(balanceHistory->s_history[i].s_balance == 0){
			BalanceState* bs=(BalanceState*)malloc(sizeof(BalanceState));
			bs->s_balance=prev_balance;
			bs->s_balance_pending_in=0;
			bs->s_time=i;
			balanceHistory->s_history[i] = *bs;
		}
	}

}

int send_history(int*** matrix, local_id proc_id, int N, int log_fd, BalanceHistory* balance_history){
	SourceProc* sp = prepare_source_proc(matrix, proc_id, N, log_fd);
	Message* msg;

	msg = create_message(BALANCE_HISTORY, 
		(char*)balance_history, 
		(balance_history->s_history_len)*sizeof(BalanceState)+
		sizeof(uint8_t)+sizeof(local_id));
	send(sp, PARENT_ID, msg);
	return 0;
}

AllHistory* receive_all_history(int*** matrix, local_id proc_id, int N, int log_fd){
	int i;
	Message* msg=(Message*)malloc(sizeof(Message));
	AllHistory* allHistory = (AllHistory*)malloc(sizeof(AllHistory));
	allHistory->s_history_len = N;

	for(i = 1; i <= N; i++){
		while(receive(prepare_source_proc(matrix, PARENT_ID, N, log_fd), i, msg)==-1);
		allHistory->s_history[i-1] = *((BalanceHistory*)(msg->s_payload));
		if(allHistory->s_history[i-1].s_history_len < get_physical_time()+1){
			allHistory->s_history[i-1].s_history_len = get_physical_time()+1;
			allHistory->s_history[i-1].s_history[get_physical_time()].s_balance = 0;
		}
		complete_history(&(allHistory->s_history[i-1]));
	}
	print_history(allHistory);
	return allHistory;
}
