#include <gtest/gtest.h>

#include "myecho_server.h"
#include "inc/stream_tcp_socket.h"
#include "inc/session.h"
#include "inc/tcp_client.h"
#include "inc/tcp_server.h"
#include "inc/exception.h"
#include <boost/make_shared.hpp>

GTEST_API_ int main(int argc, char **argv)
{
	printf("Running main() from myrpc_unittest.cc\n");
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(EchoServer, Add)
{
	using namespace boost;
	using namespace boost::asio;
    using namespace boost::asio::ip;
    using namespace msgpack;
    using namespace msgpack::myrpc;
    const char* port = "18811";

	try {
		const int PORT = 18811;
        tcp_server server(PORT, make_shared<myecho>());
	
		const char* port = "18811";
		tcp_client cli("127.0.0.1", port);

        int a = -12;
        int b = 13;
        callable cc = cli.call("add", a, b);
        int i = cc.get<int>();
		EXPECT_EQ((a+b), i);

	} catch(const argument_error& e) {
		ADD_FAILURE() << "argument_error exception " << e.what();
	}
	catch (const system::system_error& e)
	{
		ADD_FAILURE() << e.what();
	}	
	catch (...)
	{
		ADD_FAILURE() << "Uncaught exception";
	}
}


TEST(EchoServer, IncorrectPort)
{
	using namespace msgpack;
    using namespace msgpack::myrpc;
    using namespace boost;

	try {
        const int PORT = 18811;
        tcp_server server(PORT, make_shared<myecho>());       

    	tcp_client cli("127.0.0.1", "16296");
		cli.call("add", 1, 2).get<int>();
	} catch(const argument_error& e) {
		ADD_FAILURE() << "argument_error exception " << e.what();
	}
	catch (const system::system_error&)
	{
		SUCCEED();
		return;
	}	
	catch (...)
	{
		FAIL() << "Uncaught exception";
	}
	ADD_FAILURE() << "Didn't throw exception as expected";
}

TEST(EchoServer, NoMethodError)
{
	using namespace msgpack;
    using namespace msgpack::myrpc;
    using namespace boost;

	try {
        const int PORT = 18811;
        tcp_server server(PORT, make_shared<myecho>());       
   
		tcp_client cli("127.0.0.1", "18811");
		cli.call("sub", 1, 2).get<int>();
	} catch(const argument_error& e) {
		ADD_FAILURE() << "argument_error exception " << e.what();
	}	
	catch (const no_method_error&)
	{
		SUCCEED();
		return;
	}
	catch (...)
	{
		FAIL() << "Uncaught exception";
	}
	ADD_FAILURE() << "Didn't throw exception as expected";
}

TEST(EchoServer, Err)
{
	using namespace msgpack;
    using namespace msgpack::myrpc;
    using namespace boost;

	try {
        const int PORT = 18811;
        tcp_server server(PORT, make_shared<myecho>());       
   
		tcp_client cli("127.0.0.1", "18811");
		cli.call("err").get<int>();
	} 			
	catch (const std::exception&)
    {
		SUCCEED();
        return;
    }
	catch (...)
	{
		FAIL() << "Uncaught exception";
		return;
	}
	ADD_FAILURE() << "Didn't throw exception as expected";
}


TEST(EchoServer, Notify)
{
	using namespace msgpack;
    using namespace msgpack::myrpc;
    using namespace boost;

	try {
        const int PORT = 18811;
        tcp_server server(PORT, make_shared<myecho>());       
   
		tcp_client cli("127.0.0.1", "18811");
		cli.notify("echo");
	} catch(const argument_error& e) {
		ADD_FAILURE() << "argument_error exception " << e.what();
	}	
	catch (const system::system_error& e)
	{
		ADD_FAILURE() << "argument_error exception " << e.what();
	}
	catch (...)
	{
		FAIL() << "Uncaught exception";
	}	
}



// #include "attack.h"
// #include <cclog/cclog.h>
// #include <cclog/cclog_tty.h

/*
static size_t ATTACK_THREAD;
static size_t ATTACK_LOOP;

static std::auto_ptr<attacker> test;

void attack_connect()
{
	for(size_t i=0; i < ATTACK_LOOP; ++i) {
		rpc::client c(test->builder(), test->address());
		c.set_timeout(30.0);

		int result = c.call("add", 1, 2).get<int>();
		if(result != 3) {
			// LOG_ERROR("invalid response: ",result);
			std::cout << "invalid response: " << result << std::endl;
		}
	}
}

int AttackConnect()
{
	// cclog::reset(new cclog_tty(cclog::WARN, std::cout));
	signal(SIGPIPE, SIG_IGN);

	ATTACK_THREAD = attacker::option("THREAD", 25, 100);
	ATTACK_LOOP   = attacker::option("LOOP", 5, 50);

	std::cout << "connect attack"
		<< " thread=" << ATTACK_THREAD
		<< " loop="   << ATTACK_LOOP
		<< std::endl;

	test.reset(new attacker());
	test->run(ATTACK_THREAD, &attack_connect);
	
	return 0;
}

// Tests a heavy attack on the server
TEST(HeavyAttack, ConnectTest) {
  // This tests the connection between client and server
  EXPECT_EQ(0, AttackConnect());
}
*/