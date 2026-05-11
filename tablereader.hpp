#ifndef TABLEREADER_HPP
#define TABLEREADER_HPP

#include <QObject>
#include <QList>

#include "tableformer.hpp"
#include "tablepreprocessor.hpp"


class TableReader : public QObject
{
    Q_OBJECT

public:
    explicit TableReader(QObject *parent = nullptr);

    void parse_file(QString file_path);

protected:
    QString _last_file;

};

#endif // TABLEREADER_HPP
