#include "logger.h"

logger_stream::logger_stream(const std::string& file_path, 
                             const std::string& func_name, 
                             unsigned long line, 
                             priority_level level) 
    : level_(level)
{
    buffer_ = std::make_shared<std::ostringstream>();
#ifdef _WIN32
    int pos = file_path.find_last_of("\\");
#else
    int pos = file_path.find_last_of("/");
#endif
    (*buffer_) << file_path.substr(pos + 1) << " " << func_name << "(" << line << ") ";
}

logger_stream::~logger_stream()
{
    std::string log = make_log();
    print_log(log);
}

std::string logger_stream::get_current_time_us()
{
    struct timeval now_tv;
#ifdef _WIN32
    gettimeofday(&now_tv);
#else
    gettimeofday(&now_tv, nullptr);
#endif
    const time_t seconds = now_tv.tv_sec;
    struct tm t;
#ifdef _WIN32
    localtime_s(&t, &seconds);
#else
    localtime_r(&seconds, &t);
#endif
    char time_str[27] = {'\0'};
    snprintf(time_str, sizeof(time_str), "%04d-%02d-%02d %02d:%02d:%02d.%06d",
             t.tm_year + 1900,
             t.tm_mon + 1,
             t.tm_mday,
             t.tm_hour,
             t.tm_min,
             t.tm_sec,
             static_cast<int>(now_tv.tv_usec));

    return time_str;
}

std::string logger_stream::level_enum_to_string()
{
    switch (level_)
    {
    case priority_level::error: return "[error]";
    case priority_level::warn: return "[warn]";
    case priority_level::info: return "[info]";
    case priority_level::debug: return "[debug]";
    default: return "[debug]";
    }
}

std::string logger_stream::make_log()
{
    std::string log;
    log += get_current_time_us();
    log += " ";
    log += level_enum_to_string();
    log += " ";
    log += buffer_->str();

    return log;
}

void logger_stream::print_log(const std::string& log)
{
    if (level_ == priority_level::error)
    {
        // red
        printf("\033[31m%s\n\033[0m", log.c_str());
    }
    else if (level_ == priority_level::warn)
    {
        // yellow
        printf("\033[33m%s\n\033[0m", log.c_str());
    }
    else
    {
        // normal
        printf("%s\n", log.c_str());
    }
}

logger::logger(const char* file_path, 
               const char* func_name, 
               unsigned long line, 
               priority_level level) 
    : file_path_(file_path), 
    func_name_(func_name), 
    line_(line), 
    level_(level) 
{

}

logger_stream logger::log() const
{
    return logger_stream(file_path_, func_name_, line_, level_);
}
