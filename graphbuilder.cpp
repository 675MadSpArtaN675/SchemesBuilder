#include "graphbuilder.hpp"

#include <algorithm>
#include <locale>

#include <boost/format.hpp>
#include <boost/lambda2.hpp>


GraphBuilder::GraphBuilder(QObject* parent) : GraphBuilder(STANDART_GRAPH_NAME, parent)
{ }

GraphBuilder::GraphBuilder(QString name, QObject* parent) : QObject(parent), _name{name}, CoreLogger()
{ }

GraphBuilder::GraphBuilder(const GraphBuilder &other_builder)
{
    _name = other_builder._name;
    _nodes_links = other_builder._nodes_links;
    _temp_nodes = other_builder._temp_nodes;
}

GraphBuilder::GraphBuilder(GraphBuilder &&other_builder)
{
    _name = std::move(other_builder._name);
    _nodes_links = std::move(other_builder._nodes_links);
    _temp_nodes = std::move(other_builder._temp_nodes);
}

GraphBuilder::~GraphBuilder()
{
    _cache.reset();
}

GraphBuilder &GraphBuilder::create_nodes_from_matrix(int_hmdf_matrix &_matrix, QList<QString> names)
{
    unsigned int max_index = get_max_index();

    int size_rows = _matrix.rows();
    int size_columns = _matrix.cols();

    if (size_rows != size_columns)
    {
        throw std::logic_error("Vector is not square!");
    }

    for (int i = 0; i < size_rows; i++)
    {
        _matrix(i, i) = 0;
    }

    for (int i = 0; i < size_rows; i++)
    {
        QString name = !names.isEmpty() ? names[i] : QString::fromStdString(std::to_string(max_index + 1 + i));
        GraphNode _node(max_index + 1 + i, name);

        for (int j = 0; j < size_columns; j++) {
            if (_matrix(i, j) != 0) {
				connect_node_to_node(i, j);
            }
        }

		set_additional_data(_node);
        _temp_nodes.insert(i, std::move(_node));
    }

    return *this;
}

GraphBuilder &GraphBuilder::create_nodes_from_matrix(vector2d &_matrix, QList<QString> names)
{
    int size_rows = _matrix.size();
    int size_columns = _matrix[0].size();

    log((boost::format("Input vector with size %d x %d.") % size_rows % size_columns).str());
    if (size_rows != size_columns)
    {
        log("Error: row size is not equal to columns size");
        throw std::logic_error("Vector is not square!");
    }

    log("Creating graph...");
    for (int i = 0; i < size_rows; i++)
    {
        _matrix[i][i] = 0;
    }

    unsigned int max_index = get_max_index();
    for (int i = 0; i < size_rows; i++)
    {
        QString name = names.size() > i ? names[i] : QString::fromStdString(std::to_string(max_index + i));
        GraphNode _node(max_index + i, name);

        log((boost::format("Creating node %d with name: %s") % (max_index + i) % name.toStdString()).str());

		set_additional_data(_node);

        QString descr = _node.additional_data().value<NodePaintOptions>().description();
        log("Description setted: " + descr.toStdString());

        _temp_nodes.insert(max_index + i, std::move(_node));

    }

    for (int i = 0; i < size_rows; i++) {
        for (int j = 0; j < size_columns; j++)
        {
            if (i != j && _matrix[i][j] != 0) {
                connect_node_to_node(max_index + i, max_index + j);
            }
        }
    }

    return *this;
}

GraphBuilder &GraphBuilder::create_nodes_from_matrix(QList<QList<int>> _matrix, QList<QString> names)
{
    log((boost::format("QList matrix input size %d x %d. Names count %d") % _matrix.size() % _matrix[0].size() % names.size()).str());

    vector2d _transformed_lists;
    std::transform(_matrix.begin(), _matrix.end(), std::back_inserter(_transformed_lists), [this](QList<int>& _row){
        log("Transforming row with len: " + std::to_string(_row.size()));
        std::vector<int> transformed_list(_row.begin(), _row.end());

        std::stringstream _log_line;
        std::for_each(transformed_list.begin(), transformed_list.end(), _log_line << boost::lambda2::_1 << ' ');
        _log_line << std::endl;

        log_debug("Line: " + _log_line.str());
        return transformed_list;
    });

    log("Creating nodes from vector!");
    return create_nodes_from_matrix(_transformed_lists, names);
}

GraphBuilder &GraphBuilder::create_nodes_from_preprocessor(TableFormer* _matrix)
{
    std::vector<std::string> graph_names = _matrix->get_columns_names();
    hmdf::Matrix<int> _graph_matrix = _matrix->to_matrix();

    QList<QString> graph_names_performed;
    std::transform(graph_names.begin(), graph_names.end(), std::back_inserter(graph_names_performed), [](std::string& names){
        return QString::fromStdString(names);
    });

    create_nodes_from_matrix(_graph_matrix, graph_names_performed);

    return *this;
}

GraphBuilder &GraphBuilder::get_nodes_from_other_graph(const graph_data &_other_graph)
{
    QList<unsigned int> _nodes_numbers = _other_graph.get_nodes_numbers();
    unsigned int max_index = get_max_index();

    QList<GraphNode> _nodes;
    for (unsigned int& node_num : _nodes_numbers) {
        QSharedPointer<GraphNode> _node = _other_graph.get_vertex(node_num);
        GraphNode _node_copy = GraphNode(*_node);

        _node_copy.set_number(max_index + 1 + _node_copy.get_number());

        QList<unsigned int> connected_nodes_nums = _node_copy.get_connected_nodes_nums();
        std::for_each(connected_nodes_nums.begin(), connected_nodes_nums.end(), [&max_index](unsigned int& value){ value += 1 + max_index; });
        _node_copy.unconnect_all_nodes();

        _nodes.push_back(std::move(_node_copy));
    }

    return *this;
}

GraphBuilder &GraphBuilder::add_node(GraphNode _node)
{
    int node_num = _node.get_number();
    log("Add node #" + std::to_string(node_num));

	set_additional_data(_node);

    if (_temp_nodes.contains(node_num))
    {
        int new_index = get_min_free_index(_temp_nodes.keys());
        _temp_nodes.insert(new_index, std::move(_node));

        return *this;
    }

    _temp_nodes.insert(node_num, std::move(_node));

    return *this;
}

void GraphBuilder::set_additional_data(GraphNode& _node)
{
    int node_num = _node.get_number();

    if (_reader) {
		QString description = _reader->get_description_of_node(node_num);
		if (!description.isEmpty()) {
			log_debug("Description found...");
			QVariant _options = _node.additional_data();
			NodePaintOptions _options_obj;

			if (!_options.isNull()) {
				_options_obj = _options.value<NodePaintOptions>();
			}

			log_debug("Description text: " + description.toStdString());
			_options_obj.setDescription(description);
			_options.setValue(_options_obj);

			_node.set_additional_data(QVariant::fromValue(_options));
		}
    }
}

GraphBuilder &GraphBuilder::add_node(int node_num, QString name)
{
    if (node_num <= 0)
    {
        node_num = 1;
    }

    add_node(GraphNode(node_num, name));

    return *this;
}

GraphBuilder &GraphBuilder::connect_node_to_node(unsigned int first_node_num, unsigned int second_node_num)
{
    if (_nodes_links.contains(first_node_num))
    {
        log((boost::format("Node #%d linked to #%d node in builder.") % second_node_num % first_node_num).str());
        _nodes_links[first_node_num].insert(second_node_num);
    }
    else
    {
        log((boost::format("Node #%d linked to #%d node. It is first node in list") % second_node_num % first_node_num).str());
        _nodes_links[first_node_num] = QSet<unsigned int>{second_node_num};
    }

    return *this;
}

GraphBuilder& GraphBuilder::connect_all_vertexes_by_string(QString line, QJSValue table, bool is_delete_old_links)
{
	std::string pre_performed_line = boost::algorithm::trim_copy(line.replace("\n", " ").toStdString());
    log((boost::format("Input line: \'%s\'") % pre_performed_line).str());

    if (pre_performed_line.empty())
    {
        log_debug("Line is empty!");
    	return *this;
    }

    if (is_delete_old_links)
    {
        log_debug("Clearing old links...");
        _nodes_links.clear();
    }

	boost::regex _pattern("(\\d+(?:\\s*-?>\\s*\\d+\\s*)*)", boost::regex::perl);
    for (auto i = boost::make_regex_iterator(pre_performed_line, _pattern, boost::regex_constants::match_extra); i != boost::sregex_iterator(); i++)
    {
        if (!i->empty())
        {
			boost::smatch _match = *i;
			std::string line = _match[0];
            boost::algorithm::trim(line);

            log_debug("Match line: " + line);

            if (!line.empty()) {
                log_debug("Removing spaces...");
                try {
					std::string::iterator removed_spaces = std::remove_if(line.begin(),
line.end(), [](unsigned char x){ return std::isspace(x); });

					line.erase(removed_spaces, line.end());

                    line_partition(line, table);
                }
                catch (const std::exception& error)
                {
                    log_error(std::string("Error of deleting spaces: ") + error.what());
                }
            }
        }
    }

    return *this;
}

void GraphBuilder::line_partition(std::string& _line, QJSValue& table)
{
	log_debug("No space line ready: " + _line);

    boost::regex _split_pattern("(-?>)|,");
	std::list<std::string> _parts;
	boost::iter_split(_parts, _line, boost::regex_finder(_split_pattern));

	unsigned int node_prev = 0;
	for (std::string& item : _parts)
	{
		boost::algorithm::trim(item);

		if (!item.empty())
		{
			unsigned int node = std::stoi(item);
			log_debug("Parsed node num: " + item);

			if (node_prev > 0 && node > 0)
			{
				log_debug((boost::format("Connect node #%d with #%d from string") % node_prev % node).str());
				connect_node_to_node(node_prev, node);

				if (table.isArray()) {
					QJSValue row = table.property(node_prev - 1);

					if (row.isArray())
					{
						row.setProperty(node - 1, 1);
					}
				}
			}

			node_prev = node;
		}
	}
}

GraphBuilder &GraphBuilder::unconnect_node_from_node(unsigned int first_node_num, unsigned int second_node_num)
{
    if (_nodes_links.contains(first_node_num))
    {
        _nodes_links[first_node_num].remove(second_node_num);
    }

    return *this;
}

GraphBuilder &GraphBuilder::remove_node(unsigned int num)
{
    log("Deleting node #" + std::to_string(num));
    if (_temp_nodes.contains(num))
    {
        _temp_nodes.remove(num);
    }
    else
    {
        boost::format exception_format_str = boost::format("Node %d is not exists") % num;
        throw std::logic_error(exception_format_str.str());
    }

    if (_nodes_links.contains(num))
    {
        _nodes_links.remove(num);
    }

    return *this;
}

bool GraphBuilder::is_empty()
{
    if (!_cache)
    {
        log("Cache is empty!");
    }

    if (_temp_nodes.empty())
    {
        log("No nodes");
    }

    return !_cache && _temp_nodes.empty();
}

GraphBuilder &GraphBuilder::clear()
{
    _temp_nodes.clear();
    _nodes_links.clear();

    return *this;
}

void GraphBuilder::save_graph(QString file, SaverTypes _type)
{
    log((boost::format("File: %s. Type: %s") % file.toStdString() % std::to_string(static_cast<int>(_type))).str());
    if (_type > SaverTypes::None && !file.isEmpty())
    {
        log_debug("Performing file...");
		if (_type != _saver_cached_type) {
            log_debug("Choosing type...");
			switch(_type)
			{
				case SaverTypes::DrawIO:
					_saver.reset(new DrawioSaver());
					break;

				default:
					break;
			}

            if (_cache) {
				_saver->set_graph(_cache.get());
            }

			_saver->load_options("drawio_options.dat");
			_saver_cached_type = _type;
		}

        log_debug("Saving...");
        if (_saver && _saver->is_ready()) {
			QString file_name_ready = remove_prefix_for_resource_path(file);
            log_debug("Saving to: " + file_name_ready.toStdString());

			_saver->save(file);
        }
    }
}

graph_data* GraphBuilder::build_ptr()
{
    if (_temp_nodes.size() > 0) {
        graph_data* _data = new graph_data(_name);
        if (_temp_nodes.contains(0))
        {
            _temp_nodes.remove(0);
        }

        QList<GraphNode> _nodes = _temp_nodes.values();
        QList<unsigned int> _indexes = _nodes_links.keys();

        log("Creating nodes...");
        for (GraphNode& node_key : _nodes)
        {
            int num = node_key.get_number();
            log((boost::format("Creating node #%d for graph %s") % num % _name.toStdString()).str());

            _data->add_vertex(std::move(node_key));
        }

        log("Connecting nodes...");
        for (unsigned int& node_num : _indexes) {
            QList<unsigned int> _connected_vertexes = _nodes_links[node_num].values();

            std::stringstream _log_vertexes;
            _log_vertexes << std::string("Vertexes connected to ") + std::to_string(node_num) + ": ";

            if (!_connected_vertexes.empty()) {
				for (unsigned int& vertex : _connected_vertexes)
				{
					_log_vertexes << vertex << ", ";
				}

				_data->connect_vertexes_to_vertex(node_num, _connected_vertexes);
            }
            else {
                _log_vertexes << "None";
            }

            log_debug(_log_vertexes.str());
        }

        _cache.reset(_data);
        return _data;
    }

    throw std::runtime_error("Graph has no any nodes!");
}

graph_data GraphBuilder::build()
{
	return *build_ptr();
}

GraphBuilder &GraphBuilder::operator=(const GraphBuilder &_other)
{
    _name = _other._name;
    _nodes_links = _other._nodes_links;
    _temp_nodes = _other._temp_nodes;

    return *this;
}

GraphBuilder& GraphBuilder::operator=(const GraphBuilder&& _other)
{
    _name = std::move(_other._name);
    _nodes_links = std::move(_other._nodes_links);
    _temp_nodes = std::move(_other._temp_nodes);

    return *this;
}

QString GraphBuilder::name() const
{
    return _name;
}

void GraphBuilder::setName(const QString &newName)
{
    _name = newName;
}

void GraphBuilder::set_reader(TableReader *new_reader)
{
    _reader = new_reader;
}

TableReader *GraphBuilder::reader()
{
	return _reader;
}

graph_data *GraphBuilder::last_created()
{
    return new graph_data(*_cache);
}

unsigned int GraphBuilder::get_max_index()
{
    QList<unsigned int> builder_nodes = _temp_nodes.keys();

    if (builder_nodes.empty())
    {
        return 1;
    }
    return *std::max_element(builder_nodes.begin(), builder_nodes.end());
}
