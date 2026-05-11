#include "table_former.hpp"

#include <boost/algorithm/string.hpp>

table_former::table_former()
{
    _start_row_num_index = 0;
    _is_has_index = false;
    _is_has_columns = false;
}

table_former::~table_former()
{

}

void table_former::set_columns_without_names(int count)
{
    for (int i = 0; i < count; i++)
    {
        _creating_table.create_column<int>(std::to_string(i).data());
    }
}

void table_former::set_columns_names(std::list<std::string> row_list)
{
    for (std::string column_name: row_list) {
        boost::algorithm::trim(column_name);

        if (!column_name.empty()) {
            _creating_table.create_column<int>(column_name.data());
        }
    }

    _is_has_columns = true;
}

void table_former::set_index_names(std::list<std::string> row_list)
{
    for (std::string index_name: row_list)
    {
        boost::algorithm::trim(index_name);

        if (!index_name.empty()) {
            _creating_table.append_index(index_name);
        }
    }

    _is_has_index = true;
}

void table_former::add_row(std::list<std::string> row_list)
{
    std::string _index;
    std::vector<std::string> row(row_list.begin(), row_list.end());

    if (_is_has_index)
    {
        _index = row_list.front();
        row_list.pop_front();

        _creating_table.append_index(_index);
    }

    int row_index = _start_row_num_index;
    if (!_index.empty()) {
        std::vector<std::string> indexes_rows = get_row_indexes();
        auto item = std::find(indexes_rows.begin(), indexes_rows.end(), _index);

        if (item != indexes_rows.end())
        {
            row_index = item - indexes_rows.begin();
        }
    }

    std::vector<std::string> col_names = get_columns_names();
    std::list<std::string>::iterator value_to_row = row_list.begin();

    for (std::string name : col_names) {
        int value_ = std::stoi(*value_to_row);

        _creating_table.get_column<int>(name.c_str())[row_index] = value_;

        if (value_to_row != row_list.end())
        {
           value_to_row++;
        }
    };

    _start_row_num_index++;
}

std::vector<std::string> table_former::get_columns_names()
{
    auto col_info = _creating_table.get_columns_info<std::string, int>();
    std::vector<std::string> _output;

    std::transform(col_info.begin(), col_info.end(), std::back_inserter(_output), [](auto item){
        std::string name = std::get<0>(item).data();

        return name;
    });

    return _output;
}

std::vector<std::string> table_former::get_row_indexes()
{
    return _creating_table.get_index();
}

hmdf::Matrix<int> table_former::to_matrix()
{
    return _creating_table.get_matrix<int>();
}