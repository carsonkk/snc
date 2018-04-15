#include "threads.h"

//Read from stdin/send input data
void *send_thread(void *args)
{
	int send_buf_max = 4096;
	int errno_count = 0;
	int status;
	int sock_fd;
	int udp;
	char send_buf[send_buf_max];
	struct sockaddr_in *udp_addr;
	socklen_t *udp_addr_len;
	struct arg_struct *thread_args = (struct arg_struct *)args;
	struct ret_struct *thread_rets = (struct ret_struct *)malloc(sizeof(struct ret_struct));

	fflush(stdout);
	sock_fd = thread_args->sock_fd;
	udp = thread_args->udp;
	udp_addr = thread_args->udp_addr;
	udp_addr_len = thread_args->udp_addr_len;
	free(thread_args);

	while(1)
	{
		//check if ctrl^c or ctrl^d was recieved in recv_thread()
		if(end_conn == 1 && udp == 0)
		{
			thread_rets->ret = 0;
			break;
		}

		//flush stdout and reset the send_buf memory
		fflush(stdout);
		memset(send_buf, 0, send_buf_max);

		//if crtl^c or crtl^d was received from stdin, send and exit
		if(!fgets(send_buf, send_buf_max, stdin))
		{
			thread_rets->ret = 0;
			end_conn = 1;
			if(udp == 0)
			{
				send(sock_fd, send_buf, send_buf_max, 0);
			}
			break;
		}

		//send the input from stdin
		if(udp == 1)
		{
			status = sendto(sock_fd, send_buf, send_buf_max, 0, (struct sockaddr *)udp_addr, *udp_addr_len);
		}
		else
		{
			status = send(sock_fd, send_buf, send_buf_max, 0);
		}

		//handle any errors during transmission
		if(status != send_buf_max)
		{
			if(udp == 1 && errno == 97)
			{
				errno_count++;
			}
			else
			{
				fprintf(stdout, "%sencoutnered an issue while trying to send the contents of stdin (status: %d)\n", err_msg, status);
				end_conn = 1;
				thread_rets->ret = 1;
				break;
			}
		}
	}

	fflush(stdout);
	return ((void *)thread_rets);
}

//Read from the socket/print output data
void *recv_thread(void *args)
{
	int recv_buf_max = 4096;
	int init_recv = 0;
	int status;
	int status_conn;
	int sock_fd;
	int udp;
	char recv_buf[recv_buf_max];
	char recv_buf_ref[recv_buf_max];
	struct sockaddr_in *udp_addr;
	socklen_t *udp_addr_len;
	struct arg_struct *thread_args = (struct arg_struct *)args;
	struct ret_struct *thread_rets = (struct ret_struct *)malloc(sizeof(struct ret_struct));

	fflush(stdout);
	sock_fd = thread_args->sock_fd;
	udp = thread_args->udp;
	udp_addr = thread_args->udp_addr;
	udp_addr_len = thread_args->udp_addr_len;
	free(thread_args);

	while(1)
	{
		//check if ctrl^c or ctrl^d was sent in send_thread()
		if(end_conn == 1 && udp == 0)
		{
			thread_rets->ret = 0;
			break;
		}

		//flush stdout and reset the recv_buf memory
		fflush(stdout);
		memset(recv_buf, 0, recv_buf_max);

		//receive the input from the socket connection to the client, checking for an error
		if(udp == 1)
		{
			status = recvfrom(sock_fd, recv_buf, recv_buf_max, 0, (struct sockaddr *)udp_addr, udp_addr_len);
			if(init_recv == 0)
			{
				status_conn = connect(sock_fd, (struct sockaddr*)udp_addr, *udp_addr_len);
				if(status_conn < 0)
				{
					fprintf(stdout, "%srecv thread failed to connect back to sender (status: %d)\n", err_msg, status_conn);
					end_conn = 1;
					thread_rets->ret = 1;
					break;
				}
			}
			init_recv = 1;
		}
		else
		{
			status = recv(sock_fd, recv_buf, recv_buf_max, 0);
		}

		//crtl^c or crtl^d was received
		if(status == 0 || strcmp(recv_buf, recv_buf_ref) == 0)
		{
			thread_rets->ret = 0;
			end_conn = 1;
			break;
		}
		//an error in transmission occurred
		if(status != recv_buf_max)
		{
			fprintf(stdout, "%sencountered an issue while reading from the socket (status: %d)\n", err_msg, status);
			end_conn = 1;
			thread_rets->ret = 1;
			break;
		}

		//print the received input to stdout
		fprintf(stdout, "%s", recv_buf);
	}

	fflush(stdout);
	return((void *)thread_rets);
}