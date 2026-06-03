#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/log/sinks.hpp>

#include <boost/format.hpp>

#include "CoreLogger.hpp"


namespace lg = boost::log;
namespace sk = boost::log::sinks;



void logger_config(std::string filename)
{
    using file_backend = sk::text_file_backend;
    using file_task = sk::asynchronous_sink<file_backend>;

    using console_backend = sk::text_ostream_backend;
    using console_task = sk::asynchronous_sink<console_backend>;

    boost::format _name_formatter("%s%s.log");
    std::string filename_ = (_name_formatter % filename % "").str();
    std::string target_filename = (_name_formatter % filename % "_%3N").str();

    std::clog << filename_ << " " << target_filename << std::endl;
    boost::shared_ptr<file_task> file_sink = boost::make_shared<file_task>(
                boost::make_shared<file_backend>()
            );

    file_sink->locked_backend()->set_file_name_pattern(filename_);
    file_sink->locked_backend()->set_target_file_name_pattern(target_filename);
    file_sink->locked_backend()->set_open_mode(std::ios_base::out | std::ios_base::app);

    boost::shared_ptr<console_task> console_backend_task = boost::make_shared<console_task>(
                boost::make_shared<console_backend>()
            );

    console_backend_task->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::clog, null_deleter()));

    lg::core_ptr core_instance = lg::core::get();
    core_instance->add_sink(file_sink);
    core_instance->add_sink(console_backend_task);

}

void clean_log()
{
    boost::log::core::get()->flush();
    boost::log::core::get()->remove_all_sinks();
}

