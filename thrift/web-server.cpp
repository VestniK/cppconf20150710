#include <cstdlib>
#include <iostream>
#include <map>
#include <thread>

#include <boost/make_shared.hpp>
#include <thrift/async/TEvhttpServer.h>
#include <thrift/async/TAsyncProtocolProcessor.h>
#include <thrift/protocol/TJSONProtocol.h>

#include "UserManager.h"

using namespace apache::thrift;
using namespace boost;

class UserManager: public test::UserManagerCobSvIf {
public:
	UserManager() {
		mUsers[0] = test::User();
		mUsers[0].id = 0;
		mUsers[0].name = "root";
		mUsers[0].email = "admin@localhost";

		mUsers[1000] = test::User();
		mUsers[1000].id = 1000;
		mUsers[1000].name = "vestnik";
		mUsers[1000].email = "svidyuk@ac-sw.com";
	}

	virtual void all(tcxx::function<void(const std::vector<test::User>&)> onRes) override {
		std::vector<test::User> res;
		for (const auto &pair: mUsers)
			res.push_back(pair.second);
		return onRes(res);
	}

	virtual void get(
		tcxx::function<void(const test::User&)> onRes,
		tcxx::function<void(TDelayedException*)> onErr,
		const int32_t id
	) override {
		test::User res;
		const auto sres = mUsers.find(id);
		if (sres == mUsers.end()) {
			test::NotFound err;
			err.id = id;
			return onErr(TDelayedException::delayException(err));
		}
		return onRes(sres->second);
	}

private:
	std::map<int32_t, test::User> mUsers;
};

int main() {
	auto service = make_shared<UserManager>();
	auto processor = make_shared<test::UserManagerAsyncProcessor>(service);

// 	auto threadManager = server::ThreadManager::newSimpleThreadManager(std::thread::hardware_concurrency());
// 	auto threadFactory = make_shared<concurrency::PosixThreadFactory>();
// 	threadManager->threadFactory(threadFactory);
// 	// TODO: Use it to handle actual requests
// 	threadManager->start();

	auto protocolFactory = make_shared<protocol::TJSONProtocolFactory>();
	auto protocolProcessor = make_shared<async::TAsyncProtocolProcessor>(processor, protocolFactory);

	auto server = make_shared<async::TEvhttpServer>(protocolProcessor, 8080);
	server->serve();
	return EXIT_SUCCESS;
}
