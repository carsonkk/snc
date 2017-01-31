#include "server.h"

//Handle TCP/UDP server interface
int server_handler(int udp, char *hostname, int port)
{
	end_conn = 0;
	int status = 0;
	int sockopt = 1;
	int thread_err = 0;
	int send_ret;
	int recv_ret;
	int server_sock_fd;
	int client_sock_fd;
	char rslvd_hostname[256];
	struct sockaddr_in *host_addr;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t server_addr_len;
	socklen_t client_addr_len;
	struct addrinfo *info;
	struct addrinfo info_list;
	struct addrinfo *ai;
	pthread_t send_thread_id;
	pthread_t recv_thread_id;
	struct arg_struct *server_args;
	struct ret_struct *server_rets;

	//open a server socket
	if(udp == 1)
		status = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
		status = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(status < 0)
	{
		fprintf(stdout, "%sserver failed to open a socket (status: %d)\n", err_msg, status);
		return 1;
	}
	server_sock_fd = status;

	//check if the system is still holding onto the port after a recent restart
	status = setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&sockopt , sizeof(int));
	if(status < 0)
	{
		fprintf(stdout, "%sserver can't bind to port due to the system still holding the port resources from a recent restart (status: %d)\n", err_msg, status);
		return 1;
	}

	//setup the server socket
	server_addr_len = sizeof(server_addr);
	memset((char *)&server_addr, 0, server_addr_len);
	server_addr.sin_family = AF_INET;
	if(hostname != NULL)
	{
		server_addr.sin_addr.s_addr = inet_addr(hostname);
		if(server_addr.sin_addr.s_addr == (in_addr_t)-1)
		{
			//setup infoaddr to resolve hostname
			memset(&info_list, 0, sizeof(info_list));
			info_list.ai_family = AF_INET;
			if(udp == 1)
				info_list.ai_socktype = SOCK_DGRAM;
			else
				info_list.ai_socktype = SOCK_STREAM;

			//get the reolved hostname from the address info
			status = getaddrinfo(hostname, "http", &info_list, &info);
			if(status < 0)
			{
				fprintf(stdout, "%sserver failed to get the address info for the hostname (status: %d)\n", err_msg, status);
				return 1;
			}
			for(ai = info; ai != NULL; ai = ai->ai_next)
			{
				host_addr = (struct sockaddr_in *) ai->ai_addr;
				strcpy(rslvd_hostname, inet_ntoa(host_addr->sin_addr));
			}
			freeaddrinfo(info);
			server_addr.sin_addr.s_addr = inet_addr(rslvd_hostname);
		}
	}
	else
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	//bind to a port on the server
	status = bind(server_sock_fd, (struct sockaddr *)&server_addr, server_addr_len);
	if(status < 0)
	{
		fprintf(stdout, "%sserver failed to bind to the specified port (status: %d)\n", err_msg, status);
		return 1;
	}

	client_addr_len = sizeof(client_addr);
	if(udp == 0)
	{
		//listen to the port
		status = listen(server_sock_fd, 16);
		if(status < 0)
		{
			fprintf(stdout, "%sserver failed to listen for clients (status: %d)\n", err_msg, status);
			return 1;
		}

		//accept connection
		status = accept(server_sock_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if(status < 0)
		{
			fprintf(stdout, "%sserver failed to accept a connection from a client (status: %d)\n", err_msg, status);
			return 1;
		}
		client_sock_fd = status;
	}

	//spawn receive thread
	server_args = (struct arg_struct *)malloc(sizeof(struct arg_struct));
	if(udp == 1)
		server_args->sock_fd = server_sock_fd;
	else
		server_args->sock_fd = client_sock_fd;
	server_args->udp = udp;
	server_args->udp_addr = &client_addr;
	server_args->udp_addr_len = &client_addr_len;
	fflush(stdout);
	thread_err = pthread_create(&recv_thread_id, NULL, recv_thread, (void *)server_args);
	if(thread_err != 0)
	{
		fprintf(stdout, "%sserver failed to spawn receive thread (status: %d)\n", err_msg, thread_err);
		return 1;
	}

	//spawn send thread
	server_args = (struct arg_struct *)malloc(sizeof(struct arg_struct));
	if(udp == 1)
		server_args->sock_fd = server_sock_fd;
	else
		server_args->sock_fd = client_sock_fd;
	server_args->udp = udp;
	server_args->udp_addr = &client_addr;
	server_args->udp_addr_len = &client_addr_len;
	fflush(stdout);
	thread_err = pthread_create(&send_thread_id, NULL, send_thread, (void *)server_args);
	if(thread_err != 0)
	{
		fprintf(stdout, "%sserver failed to spawn send thread (status: %d)\n", err_msg, thread_err);
		return 1;
	}

	//join receive thread
	fflush(stdout);
	thread_err = pthread_join(recv_thread_id, (void **)&server_rets);
	if(thread_err != 0)
	{
		fprintf(stdout, "%sserver failed to join receive thread (status: %d)\n", err_msg, thread_err);
		return 1;
	}
	recv_ret = server_rets->ret;
	free(server_rets);

	if(end_conn == 1)
	{
		fflush(stdout);
		pthread_cancel(send_thread_id);
	}
	else
	{
		//join send thread
		fflush(stdout);
		thread_err = pthread_join(send_thread_id, (void **)&server_rets);
		if(thread_err != 0)
		{
			fprintf(stdout, "%sserver failed to join send thread (status: %d)\n", err_msg, thread_err);
			return 1;
		}
		send_ret = server_rets->ret;
		free(server_rets);
	}				

	//check for issues while sending/receiving messages
	if(send_ret == 1 || recv_ret == 1)
		return 1;

	//close the socket connection
	if(udp == 1)
		status = close(server_sock_fd);
	else
		status = close(client_sock_fd);
	if(status < 0)
	{
		fprintf(stdout, "%sserver failed to cleanly close the socket (status: %d)\n", err_msg, status);
		return 1;
	}

	return 0;
}