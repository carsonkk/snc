#include "client.h"

//Handle TCP/UDP client interface
int client_handler(int udp, char *source, char *hostname, int port)
{
	end_conn = 0;
	int status = 0;
	int thread_err = 0;
	int send_ret;
	int recv_ret;
	int client_sock_fd;
	char rslvd_hostname[256];
	char rslvd_source[256];
	struct sockaddr_in *host_addr;
	struct sockaddr_in *local_addr;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t server_addr_len;
	socklen_t client_addr_len;
	struct addrinfo *info;
	struct addrinfo info_list;
	struct addrinfo *ai;
	pthread_t send_thread_id;
	pthread_t recv_thread_id;
	struct arg_struct *client_args;
	struct ret_struct *client_rets;

	//open a client socket
	if(udp == 1)
		status = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
		status = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(status < 0)
	{
		fprintf(stdout, "%sclient failed to open a socket (status: %d)\n", err_msg, status);
		return 1;
	}
	client_sock_fd = status;

	//setup the server socket
	server_addr_len = sizeof(server_addr);
	memset((char *)&server_addr, 0, server_addr_len);
	server_addr.sin_family = AF_INET;
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
			fprintf(stdout, "%sclient failed to get the address info for the hostname (status: %d)\n", err_msg, status);
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
	server_addr.sin_port = htons(port);

	//bind to a port on the client
	if(source != NULL)
	{
		//setup the client socket
		client_addr_len = sizeof(client_addr);
		memset((char *)&client_addr, 0, client_addr_len);
		client_addr.sin_family = AF_INET;
		client_addr.sin_addr.s_addr = inet_addr(source);
		if(client_addr.sin_addr.s_addr == (in_addr_t)-1)
		{
			//setup infoaddr to resolve source
			memset(&info_list, 0, sizeof(info_list));
			info_list.ai_family = AF_INET;
			if(udp == 1)
				info_list.ai_socktype = SOCK_DGRAM;
			else
				info_list.ai_socktype = SOCK_STREAM;

			//get the reolved source from the address info
			status = getaddrinfo(source, "http", &info_list, &info);
			if(status < 0)
			{
				fprintf(stdout, "%sclient failed to get the address info for the source (status: %d)\n", err_msg, status);
				return 1;
			}
			for(ai = info; ai != NULL; ai = ai->ai_next)
			{
				local_addr = (struct sockaddr_in *) ai->ai_addr;
				strcpy(rslvd_source, inet_ntoa(local_addr->sin_addr));
			}
			freeaddrinfo(info);
			client_addr.sin_addr.s_addr = inet_addr(rslvd_source);
		}
		client_addr.sin_port = htons(0);
		status = bind(client_sock_fd, (struct sockaddr *)&client_addr, client_addr_len);
		if(status < 0)
		{
			fprintf(stdout, "%sclient failed to bind to the specified port (status: %d)\n", err_msg, status);
			return 1;
		}
	}

	//connect to the port
	if(udp == 0)
	{
		status = connect(client_sock_fd, (struct sockaddr*)&server_addr, server_addr_len);
		if(status < 0)
		{
			//client tried to connect to a server that wasn't initiated/listening
			return 1;
		}
	}

	//spawn receive thread
	client_args = (struct arg_struct *)malloc(sizeof(struct arg_struct));
	client_args->sock_fd = client_sock_fd;;
	client_args->udp = udp;
	client_args->udp_addr = &server_addr;
	client_args->udp_addr_len = &server_addr_len;
	fflush(stdout);
	thread_err = pthread_create(&recv_thread_id, NULL, recv_thread, (void *)client_args);
	if(thread_err != 0)
	{
		fprintf(stdout, "%sclient failed to spawn receive thread (status: %d)\n", err_msg, thread_err);
		return 1;
	}

	//spawn send thread
	client_args = (struct arg_struct *)malloc(sizeof(struct arg_struct));
	client_args->sock_fd = client_sock_fd;
	client_args->udp = udp;
	client_args->udp_addr = &server_addr;
	client_args->udp_addr_len = &server_addr_len;
	fflush(stdout);
	thread_err = pthread_create(&send_thread_id, NULL, send_thread, (void *)client_args);
	if(thread_err != 0)
	{
		fprintf(stdout, "%sclient failed to spawn send thread (status: %d)\n", err_msg, thread_err);
		return 1;
	}

	//join receive thread
	fflush(stdout);
	thread_err = pthread_join(recv_thread_id, (void **)&client_rets);
	if(thread_err != 0)
	{
		fprintf(stdout, "%sclient failed to join receive thread (status: %d)\n", err_msg, thread_err);
		return 1;
	}
	recv_ret = client_rets->ret;
	free(client_rets);

	if(end_conn == 1)
	{
		fflush(stdout);
		pthread_cancel(send_thread_id);
	}
	else
	{
		//join send thread
		fflush(stdout);
		thread_err = pthread_join(send_thread_id, (void **)&client_rets);
		if(thread_err != 0)
		{
			fprintf(stdout, "%sclient failed to join send thread (status: %d)\n", err_msg, thread_err);
			return 1;
		}
		send_ret = client_rets->ret;
		free(client_rets);
	}

	//check for issues while sending/receiving messages
	if(send_ret == 1 || recv_ret == 1)
		return 1;

	//close the socket connection
	status = close(client_sock_fd);
	if(status < 0)
	{
		fprintf(stdout, "%sclient failed to cleanly close the socket (status: %d)\n", err_msg, status);
		return 1;
	}

	return 0;
}