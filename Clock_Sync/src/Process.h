/*
 * Process.h
 *	This is the header file for the each individual process
 *	It defines class which holds the variables and method bound to each process in Causal and NonCausal
 *	Ordering
 *  Created on: Nov 13, 2019
 *      Author: lilbase
 */

#ifndef PROCESS_H_
#define PROCESS_H_
#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include <string>
#include<string.h>
#include<vector>
#include <sys/socket.h>
#include <unordered_map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <cctype>
#include "../include/spdlog/spdlog.h"
#include "../include/spdlog/sinks/stdout_color_sinks.h"
#include "../include/spdlog/sinks/rotating_file_sink.h"

using namespace std;
class Processes{

public:
	Processes(int id, char *msg, int multicastSocket, sockaddr_in addr1, sockaddr_in addr2, bool isCasual);
	void sendMessage();
	void receiveMessage();
private:
	int id;
	int multicastSocket;
	char * msg;
	int vectorClok[4] = {0, 0, 0, 0};
	int sendCnt;
	int recvCnt;
	std::shared_ptr<spdlog::logger> _logger;
	struct sockaddr_in sendAddr;
	struct sockaddr_in recvAddr;
	char *buff[100];
	vector<string> v;
	bool isCasual;
};


#endif /* PROCESS_H_ */
