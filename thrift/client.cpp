#include <cstdlib>
#include <iostream>
#include <map>
#include <thread>

#include <boost/make_shared.hpp>
#include <thrift/transport/THttpClient.h>
#include <thrift/protocol/TJSONProtocol.h>

#include "UserManager.h"

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace boost;

void printUser(const test::User &user) {
	std::cout
		<< "User: " << user.id
		<< "; name: " << user.name
		<< "; email: " << user.email
	<< std::endl;
}

int main() {
	boost::shared_ptr<TTransport> transport(new THttpClient("localhost", 8080, "/"));
	boost::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport));
	test::UserManagerClient client(protocol);
	transport->open();
	std::vector<test::User> users;
	client.all(users);
	for (const auto &user: users)
		printUser(user);

	test::User user;
	client.get(user, 0);
	printUser(user);
	client.get(user, 1000);
	printUser(user);
	try {
		client.get(user, 404); // No such user will throw
		printUser(user);
	} catch(test::NotFound &err) {
		std::cout << "User " << err.id << " not found" << std::endl;
	}
	return EXIT_SUCCESS;
}
