#ifndef __LAB1_H
#define __LAB1_H

#include "ipc.h"
#include "banking.h"

typedef struct{
	int*** matrix;
	local_id proc_id;
	int N;
	int fd;
} SourceProc;

typedef struct{
	int N;
	int values[10];
} Options;

Options* get_key_value(int argc, char* argv[]);

int*** create_matrix(int N);

int close_pipes(int*** matrix, int N, int num);

int log_pipes(int*** matrix, int N);

int log_events(int fd, char* str);

int receive_all(void* self, MessageType type);

int get_message(int*** matrix, local_id proc_id, int N, 
		int fd, MessageType type);

SourceProc* prepare_source_proc(int*** matrix, int proc_id,
	int N, int fd);

MessageHeader prepare_message_header(uint16_t len, int16_t type);

Message* prepare_message(MessageHeader msg_header, char msg_text[MAX_PAYLOAD_LEN]);

int send_message(int*** matrix, local_id proc_id, int N, 
		int fd, MessageType type, balance_t balance);

int first_phase(int*** matrix, int proc_id, int N,
	int fd, balance_t balance);

int third_phase(int*** matrix, int proc_id, int N,
	int fd, balance_t balance);

Message* create_message(MessageType msg_type, char* buf, int length);

#endif
