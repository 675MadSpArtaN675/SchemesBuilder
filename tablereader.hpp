#ifndef TABLEREADER_HPP
#define TABLEREADER_HPP

#include <memory>

#include <QObject>
#include <QBindable>
#include <QList>
#include <QUrl>
#include <QString>
#include <QRegularExpression>

#include <DataFrame/DataFrame.h>

#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>

#include "abstracttableformer.hpp"
#include "tablepreprocessor.hpp"

#include "CoreLogger.hpp"

namespace lg = boost::log;




class TableReader : public QObject, protected CoreLogger
{
    Q_OBJECT

    Q_PROPERTY(QString delimiter READ get_delimiter WRITE set_delimiter)

    Q_PROPERTY(bool is_header_need READ is_header_need WRITE set_header_need)
    Q_PROPERTY(bool is_index_need READ is_index_need WRITE set_index_need)
    Q_PROPERTY(QList<QList<int>> table READ get_table BINDABLE bindable_table)
    Q_PROPERTY(QList<QString> titles READ get_titles BINDABLE bindable_titles)

    Q_PROPERTY(QObject* former_of_graph_table READ table_former WRITE set_table_former)

    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TableReader, QList<QList<int>>, _table_bind, QList<QList<int>>())
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TableReader, QList<QString>, _titles, QList<QString>())
public:
	enum class FormerEnum {
		Standart,
		DatabasedType
	};
	Q_ENUM(FormerEnum)

    explicit TableReader(QObject *parent = nullptr);
    ~TableReader();

    Q_INVOKABLE void parse_file(QString file_path);
    Q_INVOKABLE void parse_file(QUrl file_path);
    Q_INVOKABLE void switch_former(FormerEnum number);
	Q_INVOKABLE QString get_description_of_node(int node_num);
    Q_INVOKABLE void clear();

    QList<QList<int>> get_table();
    QBindable<QList<QList<int>>> bindable_table();

    QList<QString> get_titles();
    QBindable<QList<QString>> bindable_titles();

    void set_delimiter(QString delimiter);
    QString get_delimiter();

    void set_header_need(bool value);
    bool is_header_need();

    void set_index_need(bool value);
    bool is_index_need();

    QObject* table_former() const;
    void set_table_former(QObject* newTable_former);

protected:
    void processor_init();
    void parse_file_cycle(const QString& file_path);

    QString _delimiter;
    QString _last_file;

    bool _is_header_need, _is_index_need, _is_headers_blocked, _is_index_blocked;

    std::unique_ptr<TablePreprocessor> _table_preprocessor;
    std::unique_ptr<AbstractTableFormer> _table_former;
};

#endif // TABLEREADER_HPP
