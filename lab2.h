#ifndef __LAB2_H
#define __LAB2_H

#include "ipc.h"
#include "banking.h"

int second_phase(int*** matrix, int proc_id, int N,
	int fd, balance_t* balance, BalanceHistory* balance_history);

#endif
