#include "tableformer.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

TableFormer::TableFormer(QObject* parent) : AbstractTableFormer(parent)
{
    _is_has_index = false;
    _is_has_columns = false;
}

TableFormer::~TableFormer()
{ }

TableFormer::TableFormer(const TableFormer& other) : AbstractTableFormer(other)
{ }

void TableFormer::set_columns_without_names(int count)
{
    log("Creating numeric columns...");
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
    log("Set named columns...");
    for (std::string column_name: row_list) {
        boost::algorithm::trim(column_name);

        if (!column_name.empty()) {
            log_debug("Creating column with name: " + column_name);
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
    log((boost::format("Adding standart row with index %d") % row_index).str());
    std::vector<std::string> col_names = get_columns_names();
    std::list<std::string>::iterator value_to_row = row_list.begin();

    for (std::string& name : col_names) {
        log_debug((boost::format("Adding for column \"%s\"") % name).str());
        int value_ = 0;

        if (value_to_row != row_list.end())
        {
            value_ = std::stoi(*value_to_row);
            value_to_row++;
        }

        log((boost::format("Setting value %d") % value_).str());
        if (_creating_table.get_column<int>(name.c_str()).size() > row_index) {
            _creating_table.get_column<int>(name.c_str())[row_index] = value_;
        }
        else {
            _creating_table.get_column<int>(name.c_str()).push_back(value_);
        }
    };
}

void TableFormer::add_row(std::string row_index, std::list<std::string> row_list)
{
    int row_index_ = get_index_num(row_index);

    if (row_index_ < 0)
    {
        _creating_table.append_index(row_index);
        row_index_ = get_index_num(row_index);
        log_debug((boost::format("New index (%s; %d)") % row_index % row_index_).str());
    }

    log((boost::format("Index str: %s. Numeric index: %d.") % row_index % row_index_).str());
    add_row(row_index_, row_list);
}

void TableFormer::add_autoindexed_row(std::list<std::string> row_list)
{
    log("Add autoindexed row...");
    std::string row_index_ = row_list.front();
    row_list.pop_front();

    add_row(row_index_, row_list);

}

void TableFormer::remove_row(int row_num)
{
    log("Removing row with num " + std::to_string(row_num));
    std::vector<std::string> _cols = get_columns_names();
    for (std::string& col : _cols)
    {
        _creating_table.get_column<int>(col.data()).pop_back();
    }
}

void TableFormer::remove_row(std::string name)
{
    int index = get_index_num(name);

    remove_row(index);
}

void TableFormer::remove_column(int column_num)
{
    log("Removing column with num #" + std::to_string(column_num));
    _creating_table.remove_column<int>(column_num);
}

void TableFormer::remove_column(std::string name)
{
    log("Removing node with name " + name);
    _creating_table.remove_column<int>(name.c_str());
}

std::unique_ptr<AbstractTableFormer> TableFormer::clone()
{
    return std::make_unique<TableFormer>(*this);
}

TableFormer& TableFormer::operator=(const TableFormer& other)
{
    _is_has_index = other._is_has_index;
    _is_has_columns = other._is_has_columns;

    _creating_table = other._creating_table;

    return *this;
}