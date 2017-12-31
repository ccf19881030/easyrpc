#include "rpc_client_test.h"
#include "../protoc/code/common.pb.h"
#include "easyrpc/easyrpc.h"

using namespace std::placeholders;

rpc_client_test::rpc_client_test()
{
    client_ = std::make_shared<rpc_client>();
    client_->set_session_status_callback(std::bind(&rpc_client_test::session_status_callback, this, _1, _2));
    client_->bind(std::bind(&rpc_client_test::received_sub_message, this, _1));
}

void rpc_client_test::run()
{
    bool ok = client_->connect("127.0.0.1:8888").connect_timeout(3).request_timeout(3).run();
    if (!ok)
    {
        log_error << "connect server failed";
        return;
    }

    call();
}

void rpc_client_test::stop()
{
    client_->stop();
    log_info << "rpc client stoped";
}

void rpc_client_test::session_status_callback(bool established, const std::string& session_id)
{
    if (established)
    {
        log_info << "session established, session id: " << session_id;
    }
    else 
    {
        log_warn << "session closed, session id: " << session_id;
    }
}

void rpc_client_test::received_sub_message(const std::shared_ptr<google::protobuf::Message>& message)
{
    log_info << message->DebugString();
}

void rpc_client_test::call()
{
    auto message = std::make_shared<echo_message>();
    message->set_str("Hello world");
    message->set_num(1024);

    client_->call(message, [](const std::shared_ptr<result>& ret)
    {
        log_info << ret->message()->DebugString();
    });
}