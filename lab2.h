#ifndef __LAB2_H
#define __LAB2_H

#include "ipc.h"
#include "banking.h"

int second_phase(int*** matrix, int proc_id, int N,
	int fd, balance_t* balance, BalanceHistory* balance_history);

void complete_history(BalanceHistory* balanceHistory);

int send_history(int*** matrix, local_id proc_id, int N, int log_fd, BalanceHistory* balanceHistory);

AllHistory* receive_all_history(int*** matrix, local_id proc_id, int N, int log_fd);

#endif
