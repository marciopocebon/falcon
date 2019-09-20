/*
 * Customer.h
 *
 *  Created on: Sep 20, 2019
 *      Author: lilbase
 */

#ifndef SERVER_CUSTOMER_H_
#define SERVER_CUSTOMER_H_

class Customer {
	friend class CustomerBuilder;
private:
	std::string name;
	int account_number;
	long int balance;
public:
	int getAccountNumber() const
	{
		return account_number;
	}

	void setAccountNumber(int accountNumber)
	{
		account_number = accountNumber;
	}

	long int getBalance() const
	{
		return balance;
	}

	void setBalance(long int balance)
	{
		this->balance = balance;
	}

	const std::string& getName() const
	{
		return name;
	}

	void setName(const std::string& name)
	{
		this->name = name;
	}
};

std::ostream& operator<< (std::ostream &out, Customer const& data) {
    out << data.getAccountNumber() << ", ";
    out << data.getName() << ", ";
    out << data.getBalance();
    return out;
}

class CustomerBuilder {
	private:
		Customer c;

	public:
		CustomerBuilder(){
			c.account_number = 0;
			c.balance = 0;
			c.name = "";
		}

		CustomerBuilder set_name(std::string name) {
			c.setName(name);
			return *this;
		}

		CustomerBuilder set_account_number(int num){
			c.setAccountNumber(num);
			return *this;
		}

		CustomerBuilder set_balance(long int bal){
			c.setBalance(bal);
			return *this;
		}

		Customer build() {
			return c;
		}
};


#endif /* SERVER_CUSTOMER_H_ */
