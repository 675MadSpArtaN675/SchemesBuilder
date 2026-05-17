#ifndef LOGGER
#define LOGGER

#include <memory>
#include <concepts>
#include <algorithm>

#include <QList>

#include <boost/log/core.hpp>
#include <boost/log/common.hpp>

#include <boost/algorithm/string.hpp>

class CoreLogger
{
public:
    static boost::log::core_ptr get_logger()
    {
        return boost::log::core::get();
    }

    CoreLogger() : _logger(get_logger())
    {
        _rec = _logger->open_record(boost::log::attribute_value_set());
        _ostream = std::make_unique<boost::log::record_ostream>(_rec);

    }

    void log(std::string message) const
    {
        *_ostream << message;
        _ostream->flush();
    }

protected:
    template<typename T>
    requires std::integral<T>
    std::string join_list(QList<T> _strings, std::string delimiter)
    {
        std::vector<std::string> pre_result;
        std::transform(_strings.begin(), _strings.end(), std::back_inserter(pre_result),
                            [](T par){
                                return std::to_string(par);
                            }
        );
        std::string _result = std::reduce(pre_result.begin(), pre_result.end(), std::string(),
                    [delimiter](const std::string& s1, const std::string& s2){ return s1 + delimiter + s2; });

        return _result;
    }

    boost::log::record _rec;
    boost::log::core_ptr _logger;

    std::unique_ptr<boost::log::record_ostream> _ostream;
};

#endif //
