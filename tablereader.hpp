#ifndef TABLEREADER_HPP
#define TABLEREADER_HPP

#include <QObject>
#include <QList>
#include <QUrl>
#include <QString>
#include <QRegularExpression>
#include <DataFrame/DataFrame.h>

#include "tableformer.hpp"
#include "tablepreprocessor.hpp"


class TableReader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString delimiter READ get_delimiter WRITE set_delimiter)

    Q_PROPERTY(bool is_header_need READ is_header_need WRITE set_header_need)
    Q_PROPERTY(bool is_index_need READ is_index_need WRITE set_index_need)

public:
    explicit TableReader(QObject *parent = nullptr);

    Q_INVOKABLE QList<QList<int>> parse_file(QString file_path);
    Q_INVOKABLE QList<QList<int>> parse_file(QUrl file_path);
    Q_INVOKABLE QList<QList<int>> get_table();

    void set_delimiter(QString delimiter);
    QString get_delimiter();

    void set_header_need(bool value);
    bool is_header_need();

    void set_index_need(bool value);
    bool is_index_need();

protected:
    QString _delimiter;
    QString _last_file;

    bool _is_header_need, _is_index_need;

    TablePreprocessor _table_preprocessor;
    TableFormer _table_former;
};

#endif // TABLEREADER_HPP
