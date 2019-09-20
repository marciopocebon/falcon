/*
 * Transaction.h
 *
 *  Created on: Sep 20, 2019
 *      Author: lilbase
 */

#ifndef SERVER_TRANSACTION_H_
#define SERVER_TRANSACTION_H_


class Transaction {
private:
	std::string name;
	int account_number;
	char type;
	long int amount;
public:
	Transaction();

	int getAccountNumber() const
	{
		return account_number;
	}

	void setAccountNumber(int accountNumber)
	{
		account_number = accountNumber;
	}

	long int getAmount() const
	{
		return amount;
	}

	void setAmount(long int amount)
	{
		this->amount = amount;
	}

	const std::string& getName() const
	{
		return name;
	}

	void setName(const std::string& name)
	{
		this->name = name;
	}

	char getType() const
	{
		return type;
	}

	void setType(char type)
	{
		this->type = type;
	}
};

class TransactionBuilder{
public:
	Transaction *t;
	TransactionBuilder(){
		t = new Transaction();
	}

	TransactionBuilder set_name(std::string name){
		t->setName(name);
		return *this;
	}

	TransactionBuilder set_transaction_type(char type) {
		t->setType(type);
		return *this;
	}

	TransactionBuilder set_amount(long int amount) {
		t->setAmount(amount);
		return *this;
	}

	TransactionBuilder set_account_number(int account_number) {
		t->setAccountNumber(account_number);
		return *this;
	}

	Transaction * build(){
		return t;
	}

};



#endif /* SERVER_TRANSACTION_H_ */
