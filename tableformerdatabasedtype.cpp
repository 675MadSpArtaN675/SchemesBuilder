#include "tableformerdatabasedtype.hpp"

namespace al = boost::algorithm;

TableFormerDatabasedType::TableFormerDatabasedType(QObject* parent) : TableFormer(parent)
{ }

TableFormerDatabasedType::TableFormerDatabasedType(const TableFormerDatabasedType &other) : TableFormer(other)
{ }

TableFormerDatabasedType::~TableFormerDatabasedType()
{ }

void TableFormerDatabasedType::add_row(int index, std::list<std::string> _row_values)
{
    int size = _row_values.size();
    log((boost::format("Adding node with num %d. Line size: %d") % index % size).str());

    std::list<std::string>::iterator _elements = _row_values.begin();
    std::string _node_name = *_elements;
    log("Node name: " + _node_name);

    _creating_table.create_column<int>(_node_name.c_str());
	_creating_table.append_index(_node_name);
    fill_table();

    if (size > 1) {
        log("Found description...");
		_elements++;

        std::string _descr = *_elements;

		_descriptions[index] = _descr;
    }

    if (size > 2) {
        log("Found links...");
        _elements++;

        std::string _links_line = *_elements;
        std::list<int> _links_of_node;

        for (auto i = al::make_split_iterator(_links_line, boost::algorithm::first_finder(","));
             i != al::split_iterator<std::string::iterator>(); i++)
        {
        	std::string _parted_list = boost::copy_range<std::string>(*i);
            int _index = -1;

            try {
                log_debug("Casting: " + _parted_list);
                _index = boost::lexical_cast<int>(_parted_list) - 1;
                _links_of_node.push_back(_index);

                log((boost::format("Index %d was linked to %d") % _index % index).str());
            }
            catch (const boost::bad_lexical_cast& _error)
            {
                log((boost::format("Error of cast \'%s\' to link. Traceback: \'%s\'.") % _parted_list % _error.what()).str());
            }
        }

		_links[index] = _links_of_node;
    }

    log((boost::format("Node %d was created") % index).str());
}

hmdf::Matrix<int> TableFormerDatabasedType::to_matrix()
{
    log("Converting table to matrix");
    hmdf::Matrix<int> _matrix = _creating_table.get_matrix<int>();

    for (int i = 0; i < _matrix.rows(); i++)
	{
        std::stringstream value_rows("Row:\n");
        for (int j = 0; j < _matrix.cols(); j++)
        {
            value_rows << (boost::format("\tValue in (%d; %d) = %d\n") % i % j % _matrix(i, j)).str();
        }

        log(value_rows.str());

        if (_links.contains(i)) {
			for (const int& index : _links[i])
			{
                log((boost::format("Linking %d to %d") % i % index).str());
				_matrix[index, i] = 1;
			}
        }

    }

    log("Matrix ready!");
    return _matrix;
}

std::unique_ptr<AbstractTableFormer> TableFormerDatabasedType::clone()
{
	return std::make_unique<TableFormerDatabasedType>(*this);
}

void TableFormerDatabasedType::fill_table()
{
	int x_size = _creating_table.shape().first, y_size = _creating_table.shape().second;
	log((boost::format("Table size: %dx%d") % x_size % y_size).str());

    for (int i = 0; i < x_size; i++)
    {
        std::vector<int>& _col = _creating_table.get_column<int>(i);
        int col_real_size = _col.size();
        log_debug("Column real size: " + std::to_string(col_real_size));

        if (col_real_size < y_size)
        {
            int dx = y_size - col_real_size;
            log_debug("Adding empty elements... Empty elements: " + std::to_string(dx));
            for (int j = 0; j < dx; j++)
            {
                _col.push_back(0);
            }
        }
        else if (col_real_size > y_size)
        {
            log_debug("Resizing table..");
            _col.resize(y_size);
        }
    }
}