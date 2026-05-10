#include "tablereader.hpp"


TableReader::TableReader(QObject *parent)
    : QObject{parent}
{

}

QString TableReader::parse_file(QString file_path)
{
    return QString();
}