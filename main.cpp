#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "tablereader.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<TableReader>("TableReader", 1, 0, "TableReader");
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);
    engine.loadFromModule("GraphPainter", "Main");

    return QCoreApplication::exec();
}
