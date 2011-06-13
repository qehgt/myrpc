#ifndef H_MYRPC_RPC_CLIENT_H
#define H_MYRPC_RPC_CLIENT_H

#include <boost/shared_ptr.hpp>
#include "session.h"

namespace msgpack {
namespace myrpc {

class rpc_client {
public:
    virtual ~rpc_client()
    {}

    inline callable call(const std::string& name);

    inline void notify(const std::string& name);

    template <typename A1>
    inline callable call(const std::string& name,
			const A1& a1);

    template <typename A1>
    inline void notify(const std::string& name,
			const A1& a1);

    template <typename A1, typename A2>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2);

    template <typename A1, typename A2>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2);

    template <typename A1, typename A2, typename A3>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3);

    template <typename A1, typename A2, typename A3>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3);

    template <typename A1, typename A2, typename A3, typename A4>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4);

    template <typename A1, typename A2, typename A3, typename A4>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4);

    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5);

    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9);


protected:
    boost::shared_ptr<myrpc::session> session;
};


inline callable rpc_client::call(const std::string& name)
{
    return session->call(name);
}

inline void rpc_client::notify(const std::string& name)
{
    session->notify(name);
}

template <typename A1>
inline callable rpc_client::call(const std::string& name,
            const A1& a1)
{
    return session->call(name, a1);
}

template <typename A1>
inline void rpc_client::notify(const std::string& name,
            const A1& a1)
{
    session->notify(name, a1);
}

template <typename A1, typename A2>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2)
{
    return session->call(name, a1, a2);
}

template <typename A1, typename A2>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2)
{
    session->notify(name, a1, a2);
}

template <typename A1, typename A2, typename A3>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3)
{
    return session->call(name, a1, a2, a3);
}

template <typename A1, typename A2, typename A3>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3)
{
    session->notify(name, a1, a2, a3);
}

template <typename A1, typename A2, typename A3, typename A4>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
    return session->call(name, a1, a2, a3, a4);
}

template <typename A1, typename A2, typename A3, typename A4>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
    session->notify(name, a1, a2, a3, a4);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
    return session->call(name, a1, a2, a3, a4, a5);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
    session->notify(name, a1, a2, a3, a4, a5);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
    return session->call(name, a1, a2, a3, a4, a5, a6);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
    session->notify(name, a1, a2, a3, a4, a5, a6);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
    return session->call(name, a1, a2, a3, a4, a5, a6, a7);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
    session->notify(name, a1, a2, a3, a4, a5, a6, a7);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
    return session->call(name, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
    session->notify(name, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline callable rpc_client::call(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
{
    return session->call(name, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline void rpc_client::notify(const std::string& name,
            const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
{
    session->notify(name, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


}  // namespace myrpc
}  // namespace msgpack

#endif // H_MYRPC_RPC_CLIENT_H
