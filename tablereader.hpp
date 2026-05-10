#ifndef TABLEREADER_HPP
#define TABLEREADER_HPP

#include <QObject>

class TableReader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString last_parsed_file READ _last_file)

public:
    explicit TableReader(QObject *parent = nullptr);

    QString parse_file(QString file_path);

protected:
    QString _last_file;
};

#endif // TABLEREADER_HPP
