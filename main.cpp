#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <boost/smart_ptr.hpp>
#include <boost/format.hpp>

#include <boost/log/trivial.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>

#include "graph_data.hpp"
#include "graphbuilder.hpp"
#include "tableformer.hpp"
#include "tableformerdatabasedtype.hpp"
#include "tablereader.hpp"

#include "graph_painter.hpp"
#include "CoreLogger.hpp"

namespace lg = boost::log;
namespace sk = boost::log::sinks;

int main(int argc, char *argv[])
{
    logger_config("app_painter");
    BOOST_LOG_TRIVIAL(info) << "Start application!";

    QGuiApplication app(argc, argv);
    app.setOrganizationDomain("Corp");
    app.setOrganizationName("SpartanCorp");
    app.setApplicationName("GraphPainter");

    qmlRegisterType<TableReader>("TableReader", 1, 0, "TableReader");
    qmlRegisterType<GraphNode>("GraphPaint", 1, 0, "graphNode");
    qmlRegisterType<graph_data>("GraphPaint", 1, 0, "graphData");
    qmlRegisterType<TableFormer>("TableReader", 1, 0, "TableFormer");
    qmlRegisterType<GraphBuilder>("GraphPaint", 1, 0, "GraphBuilder");
    qmlRegisterType<GraphOptions>("GraphPaint", 1, 0, "GraphOptions");
    qmlRegisterType<GraphPainter>("GraphPaint", 1, 0, "GraphPainter");
    qmlRegisterType<TableFormerDatabasedType>("GraphPaint", 1, 0, "TFormer_DT");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);
    engine.loadFromModule("GraphPainter", "Main");

    int code = app.exec();
    clean_log();

    return code;
}
