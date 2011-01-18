#ifndef H_MYECHO_SERVER_H
#define H_MYECHO_SERVER_H

#include "inc/dispatcher_type.h"

class myecho : public msgpack::myrpc::dispatcher_type {
public:
    typedef msgpack::myrpc::request request;

	void dispatch(request req)
    {
        try {
            std::string method;
            req.method().convert(&method);

            if (method == "add") {
                msgpack::type::tuple<int, int> params;
                req.params().convert(&params);
                add(req, params.get<0>(), params.get<1>());

            } else if (method == "echo") {
                msgpack::type::tuple<std::string> params;
                req.params().convert(&params);
                echo(req, params.get<0>());

            } else if (method == "echo_huge") {
                msgpack::type::tuple<msgpack::type::raw_ref> params;
                req.params().convert(&params);
                echo_huge(req, params.get<0>());

            } else if (method == "err") {
                msgpack::type::tuple<> params;
                req.params().convert(&params);
                err(req);

            } else {
                req.error(msgpack::myrpc::NO_METHOD_ERROR);
            }

        } catch (msgpack::type_error&) {
            req.error(msgpack::myrpc::ARGUMENT_ERROR);
            return;

        } catch (std::exception& e) {
            req.error(std::string(e.what()));
            return;
        }
    }


	void add(request req, int a1, int a2)
	{
		req.result(a1 + a2);
	}

	void echo(request req, const std::string& msg)
	{
		req.result(msg);
	}

	void echo_huge(request req, const msgpack::type::raw_ref& msg)
	{
		req.result(msg);
	}

	void err(request req)
	{
		req.error(std::string("always fail"));
	}
};

#endif //  H_MYECHO_SERVER_H
