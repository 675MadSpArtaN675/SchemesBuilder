#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <boost/smart_ptr.hpp>
#include <boost/format.hpp>

#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>

#include "graph_data.hpp"
#include "graphbuilder.hpp"
#include "tableformer.hpp"
#include "tablereader.hpp"

#include "graph_painter.hpp"

namespace lg = boost::log;
namespace sk = boost::log::sinks;

struct null_deleter
{
    void operator()(const boost::shared_ptr<std::ostream>& ptr)
    { }
};

void logger_config(std::string filename)
{
    using file_backend = sk::text_file_backend;
    using file_task = sk::asynchronous_sink<file_backend>;

    using console_backend = sk::text_ostream_backend;
    using console_task = sk::asynchronous_sink<console_backend>;

    boost::format _name_formatter = boost::format("%s%s.log") % filename;

    boost::shared_ptr<file_task> file_sink = boost::make_shared<file_task>(
                boost::make_shared<file_backend>(
                    lg::keywords::file_name = (_name_formatter % "").str(),
                    lg::keywords::target_file_name = (_name_formatter % "_%3N").str(),
                    lg::keywords::auto_flush = true
                )
            );

    boost::shared_ptr<console_task> console_backend_task = boost::make_shared<console_task>(
                boost::make_shared<console_backend>(
                    boost::shared_ptr<std::ostream>(&std::cout, null_deleter()),
                    lg::keywords::auto_flush = true
                )
            );

    lg::core_ptr core_instance = lg::core::get();

    core_instance->add_sink(file_sink);
    core_instance->add_sink(console_backend_task);

}

int main(int argc, char *argv[])
{
    logger_config("app_painter");
    QGuiApplication app(argc, argv);

    qmlRegisterType<TableReader>("TableReader", 1, 0, "TableReader");
    qmlRegisterType<GraphNode>("TableReader", 1, 0, "GraphNode");
    qmlRegisterType<graph_data>("TableReader", 1, 0, "GraphData");
    qmlRegisterType<TableFormer>("TableReader", 1, 0, "TableFormer");
    qmlRegisterType<GraphBuilder>("TableReader", 1, 0, "GBuilder");
    qmlRegisterType<GraphPainter>("Painters", 1, 0, "GraphPainter");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);
    engine.loadFromModule("GraphPainter", "Main");

    return QCoreApplication::exec();
}
