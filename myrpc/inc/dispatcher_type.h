//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_MYRPC_DISPATCHER_TYPE_H__
#define MSGPACK_MYRPC_DISPATCHER_TYPE_H__

#include "request.h"

namespace msgpack {
namespace myrpc {


class dispatcher_type {
public:
    virtual ~dispatcher_type() {}

    virtual void dispatch(msgpack::myrpc::request req) = 0;
};
typedef boost::shared_ptr<dispatcher_type> shared_dispatcher;

class dummy_dispatcher_type : public dispatcher_type {
public:
    void dispatch(msgpack::myrpc::request req)
    {
        req.error(msgpack::myrpc::NO_METHOD_ERROR);
    }
};

}  // namespace myrpc
}  // namespace msgpack

#endif // MSGPACK_MYRPC_DISPATCHER_TYPE_H__
