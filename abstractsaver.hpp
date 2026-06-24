#ifndef ABSTRACTSAVER_HPP
#define ABSTRACTSAVER_HPP

#include <memory>

#include <QObject>
#include <QBindable>
#include <QException>

#include <QMap>
#include <QFile>
#include <QFileDevice>
#include <QVariant>

#include <QQmlEngine>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "CoreLogger.hpp"
#include "graph_data.hpp"
#include "graphnode.hpp"

QString escapeXml(const QString &str);

class AbstractSaver : public QObject, protected CoreLogger
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString options_file READ options_file WRITE set_option_file BINDABLE bindable_options_filename)
    Q_PROPERTY(graph_data* graph READ graph WRITE set_graph BINDABLE bindable_graph)

public:
    explicit AbstractSaver(QObject *parent = nullptr);
    virtual ~AbstractSaver();

    Q_INVOKABLE void load_options(QString filename);
    Q_INVOKABLE void save(QString filename);

    Q_INVOKABLE void set_option(QString _key, QString _value);
    Q_INVOKABLE QVariant get_option_value(QString _key);

    Q_INVOKABLE bool is_base_element_load();
    Q_INVOKABLE bool is_ready();

    Q_INVOKABLE void clear();

    QString options_file();
    void set_option_file(QString filename);
    QBindable<QString> bindable_options_filename();

    void set_graph(graph_data* _graph_data);
    graph_data* graph();
    QBindable<graph_data*> bindable_graph();

protected:
    Q_OBJECT_BINDABLE_PROPERTY(AbstractSaver, QString, _options_file_name)
    Q_OBJECT_BINDABLE_PROPERTY(AbstractSaver, graph_data*, _graph)

    virtual void save_logic(QFile& file) = 0;
    virtual void options_loading_logic(QFile& file);

   	void recalculate_results();
    void add_result_to_option(const QString& _key, const QString& _result);

	std::unique_ptr<QMap<QString, QString>> _elements;
};

#endif // ABSTRACTSAVER_HPP
