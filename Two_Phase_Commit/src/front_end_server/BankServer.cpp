/*
 * Server.cpp
 *
 *  Created on: Sep 16, 2019
 *      Author: Swapnil Bhosale
 */

#include "BankServer.h"

BankServer::BankServer() {
	mutex1 = PTHREAD_MUTEX_INITIALIZER;
	mwrite = PTHREAD_MUTEX_INITIALIZER;
	mread = PTHREAD_MUTEX_INITIALIZER;
	rallow = PTHREAD_MUTEX_INITIALIZER;
	serverSock = NULL;
	mutex_map = PTHREAD_MUTEX_INITIALIZER;
	_logger = spdlog::get("FrontEndServer");

}

/**
 * This function actually updates the value of customer in the hashmap
 * @param c It is the customer class object
 */
void BankServer::update_customer_map(Customer c) {
	pthread_mutex_lock(&mutex_map);
	BankServer::customer_map[c.getAccountNumber()] = c;
	pthread_mutex_unlock(&mutex_map);
}

/**
 * This function is a implementation of the intrest service which periodically
 * adds the intrest to the customeres
 */
/*
 void *BankServer::handle_intrest_service(){
 std::unordered_map<int, Customer>:: iterator itr;
 while(true){
 sleep(INTREST_SERVICE_SCHEDULE);
 for (itr = BankServer::customer_map.begin(); itr != BankServer::customer_map.end(); itr++) {
 int act_no = itr -> first;
 Customer c = itr -> second;
 double updated_bal = c.calculate_intrest();
 c.add_money(updated_bal);
 update_customer_map(c);
 }
 _logger -> info("Intrest service ran!");
 }
 }

 void BankServer::create_intrest_service(){
 pthread_create(&intrest_service__thread, NULL, &BankServer::intrest_service_invoke_helper, this);
 }
 */

/**
 * This method withdrawls the amount from the customer account
 *
 * @param tstamp It is a timestamp send by client
 * @param acc_no It is the account number
 * @param amt	It is the amount to be withdrawn
 * @return
 */
std::string BankServer::withdrawal(std::string tstamp, std::string acc_no,
		std::string amt) {
	std::string msg;
	int int_acc_no = stoi(acc_no);
	if (BankServer::customer_map.find(int_acc_no)
			== BankServer::customer_map.end()) {
		msg = "Customer id: " + acc_no + " not present!";
	}

	double amount = stod(amt);
	/*TransactionBuilder b;
	 Transaction trans = b.set_account_number(int_acc_no)
	 .set_timestamp(tstamp)
	 .set_transaction_type('W')
	 .set_amount(amount)
	 .build();*/
	//v.push_back(trans);
	msg = update_customer_by_id(stoi(acc_no), amount, 0);

	_logger->info("{}", msg);
	return msg;
}

/**
 * This function used mutes locking and implement reader protections
 * This function returns the value of customer from the hashmap
 * @param id This is the id of the customer whose record is to be retrieved
 * @return return the Customer class object
 */
Customer BankServer::get_customer_by_id(int id) {
	Customer c;

	pthread_mutex_lock(&rallow);
	pthread_mutex_lock(&mread);

	readcount++;

	if (readcount == 1)

		pthread_mutex_lock(&mutex1);

	pthread_mutex_unlock(&mread);

	pthread_mutex_unlock(&rallow);

	try {
		c = BankServer::customer_map.at(id);
	} catch (const std::out_of_range &oor) {
	}
	pthread_mutex_lock(&mread);
	readcount--;
	if (readcount == 0)
		pthread_mutex_unlock(&mutex1);
	pthread_mutex_unlock(&mread);
	return c;
}

/**
 * This function updates the customer's balances
 * Mutex writer implementation is added for thread synchronization and data race issues
 * @param id This is the id of the customer whose record needs to be updated
 * @param amount This is the amount to be deposited or withdrawn
 * @param op This is the opcode, it can be 1(deposit) or 0(withdrawal)
 * @return
 */
std::string BankServer::update_customer_by_id(int id, double amount, int op) {

	//Customer c = get_customer_by_id(id);
	std::string msg;
	pthread_mutex_lock(&mwrite);
	writecount++;
	if (writecount == 1)
		pthread_mutex_lock(&rallow);
	pthread_mutex_unlock(&mwrite);
	pthread_mutex_lock(&mutex1);

	Customer c = BankServer::customer_map.at(id);
	if (op == 1) {
		c.add_money(amount);
		msg = c.get_deposit_success_msg(amount);
	} else {
		if (c.can_withdraw(amount)) {
			c.reduce_money(amount);
			msg = c.get_withdrawl_success_msg(amount);

		} else {
			msg = c.get_withdraw_fail_msg(amount);
		}

	}
	BankServer::customer_map[id] = c;
	pthread_mutex_unlock(&mutex1);
	pthread_mutex_lock(&mwrite);
	writecount--;
	if (writecount == 0)
		pthread_mutex_unlock(&rallow);
	pthread_mutex_unlock(&mwrite);
	return msg;

}

/**
 * This function deposits the amount to the customer account
 * @param tstamp this is the timestamp sent by the server
 * @param acc_no this is the accoun_no of the customer
 * @param amt this is the amount to be withdrawn
 * @return The string message specifying the success or failure
 */
std::string BankServer::deposit(std::string tstamp, std::string acc_no,
		std::string amt) {
	std::string msg;
	int int_acc_no = stoi(acc_no);
	if (BankServer::customer_map.find(int_acc_no)
			== BankServer::customer_map.end()) {
		std::cout << "Cutomer id : " << int_acc_no << " not present!"
				<< std::endl;
		msg = "Customer id: " + acc_no + " not present!";
	} else {

		double amount = stod(amt);
		//std::vector<Transaction> &v = BankServer::transaction_map[int_acc_no];
		/*TransactionBuilder b;

		 Transaction trans = b.set_account_number(int_acc_no)
		 .set_timestamp(tstamp)
		 .set_transaction_type('W')
		 .set_amount(amount)
		 .build();*/

		msg = update_customer_by_id(int_acc_no, amount, 1);
	}
	_logger->info(msg);
	return msg;
}

/**
 * This method actually calls to withdrawl or deposit method
 * @param data This is the data received from the client
 * @param clientSocket This is client socket descriptor
 */
void BankServer::do_action(char *data, int clientSocket) {
	count += 1;
	char *buf;
	std::string msg;
	//std::string arr[4];
	//splitString(arr, std::string(data));
	int backendClient[3];
	int backendPorts[3];
	backendPorts[0] = htons(SERVER1);
	backendPorts[1] = htons(SERVER2);
	backendPorts[2] = htons(SERVER3);

	int backendServerCount = 0;
	int backendServerConn[3];

	for (int i = 0; i < 3; i++) {

		struct sockaddr_in serv_addr;
		if ((backendClient[i] = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("\n Socket creation error \n");
		}
		serv_addr.sin_family = PF_INET;
		serv_addr.sin_port = htons(backendPorts[i]);
		// Convert IPv4 and IPv6 addresses from text to binary form
		if (inet_pton(PF_INET, this->backendHost.c_str(), &serv_addr.sin_addr)
				<= 0) {
			perror("\nInvalid address/ Address not supported \n");
		}

		if (connect(backendClient[i], (struct sockaddr*) &serv_addr,
				sizeof(serv_addr)) < 0) {
			perror("\nConnection Failed \n");
			backendServerConn[i] = 0;
		} else {
			backendServerConn[i] = 1;
			backendServerCount += 1;
		}
	}

	while (true) {

		char msg[256];
		char status[256];

		//Read from client
		int r1 = read(clientSocket, msg, sizeof(msg));
		//cout<<"Transaction received: " << message << endl;

		//check and change the transaction details to uppercase
		for (int i = 0; i < sizeof(msg); i++) {
			msg[i] = toupper(msg[i]);
		}

		_logger->info("transaction received: {}", msg);

		if (strcmp(msg, "QUIT") == 0) {
			int s = send(clientSocket, "OK", 3, 0);

			close(clientSocket);
			break;
		}

		//ask the backend servers to provide their status
		for (int i = 0; i < 3; i++) {
			if (backendServerConn[i] == 1) {
				int status = send(backendClient[i], "VOTE", 256, 0);
			}
		}

		//Recieve status reply from the backend servers
		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		backendServerConn[0] = 0;
		backendServerConn[1] = 0;
		backendServerConn[2] = 0;
		char buff[256];
		int count = 0;

		for (int i = 0; i < 3; i++) {
			if (setsockopt(backendClient[i], SOL_SOCKET, SO_RCVTIMEO,
					(char*) &timeout, sizeof(timeout)) < 0) {
				perror("timeout error");
				exit(0);

			}
			while (recv(backendClient[i], buff, 256, 0) > 0) {
				count++;
				backendServerConn[i] = 1;
			}
		}
		//cout << "Count: "<< count << endl;
		if (count == 0) {
			_logger->warn("Active backend server count is zero!");
		}
		_logger->info("Active backend server count is: {}", backendServerCount);

		//sleep(3);

		if (count == 0) {
			//cout << "No active backend servers" << endl;
			sprintf(status, "ERROR: Please try after sometime");
			int s = send(clientSocket, status, sizeof(status), 0);
		} else {
			if (count == backendServerCount) {
				char buffer[256];
				for (int i = 0; i < 3; i++) {
					if (recv(backendClient[i], buffer, 256, 0) != 0) {
						if (backendServerConn[i] == 1) {
							int s3 = send(backendClient[i], "COMMIT", 256, 0);

							backendServerCount++;
						}
					} else {
						backendServerConn[i] = 0;
					}

				}

				bzero(buffer, 256);
				backendServerCount = 0;
				for (int i = 0; i < 3; i++) {

					if (recv(backendClient[i], buffer, 256, 0) != 0) {
						if (backendServerConn[i] == 1) {
							int s4 = send(backendClient[i], msg, sizeof(msg),
									0);
							backendServerCount++;
						}
					} else {
						backendServerConn[i] = 0;
					}
				}

				//Receive the updated transaction from the backend servers
				int y = 0;
				backendServerCount = 0;

				bzero(buffer, 256);
				bzero(status, 256);
				for (int i = 0; i < 3; i++) {

					if (recv(backendClient[i], status, 256, 0) != 0) {
						if (backendServerConn[i] == 1) {

							backendServerCount++;
						}
					} else {
						backendServerConn[i] = 0;
					}

				}

				int s3 = send(clientSocket, status, sizeof(status), 0);

			} else {
				int sock_closed;

				backendServerCount = 0;
				for (int i = 0; i < 3; i++) {
					if (setsockopt(backendClient[i], SOL_SOCKET, SO_RCVTIMEO,
							(char*) &timeout, sizeof(timeout)) < 0) {
						perror("Timeout error!");
						exit(0);

					}
					if (backendServerConn[i] == 1) {
						send(backendClient[i], "ABORT", 256, 0);
						backendServerCount++;
					}
				}
				for (int i = 0; i < 3; i++) {
					if (setsockopt(backendClient[i], SOL_SOCKET, SO_RCVTIMEO,
							(char*) &timeout, sizeof(timeout)) < 0) {
						perror("Timeout error!");
						exit(0);
					}
					if (backendServerConn[i] == 1) {
						int rd = read(backendClient[i], status, sizeof(status));
					}
				}

				int s3 = send(clientSocket, status, sizeof(status), 0);
			}
		}

		bzero(msg, 256);
		bzero(buff, 256);

	}
	close(clientSocket);
}

BankServer::~BankServer() {
	delete serverSock;
}

/**
 * This function initializes server.
 * It creates a server socket
 * Creates intrest service
 * Creates Thread pool
 * @param serverFile It is a path to Record.txt file
 * @param port It is a port number for the server to listen on
 * @param threadCount This is the thread count for the thread pool
 */
void BankServer::init(int port, int threadCount, std::string host) {
	//initialize_static_data(serverFile);
	serverSock = new ServerSock(port);
	this->backendHost = host;
	serverSock->init();
	//create_intrest_service();
	for (int i = 0; i < threadCount; ++i) {
		create_thread(i, serverSock);
	}
}

/**
 * This function actually reads the Records.txt and populates the HashMap
 * @param ipFile This is path to the file
 */
/*void BankServer::initialize_static_data(std::string ipFile) {
	std::ifstream file;
	file.open(ipFile);
	std::string line;
	if (file.is_open()) {
		while (getline(file, line)) {
			std::string arr[3];
			splitString(arr, line);

			CustomerBuilder b;
			Customer c = b.set_account_number(std::stoi(arr[0])).set_name(
					arr[1]).set_balance(std::stol(arr[2])).build();
			update_customer_map(c);
		}
		_logger->info("loaded static data of size {}", customer_map.size());
	}
	file.close();
}*/

/**
 * This function prints the HashMap contents when terminate signal is received on the
 * server
 * @param signal_Number it is the signal number intercepted by the program
 */
void BankServer::print_stats(int signal_Number) {
	std::cout << std::endl << "*********************************************"
			<< std::endl;
	std::cout << "Current state of the Customer records!" << std::endl
			<< std::endl;
	for (auto it = customer_map.cbegin(); it != customer_map.cend(); ++it) {

		std::cout << it->second << std::endl;
	}
	std::cout << std::endl << "*********************************************"
			<< std::endl;
	std::cout << "Total request received : " << count << std::endl;
	double user, sys;
	struct rusage myusage;

	if (getrusage(RUSAGE_SELF, &myusage) < 0)
		std::cout << "Error: getrusage()";

	user = (double) myusage.ru_utime.tv_sec
			+ myusage.ru_utime.tv_usec / 1000000.0;
	sys = (double) myusage.ru_stime.tv_sec
			+ myusage.ru_stime.tv_usec / 1000000.0;

	printf("\nuser time = %g, sys time = %g\n", user, sys);
	exit(0);
}

/**
 * This function creates a new thread
 * @param index this is the index of the thread in threadpool
 * @param serverSock This is the object of the ServerSock class
 */
void BankServer::create_thread(int index, ServerSock *serverSock) {
	pthread_create(&threads[index], NULL, &ServerSock::thread_pool_loop_helper,
			serverSock);
}

/**
 * This method prints the CLI usage
 * @param progname This is the program name
 */
static void usage(const char *progname) {
	fprintf(stderr, "Usage: %s [options] \n", progname);
	/* 80 column ruler:  ********************************************************************************
	 */
	fprintf(stderr, "Options are:\n");
	fprintf(stderr,
			"    -t thread pool size     Thread pool count, Default is 100\n");
	fprintf(stderr,
			"    -p port  	      	    Server port to listen, Default is 8080\n");
	fprintf(stderr,
			"    -h file         	    Host name for the Front End Server\n");
	exit(EINVAL);
}

int main(int argc, char **argv) {

	extern char *optarg;
	char c;
	int p = FRONT_END_PORT;
	std::string host(HOST);
	int thread_count = THREADS_COUNT;

	while ((c = getopt(argc, argv, ":h:p:")) != -1) {
		switch (c) {
		case 'p':
			p = atoi(optarg);
			if (p <= 0)
				usage(argv[0]);
			break;
		case 'h':
			host = optarg;
			if (host.length() == 0)
				usage(argv[0]);
			break;
		case 't':
			thread_count = atoi(optarg);
			if (p <= 0)
				usage(argv[0]);
			break;
		case ':':
			usage(argv[0]);
			break;
		case '?':
			usage(argv[0]);
			break;
		}
	}
	//create a logger object
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	sinks.push_back(
			std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
					"./logs/FrontEndServer.txt", 1024 * 1024 * 50, 10, true));
	auto combined_logger = std::make_shared<spdlog::logger>("FrontEndServer",
			begin(sinks), end(sinks));

	//set log level
	combined_logger->set_level(spdlog::level::info);

	//set logging pattern
	combined_logger->set_pattern(
			"[%Y-%m-%d %H:%M:%S.%e] [Thread - %t] [%l] %v");
	spdlog::register_logger(combined_logger);
	BankServer server;

	//initialize observer pattern and BankServer class as a observer
	ObserverPattern *obj = ObserverPattern::get_instance();
	Observer *ob = &server;
	obj->add_observant(ob);

	//catch for signal
	std::signal(SIGINT, server.print_stats);
	signal(SIGPIPE, server.print_stats);
	server.init(p, thread_count, host);

}
