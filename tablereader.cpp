#include "tablereader.hpp"

#include <boost/algorithm/string.hpp>

TableReader::TableReader(QObject *parent)
    : QObject{parent}
{ }

void TableReader::parse_file(QString file_path)
{
    std::string filename = file_path.toStdString();

    _table_preprocessor.open(filename);

    if (_table_preprocessor.validate_file())
    {
        TableFormer _table_former;

        bool is_first_line = true;
        while (_table_preprocessor.is_eof())
        {
            std::list<std::string> _line = _table_preprocessor.read_line();

            if (_line.size() > 1)
            {
                if (is_first_line && _is_header_need) {
                    is_first_line = false;
                }
                else {
                    if (_is_index_need) {
                        _table_former.add_autoindexed_row(_line);
                    }
                    else
                    {
                        int last_new_row_index = _table_former.get_new_row_num();

                        _table_former.add_row(last_new_row_index, _line);
                    }
                }
            }
            else
            {
                std::string row_value = _line.front();
                boost::algorithm::trim(row_value);

                if (row_value == "end" || row_value == "e")
                {
                    break;
                }
            }
        }

        _table_preprocessor.close();
    }
    else
    {
        _table_preprocessor.close();
    }

    return;
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
