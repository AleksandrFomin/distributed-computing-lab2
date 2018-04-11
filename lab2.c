#include <stdlib.h>
#include <stdio.h>

#include "banking.h"
#include "lab1.h"
#include "ipc.h"

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
    	send(parent_data, dst, msg);
		send(parent_data, src, msg);
		receive(parent_data, dst, msg);
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
	BalanceState balance_state;

	msg = (Message*)malloc(sizeof(Message));
	trOrd = (TransferOrder*)malloc(sizeof(TransferOrder));
	
	sp = prepare_source_proc(matrix, proc_id, N, fd);

	while(1){
		receive(sp, PARENT_ID, msg);
		trOrd = (TransferOrder*)(msg->s_payload);
		from = trOrd->s_src;
		to = trOrd->s_dst;
		amount = trOrd->s_amount;
		if(msg->s_header.s_type == TRANSFER){
			if(proc_id == from){
				(*balance) -= amount;

				balance_state.s_balance = *balance;
				balance_state.s_time = get_physical_time();
				balance_state.s_balance_pending_in = 0;
				balance_history->s_history[balance_state.s_time] = balance_state;

				transfer(sp, from, to, amount);
			}

			if(proc_id == to){
				receive(sp, from, msg);
				amount = ((TransferOrder*)(msg->s_payload))->s_amount;
				(*balance) += amount;

				balance_state.s_balance = *balance;
				balance_state.s_time = get_physical_time();
				balance_state.s_balance_pending_in = 0;
				balance_history->s_history[balance_state.s_time] = balance_state;

				header = prepare_message_header(4, ACK);
	    		msg = prepare_message(header, "NULL");
				send(sp, PARENT_ID, msg);
			}
		}
		if(msg->s_header.s_type == STOP){
			break;
		}
	}
	balance_history->s_history_len=balance_state.s_time ;
	return 0;
}

/*int main(int argc, char * argv[])
{
    //bank_robbery(parent_data);
    //print_history(all);

    return 0;
}*/
