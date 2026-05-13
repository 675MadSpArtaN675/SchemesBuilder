#include "tableformer.hpp"

#include <boost/algorithm/string.hpp>

TableFormer::TableFormer()
{
    _is_has_index = false;
    _is_has_columns = false;
}

TableFormer::~TableFormer()
{ }

TableFormer::TableFormer(const TableFormer& other)
{
    _is_has_index = other._is_has_index;
    _is_has_columns = other._is_has_columns;

    _creating_table = other._creating_table;
}

void TableFormer::set_columns_without_names(int count)
{
    for (int i = 0; i < count; i++)
    {
        _creating_table.create_column<int>(std::to_string(i).data());
    }
}

void TableFormer::set_index_without_names(int count)
{
    for (int i = 0; i < count; i++)
    {
        _creating_table.append_index(std::to_string(i).data());
    }
}

void TableFormer::set_columns_names(std::list<std::string> row_list)
{
    for (std::string column_name: row_list) {
        boost::algorithm::trim(column_name);

        if (!column_name.empty()) {
            _creating_table.create_column<int>(column_name.data());
        }
    }

    _is_has_columns = true;
}

void TableFormer::set_index_names(std::list<std::string> row_list)
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


void TableFormer::add_row(int row_index, std::list<std::string> row_list)
{
    std::vector<std::string> col_names = get_columns_names();
    std::list<std::string>::iterator value_to_row = row_list.begin();

    for (std::string name : col_names) {
        int value_ = 0;

        if (value_to_row != row_list.end())
        {
            value_ = std::stoi(*value_to_row);
            value_to_row++;
        }

        _creating_table.get_column<int>(name.c_str())[row_index] = value_;
    };
}

void TableFormer::add_row(std::string row_index, std::list<std::string> row_list)
{
    int row_index_ = get_index_num(row_index);

    if (row_index_ < 0)
    {
        _creating_table.append_index(row_index);

        row_index_ = get_index_num(row_index);
    }

    add_row(row_index_, row_list);
}

void TableFormer::add_autoindexed_row(std::list<std::string> row_list)
{
    std::string row_index_ = row_list.front();
    row_list.pop_front();

    int row_index = get_index_num(row_index_);

    add_row(row_index_, row_list);

}

int TableFormer::get_last_row_num()
{
    int last_index = _creating_table.get_index().size() - 1;

    return last_index;
}

int TableFormer::get_new_row_num()
{
    return _creating_table.get_index().size();
}

int TableFormer::get_index_num(std::string index_str)
{
    std::vector<std::string> indexes = _creating_table.get_index();
    std::vector<std::string>::iterator finded_index = std::find(indexes.begin(), indexes.end(), index_str);

    if (finded_index != indexes.end())
    {
        return finded_index - indexes.begin();
    }

    return 0;
}

std::vector<std::string> TableFormer::get_columns_names()
{
    auto col_info = _creating_table.get_columns_info<std::string, int>();
    std::vector<std::string> _output;

    std::transform(col_info.begin(), col_info.end(), std::back_inserter(_output), [](auto item){
        std::string name = std::get<0>(item).data();

        return name;
    });

    return _output;
}


void TableFormer::clear()
{
    _creating_table.clear();

    _is_has_index = false;
    _is_has_columns = false;
}

hmdf::Matrix<int> TableFormer::to_matrix()
{
    return _creating_table.get_matrix<int>();
}

TableFormer& TableFormer::operator=(const TableFormer& other)
{
    _is_has_index = other._is_has_index;
    _is_has_columns = other._is_has_columns;

    _creating_table = other._creating_table;

    return *this;
}