#include "myecho_server.h"
#include "inc/stream_tcp_socket.h"
#include "inc/session.h"
#include "inc/tcp_client.h"
#include "inc/tcp_server.h"
#include <boost/thread.hpp>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>

namespace {
    class run_me {
    public:
        run_me()
        {
            int tmpDbgFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG); 
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT); 
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG); 
            _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT); 
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG); 
            _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

            tmpDbgFlag |=_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF;
            _CrtSetDbgFlag(tmpDbgFlag);
        }
    };

    run_me run_me; // static class with constructor
}
#endif

void run_client_test()
{
    using namespace boost::asio;
    using namespace boost::asio::ip;
    using namespace msgpack;
    using namespace msgpack::myrpc;
    const char* port = "18811";

    try {
        tcp_client cli("localhost", port);

        callable cc = cli.call("add", -12, 13);
        int i = cc.get<int>();
        printf("i = %d\n", i);
        try {
            i = cli.call("add", 12, 13).get<int>();
        }
        catch (const std::exception& e)
        {
            printf("internal catch: %s\n", e.what());
        }

        printf("i = %d\n", i);
        std::string str = cli.call("echo", std::string("aaaBBB")).get<std::string>();
        printf("str = %s\n", str.c_str());
    }
    catch (const boost::exception& e)
    {
        printf("boost ex: %s\n", boost::diagnostic_information(e).c_str());
    }
    catch (const std::exception& e)
    {
        printf("ex: %s\n", e.what());
    }
    catch (...) {
        printf("client: unknown ex=\n");
    }
}

int main()
{
    using namespace msgpack;
    using namespace msgpack::myrpc;

    try {
        /*
        using namespace boost::asio;
        const int PORT = 18811;
        io_service io;

        msgpack::myrpc::shared_dispatcher dispatcher(new myecho());
        boost::shared_ptr<stream_tcp_socket> socket(new stream_tcp_socket(io));
        boost::shared_ptr<msgpack::myrpc::session> s(new msgpack::myrpc::session(
            boost::static_pointer_cast<io_stream_object>(socket),
            dispatcher));

        boost::thread t_client(run_client_test);

        ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), PORT));
        acceptor.accept(*socket);

        s->start();
        boost::thread t(boost::bind(&io_service::run, &io));

        t.join();
        t_client.join();
        */

        using namespace boost::asio;
        const int PORT = 18811;

        tcp_server server(PORT, shared_dispatcher(new myecho()));

        boost::thread t_client1(run_client_test);
        boost::thread t_client2(run_client_test);
        run_client_test();
        t_client1.join();
        t_client2.join();
    }
    catch (std::exception& e) {
        printf("main: ex=%s\n", e.what());
    }

    return 0;
}
