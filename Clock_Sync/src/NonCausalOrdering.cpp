/*
 * CausalOrdering.cpp
 *
 *  Created on: Nov 12, 2019
 *      Author: lilbase
 */

#include "CausalOrdering.h"


/**
 * This is the constructor initializes the class instance variables

 */
CausalOrdering::CausalOrdering(int port, int id, char * msg){
	this -> multicastSock = socket(AF_INET, SOCK_DGRAM, 0);;
	this -> pointToPointSock = 0;
	this -> senderThread = NULL;
	this -> recvThread = NULL;
	this -> _logger = spdlog::get(NON_CAUSAL);
	this -> port = port;
	this -> id = id;
	this -> msg = msg;
	this -> process = NULL;
}

/**
 * This method initializes the Multicast and point to point socket
 * Bind these sockets to the operating system and set multicast options to the socket
 *
 * @param NA
 * @return NA
 */
void CausalOrdering::init(){
	int enable = 1;

	if (setsockopt(multicastSock,
			SOL_SOCKET, SO_REUSEADDR,
			&enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR)");

	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(port);
	srvAddr.sin_addr.s_addr = inet_addr(HOST);


	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr =htonl(INADDR_ANY);
	recvAddr.sin_port = htons(port);

	this -> process = new Processes(id, msg, multicastSock, srvAddr, recvAddr, false);

	if (bind(multicastSock, (const struct sockaddr *)&recvAddr, sizeof(recvAddr)) < 0){
		_logger -> error("Error in bind()");
		exit(1);
	}

	_logger -> info("bound the first port");

	struct ip_mreq mreq;
	if (-1 == inet_pton(AF_INET, HOST, &recvAddr.sin_addr))
		_logger -> info("cannot set multicast address: %s\n", strerror(errno));
	memcpy(&mreq.imr_multiaddr.s_addr, (void*)&recvAddr.sin_addr, sizeof(struct in_addr));
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	struct ip_mreq multicast_req;
	multicast_req.imr_multiaddr.s_addr = inet_addr(HOST);
	multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(multicastSock,
			IPPROTO_IP, IP_ADD_MEMBERSHIP,
			&multicast_req, sizeof(multicast_req)) < 0)
		perror("setsockopt(IP_ADD_MEMBERSHIP)");


	pointToPointSock = socket(AF_INET, SOCK_DGRAM, 0);
	p2pAddr.sin_family = AF_INET;
	p2pAddr.sin_port = htons(INADDR_ANY);
	p2pAddr.sin_addr.s_addr = INADDR_ANY;

	if (setsockopt(pointToPointSock,
			SOL_SOCKET, SO_REUSEADDR,
			&enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR pointToPointSock)");

	if (bind(pointToPointSock, (const struct sockaddr *)&p2pAddr, sizeof(p2pAddr)) < 0){
		_logger -> error("Error in bind pointToPointSock()");
		exit(1);
	}


	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	if(setsockopt(pointToPointSock,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout))<0)
	{
		cout << "Timeout eror" << endl;
	}


}

/**
 * This method calls the thread method which handles the message sending
 */
void *CausalOrdering::handleSenderService(){
	while(true)
		process -> sendMessage();
}

/**
 * This method calls the thread method which handles the message receiving
 */
void *CausalOrdering::handleRecvService(){
	process -> receiveMessage();
}

/**
 * This method is responsible for creating the sender and receiver threads
 */
void CausalOrdering::createSendAndRecvThread(){
	int res;
	res = pthread_create(&senderThread, NULL, &CausalOrdering::senderServiceHelper, this);
	if(res<0) {
		_logger -> error("Error in creating sender thread!");
		exit(1);
	}

	res = pthread_create(&recvThread,NULL, &CausalOrdering::recvServiceHelper, this);
	if(res<0)
	{
		_logger -> error("Error in creating receiver thread!");
		exit(1);
	}
}

/**
 * This method prints the usage of command line options
 */
static void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] \n", progname);
/* 80 column ruler:  ********************************************************************************
 */
    fprintf(stderr, "Options are:\n");
    fprintf(stderr, "    -t TimeDrift     Time drift from current clock epoch, Default +10\n");
    fprintf(stderr, "    -p port  	      	    port to listen, Default is 8080\n");
    fprintf(stderr, "    -f file         	    Address of startup data file for customers, Default is './src/Records.txt'\n");
    exit(EINVAL);
}

/**
 * This is the main function
 */
int main(int argc, char **argv) {
	extern char *optarg;
	char c;
	long int timeDrift = TIME_DRIFT;
	int p = PORT;
	bool isCordinator = false;
	int id;
	char * msg;
	while ((c = getopt (argc, argv, ":p:i:m:")) != -1) {
		switch(c) {
		case 'p' :
			p = atoi(optarg);
			if(p <= 0)
				usage(argv[0]);
			break;
		case 'm' :
			msg = optarg;
			break;
		case 'i' :
			id = atoi(optarg);
			break;
		case ':':
			usage(argv[0]);
			break;
		case '?':
			usage(argv[0]);
			break;

		}
	}
	pid_t pid = getpid();

	string logFileName = string("./logs/").append(NON_CAUSAL).append("_").append(to_string(pid)).append(".txt");
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFileName,1024 * 1024 * 50, 10, true));
	auto combined_logger = std::make_shared<spdlog::logger>(NON_CAUSAL, begin(sinks), end(sinks));
	combined_logger -> set_level(spdlog::level::info);
	combined_logger -> set_pattern("[%Y-%m-%d %H:%M:%S.%e] [Thread - %t] [%l] %v");
	spdlog::register_logger(combined_logger);
	CausalOrdering causalOrdering(p, id, msg);
	causalOrdering.init();
	causalOrdering.createSendAndRecvThread();
	(void) pthread_join(causalOrdering.getSenderThread(), NULL);
	(void) pthread_join(causalOrdering.getRecvThread(), NULL);
}


