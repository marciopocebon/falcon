/*
 * client.h
 *
 *  Created on: Sep 21, 2019
 *      Author: lilbase
 */

#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include "../include/spdlog/spdlog.h"
#include "../include/spdlog/sinks/stdout_color_sinks.h"
#include "../include/spdlog/sinks/rotating_file_sink.h"
#include "../utils/constants.h"
#include "../utils/Transaction.h"
#include "../utils/util.h"
#include <sys/time.h>
#include <getopt.h>
#include <stdlib.h>
typedef unsigned long long timestamp_t;

static timestamp_t
get_timestamp ()
{
	struct timeval now;
	gettimeofday (&now, NULL);
	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

timestamp_t t0 = get_timestamp();
   // Process
timestamp_t t1 = get_timestamp();

double secs = (t1 - t0) / 1000000.0L;


static int count1 = -1;
static int count = 0;
class Client {
private:

	std::string host;
	std::string file;
	int concurrency;
	int port;
	int client_socket;
	std::shared_ptr<spdlog::logger> _logger;
public:
	pthread_t client_service_thread[THREAD_MAX];

	Client(std::string host, std::string file, int c, int port){
		client_socket = 0;
		_logger = spdlog::get("Server");
		this -> concurrency = c;
		this -> port = port;
		this -> host = host;
		this -> file = file;

	}
	int thread_status[THREAD_MAX] = {0};
	static void *client_service_invoke_helper(void *context){
		return ((Client *)context)->handle_client_service();
	}
	void create_client_Service(long int concurr);
	void  * handle_client_service();



	std::string do_transaction(Transaction t);

	const pthread_t* getClientServiceThread() const
	{
		return client_service_thread;
	}
};

#endif /* CLIENT_CLIENT_H_ */
