#include "abstracttableformer.hpp"

#include <boost/format.hpp>

AbstractTableFormer::AbstractTableFormer(QObject* parent) : QObject(parent)
{ }

AbstractTableFormer::AbstractTableFormer(const AbstractTableFormer& other) : _is_has_index(other._is_has_index),
    _is_has_columns(other._is_has_columns), _creating_table(other._creating_table)
{ }

AbstractTableFormer::~AbstractTableFormer()
{ }



int AbstractTableFormer::get_index_num(std::string index_str)
{
    std::vector<std::string> indexes = _creating_table.get_index();
    std::vector<std::string>::iterator finded_index = std::find(indexes.begin(), indexes.end(), index_str);

    if (finded_index != indexes.end())
    {
        return finded_index - indexes.begin();
    }

    return -1;
}

std::vector<std::string> AbstractTableFormer::get_columns_names()
{
    auto col_info = _creating_table.get_columns_info<std::string, int>();
    std::vector<std::string> _output;

    std::transform(col_info.begin(), col_info.end(), std::back_inserter(_output), [](auto item){
        std::string name = std::get<0>(item).data();

        return name;
    });

    return _output;
}

hmdf::Matrix<int> AbstractTableFormer::to_matrix()
{
    hmdf::Matrix<int> matrix = _creating_table.get_matrix<int>();

    log("Transforming dataframe to matrix...");
    for (int i = 0; i < matrix.rows(); i++)
	{
        for (int j = 0; j < matrix.cols(); j++)
        {
            log((boost::format("Value get (%d; %d) = %d") % i % j % matrix(i, j)).str());
        }
    }

    log("Matrix ready...");
    return matrix;
}

std::vector<std::string> AbstractTableFormer::get_row_indexes()
{
    return _creating_table.get_index();
}

std::optional<std::string> AbstractTableFormer::set_description_for_node(int node_num, std::string description)
{
    std::string _old_description;
    if (_descriptions.contains(node_num))
    {
		_old_description = _descriptions[node_num];
    }

	_descriptions[node_num] = description;

    log("Added description for node #" + std::to_string(node_num) + " text: " + description);
    return (_old_description.empty()) ? std::nullopt : std::optional<std::string>(_old_description);
}

void AbstractTableFormer::remove_description(int node_num)
{
	if (_descriptions.contains(node_num))
    {
        _descriptions.erase(node_num);
    }
}

std::optional<std::string> AbstractTableFormer::get_description_by_node_num(int node_num)
{
    if (_descriptions.contains(node_num)) {
		return _descriptions[node_num];
    }

    return std::nullopt;
}

std::map<int, std::string> AbstractTableFormer::get_descriptions()
{
	return _descriptions;
}

int AbstractTableFormer::get_last_row_num()
{
    int last_index = _creating_table.get_index().size() - 1;

    return last_index;
}

int AbstractTableFormer::get_new_row_num()
{
    if (!_creating_table.empty()) {
		std::string idx = _creating_table.col_idx_to_name(0);

		return _creating_table.get_column<int>(idx.data()).size();
    }

    return _creating_table.get_index().size();
}


void AbstractTableFormer::clear()
{
    _creating_table.clear();
    _descriptions.clear();

    _is_has_index = false;
    _is_has_columns = false;
}