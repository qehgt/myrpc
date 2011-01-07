// myrpc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

class session;
class socket {};
struct data_type {
    int i;
    float f;
};

typedef int session_id_type;
typedef boost::shared_future<data_type> future_data;

class callable {
public:
    callable(session_id_type id, const future_data& future_result, const boost::shared_ptr<session>& session) : 
      id(id), f(future_result), weak_session_ptr(session)
      {}
    ~callable();

    const future_data& get_future() const { return f; }


protected:
    session_id_type id;
    future_data f;
    boost::weak_ptr<session> weak_session_ptr;
};

class session : public boost::enable_shared_from_this<session> {
public:
    session() : current_id(0) {}

    callable create_call();
    void remove_unused_callable(session_id_type id);

protected:
    session_id_type current_id;
    typedef std::map<session_id_type, future_data> future_map_type;
    socket s;

    typedef boost::recursive_mutex mutex_type;
    mutex_type mutex;
    future_map_type future_map;
};

callable::~callable()
{
    if(boost::shared_ptr<session> r = weak_session_ptr.lock())
    {
        r->remove_unused_callable(id);
    }
}

void session::remove_unused_callable(session_id_type id)
{
    mutex_type::scoped_lock lock(mutex);
    future_map.erase(id);
}



callable session::create_call()
{
    mutex_type::scoped_lock lock(mutex);

    // create new future
    session_id_type id = current_id++;
    future_data new_future_data = future_map[id];
    
    return callable(id, new_future_data, shared_from_this());
}
int main()
{
    boost::shared_ptr<session> s(new session);
    s->create_call();
	return 0;
}
