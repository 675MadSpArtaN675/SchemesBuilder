#ifndef LOGGER
#define LOGGER

#include <concepts>
#include <algorithm>

#include <QList>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/common.hpp>
#include <boost/log/sinks.hpp>

#include <boost/algorithm/string.hpp>

void logger_config(std::string);
void clean_log();

struct null_deleter
{
    void operator()(const std::ostream* ptr)
    { }
};

class CoreLogger
{
public:
    static boost::log::core_ptr get_logger()
    {
        return boost::log::core::get();
    }

    CoreLogger()
    {
        _logger = get_logger();
    }

    void log(std::string message) const
    {
        if (_logger) {
        	BOOST_LOG_TRIVIAL(info) << message;
        }
    }

    void log_debug(std::string message) const
    {
        if (_logger) {
        	BOOST_LOG_TRIVIAL(debug) << message;
        }
    }

    void log_error(std::string message) const
    {
        if (_logger) {
			BOOST_LOG_TRIVIAL(error) << message;
        }
    }

protected:
    template<typename T>
    requires std::integral<T>
    std::string join_list(QList<T> _strings, std::string delimiter)
    {
        std::string _result;
        if (_strings.size() > 0) {
			std::vector<std::string> pre_result;
			std::transform(_strings.begin(), _strings.end(), std::back_inserter(pre_result),
								[this](T par){
                					log_debug(std::string("Nice vertex: ") + std::to_string(par));
									return std::to_string(par);
								}
			);
			_result = std::reduce(pre_result.begin(), pre_result.end(), std::string(),
                    [delimiter](const std::string& s1, const std::string& s2){ return s1 + delimiter + s2; });
		}

        return _result;
    }

    boost::log::record _rec;
    boost::log::core_ptr _logger;
};

#endif //
