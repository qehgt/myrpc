#ifndef H_MYRPC_TCP_CLIENT_H
#define H_MYRPC_TCP_CLIENT_H

#include <boost/shared_ptr.hpp>
#include "inc/session.h"

namespace msgpack {
namespace myrpc {

class tcp_client {
public:
    tcp_client(const char* host, const char* service_name);
    ~tcp_client();

    inline callable call(const std::string& name);

    template <typename A1>
    inline callable call(const std::string& name, const A1& a1);

    template <typename A1, typename A2>
    inline callable call(const std::string& name, const A1& a1, const A2& a2);

    inline void notify(const std::string& name);

    template <typename A1>
    inline void notify(const std::string& name, const A1& a1);

    template <typename A1, typename A2>
    inline void notify(const std::string& name, const A1& a1, const A2& a2);

protected:
    struct tcp_client_impl;
    boost::shared_ptr<tcp_client_impl> pimpl;
    boost::shared_ptr<myrpc::session> session;
};


inline 
callable tcp_client::call(const std::string& name)
{
    return session->call(name);
}

template <typename A1>
inline callable tcp_client::call(const std::string& name, const A1& a1)
{
    return session->call(name, a1);
}

template <typename A1, typename A2>
inline callable tcp_client::call(const std::string& name, const A1& a1, const A2& a2)
{
    return session->call(name, a1, a2);
}

inline void tcp_client::notify(const std::string& name)
{
    session->notify(name);
}

template <typename A1>
inline void tcp_client::notify(const std::string& name, const A1& a1)
{
    session->notify(name, a1);
}

template <typename A1, typename A2>
inline void tcp_client::notify(const std::string& name, const A1& a1, const A2& a2)
{
    session->notify(name, a1, a2);
}

}  // namespace myrpc
}  // namespace msgpack

#endif // H_MYRPC_TCP_CLIENT_H
