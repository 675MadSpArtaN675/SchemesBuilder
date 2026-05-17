#include "tablereader.hpp"

#include "DataFrame/Utils/Matrix.h"

#include <boost/algorithm/string.hpp>

TableReader::TableReader(QObject *parent)
    : QObject{parent}, CoreLogger()
{ }


void TableReader::parse_file(QString file_path)
{
    std::string _delim = _delimiter.toStdString();
    if (!_table_preprocessor
            || _table_preprocessor->delimiter() != _delim
            || _table_preprocessor->is_header_has() != _is_header_need
            || _table_preprocessor->is_index_has() != _is_index_need)
    {
        _table_preprocessor.reset(new TablePreprocessor(_is_header_need, _is_index_need, _delim));
    }

    std::string filename = file_path.toStdString();

    _table_preprocessor->open(filename);

    if (_table_preprocessor->validate_file())
    {
        std::cout << "Парсим файл: " << filename << std::endl;
        _table_former->clear();

        bool is_first_line = true;
        while (!_table_preprocessor->is_eof())
        {
            std::list<std::string> _line = _table_preprocessor->read_line();
            std::cout << "Длина считанной линии: " << _line.size() << std::endl;

            if (_line.size() > 1)
            {
                if (is_first_line) {
                    _table_former->set_columns_without_names(_line.size());
                    is_first_line = false;
                }

                if (_is_index_need) {
                    _table_former->add_autoindexed_row(_line);
                }
                else
                {
                    int last_new_row_index = _table_former->get_new_row_num();
                    std::cout << "Индекс: " << last_new_row_index << std::endl;
                    _table_former->add_row(last_new_row_index, _line);
                }
            }
            else
            {
                std::string row_value = _line.front();
                std::cout << "Значение: " << row_value << std::endl;
                boost::algorithm::trim(row_value);

                if (row_value == "end" || row_value == "e")
                {
                    _table_preprocessor->close();
                    break;
                }
            }
        }

        _last_file = file_path;
    }
    else
    {
        _table_preprocessor->close();
    }
}

void TableReader::parse_file(QUrl file_path)
{
    QString file_path_str = file_path.toString();
    file_path_str = file_path_str.replace(QRegularExpression("^(file|qrc)://"), QString(""));

    parse_file(file_path_str);
}

QList<QList<int>> TableReader::get_table()
{
    if (_table_former) {
        hmdf::Matrix<int> _table = _table_former->to_matrix();
        QList<QList<int>> _result_table_to_qml;

        for (int i = 0; i < _table.rows(); i++)
        {
            _result_table_to_qml.emplaceBack();

            for (int j = 0; j < _table.cols(); j++)
            {
                _result_table_to_qml[i].push_back(_table(i,j));
            }
        }

        _table_bind.setValue(QList<QList<int>>(_result_table_to_qml));
        return _result_table_to_qml;
    }

    return QList<QList<int>>();
}

QBindable<QList<QList<int>> > TableReader::bindable_table() {
    return QBindable<QList<QList<int>>>(&_table_bind);
}

QList<QString> TableReader::get_titles()
{
    if (_table_former) {
        QList<QString> _result_titles;

        std::vector<std::string> _titles_raw = _table_former->get_row_indexes();
        std::transform(_titles_raw.cbegin(), _titles_raw.cend(), std::back_inserter(_result_titles), [](std::string title){ return QString::fromStdString(title); });

        _titles.setValue(QList<QString>(_result_titles));

        return _result_titles;
    }

    return QList<QString>();
}

QBindable<QList<QString> > TableReader::bindable_titles() {
    return QBindable<QList<QString>>(&_titles);
}

void TableReader::set_delimiter(QString delimiter)
{
    _delimiter = delimiter;
}

QString TableReader::get_delimiter()
{
    return _delimiter;
}

void TableReader::set_header_need(bool value)
{
    _is_header_need = value;
}

bool TableReader::is_header_need() {
    return _is_header_need;
}

void TableReader::set_index_need(bool value) {
    _is_index_need = value;
}

bool TableReader::is_index_need() {
    return _is_index_need;
}

TableFormer* TableReader::table_former() const
{
    return _table_former.get();
}

void TableReader::set_table_former(TableFormer* new_table_former)
{
    _table_former.reset(new_table_former);
}
