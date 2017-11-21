#include "task_dispatcher.h"
#include "easyrpc/utility/logger.h"

task_dispatcher::task_dispatcher(time_t request_timeout)
    : request_timeout_(request_timeout)
{
    threadpool_.init_thread_size(1);
    timer_.bind([this]{ check_request_timeout(); });
    timer_.start(1);
}

task_dispatcher::~task_dispatcher()
{
    stop();
}

void task_dispatcher::add_recv_handler(int serial_num, const recv_handler& handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_.emplace(serial_num, task{ handler, time(nullptr) });
}

void task_dispatcher::dispatch(const response_body& body)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = tasks_.find(body.serial_num);
    if (iter != tasks_.end())
    {
        threadpool_.add_task(iter->second.handler, body);
        tasks_.erase(body.serial_num);
    }
    else
    {
        log_warn() << "dispatch failed, serial num: " << body.serial_num << ", message name: " << body.message_name;
    }
}

void task_dispatcher::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_.clear();
}

void task_dispatcher::stop()
{
    timer_.destroy();
    threadpool_.stop();
}

void task_dispatcher::check_request_timeout()
{
    std::lock_guard<std::mutex> lock(mutex_);
    time_t current_time = time(nullptr);
    auto begin = tasks_.begin();
    auto end = tasks_.end();

    while (begin != end)
    {
        if (current_time - begin->second.begin_time >= request_timeout_)
        {
            response_body body;
            body.code = rpc_error_code::request_timeout;
            threadpool_.add_task(begin->second.handler, body);
            begin = tasks_.erase(begin);
        }
        else
        {
            ++begin;
        }
    }
}
