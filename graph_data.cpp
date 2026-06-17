#include "graph_data.hpp"

#include <QSet>
#include <QQueue>

#include <boost/format.hpp>

graph_data::graph_data() : graph_data(QString("Name"))
{}

graph_data::graph_data(QString name) : _name{name}, CoreLogger()
{
    log(std::string("Created graph with name: ") + name.toStdString());
}

graph_data::graph_data(const graph_data &other)
{
    log(std::string("Copying graph with name: ") + other._name.toStdString());

    _name = other._name;
    _nodes = other._nodes;
}

graph_data::graph_data(const graph_data &&other)
{
    log(std::string("Moving graph with name: ") + other._name.toStdString());

    _name = std::move(other._name);
    _nodes = std::move(other._nodes);
}

graph_data::~graph_data()
{
    log(std::string("Destroying graph - ") + _name.toStdString());
    _logger.reset();
}

void graph_data::add_vertex(unsigned int node_num, QString node_name, QList<unsigned int> connect_to_vertexes)
{
    log((boost::format("Adding node %d with name %s") % node_num % node_name.toStdString()).str());
    if (_nodes.contains(node_num))
    {
        throw std::logic_error("Vertex already exists");
    }

    QSet<int> dublicate_delete(connect_to_vertexes.begin(), connect_to_vertexes.end());

    if (dublicate_delete.contains(node_num))
    {
        dublicate_delete.remove(node_num);
    }
    log((boost::format("Connected vertexes: %s") % join_list(connect_to_vertexes, ", ")).str());

    QSharedPointer<GraphNode> node = QSharedPointer<GraphNode>::create(GraphNode(node_num, node_name));

    for (int node_num_to_connect: dublicate_delete)
    {
        try {
			log("Connecting node #" + std::to_string(node_num_to_connect) + " to node #" + std::to_string(node_num));
			if (!_nodes.contains(node_num_to_connect))
			{
				std::string message = (boost::format("Vertex with num \'%d\' is not exists! You can't connect unexcepted vertex to %d") % node_num_to_connect % node_num).str();
				node.clear();

				throw std::logic_error(message);
			}

			if (_nodes[node_num]) {
				node->connect_node(_nodes[node_num]);
				log("Node #" + std::to_string(node_num_to_connect) + " successfully connected!");
			}
        }
        catch (const std::exception& error)
        {
            log_error(std::string("Error of connecting vertex: ") + error.what());
        }
    }

    _nodes.insert(node_num, QSharedPointer<GraphNode>(node));
}

void graph_data::add_vertex(GraphNode &&_node, QList<unsigned int> connected_vertexes)
{
    int num = _node.get_number();

    log((boost::format("Adding ready node %d with name %s") % num % _node.get_name().toStdString()).str());

    QSharedPointer<GraphNode> _node_ptr = QSharedPointer<GraphNode>::create(std::forward<GraphNode>(_node));

    for (unsigned int& connected_vertex_num : connected_vertexes)
    {
        try {
			log("Connecting node #" + std::to_string(connected_vertex_num) + " to node #" + std::to_string(num));

			if (_nodes.contains(connected_vertex_num)) {
				log("Node #" + std::to_string(connected_vertex_num) + " successfully connected!");
				QSharedPointer<GraphNode> connected_node = get_vertex(connected_vertex_num);

				_node_ptr->connect_node(connected_node.toWeakRef());
			}
        }
        catch (const std::exception& error)
        {
            log_error((boost::format("Failed to connect node #%d to #%d. Error: ") % connected_vertex_num % _node_ptr->get_number()).str() + error.what());
        }
    }

    _nodes.insert(num, _node_ptr);
}

QSharedPointer<GraphNode> graph_data::get_vertex(unsigned int node_num) const
{
    reindex_vertexes_numbers();

    if (_nodes.contains(node_num)) {
        return _nodes[node_num];
    }

    return nullptr;
}

QSharedPointer<GraphNode> graph_data::get_vertex(QString node_name) const
{
    log(std::string("Removing node with name: ") + node_name.toStdString());

    reindex_vertexes_numbers();
    auto finded_obj = std::find_if(_nodes.values().begin(), _nodes.values().end(),
                 [&node_name](const QSharedPointer<GraphNode>& _node){ return _node->get_name() == node_name; });

    if (finded_obj != _nodes.values().end())
    {
        return *finded_obj;
    }

    return nullptr;
}

void graph_data::remove_vertex(unsigned int num)
{
    log("Removing node #" + std::to_string(num));

    reindex_vertexes_numbers();
    if (_nodes.contains(num))
    {
        _nodes.remove(num);
    }
}

void graph_data::remove_vertex(QString node_name)
{
    log("Removing node with name" + node_name.toStdString());

    reindex_vertexes_numbers();
    auto finded_obj = std::find_if(_nodes.values().begin(), _nodes.values().end(),
                 [&node_name](const QSharedPointer<GraphNode>& _node){ return _node->get_name() == node_name; });

    if (finded_obj != _nodes.values().end())
    {
        _nodes.remove((*finded_obj)->get_number());
    }
}

void graph_data::transform_links(QList<QList<int>>& _links)
{
    log("Transforming links.");
    int rows = _links.size();
    int columns = _links[0].size();
    for (int i = 0; i < rows; i++)
    {
        _nodes[i + 1]->unconnect_all_nodes();

        log((boost::format("Connecting vertexes to node #%d...") % (i + 1)).str());
        for (int j = 0; j < columns; j++)
        {
            int element = _links[i][j];
            if (i != j && element != 0)
            {
                log((boost::format("Connected node #%d, to #%d") % i % j).str());
                _nodes[i + 1]->connect_node(_nodes[j + 1]);
            }
        }
    }
}


void graph_data::connect_vertex_to_vertex(unsigned int first_vertex, unsigned int second_vertex)
{
    log((boost::format("Connecting node #%d to #%d") % first_vertex % second_vertex).str());
    if (!_nodes.contains(first_vertex))
    {
        throw std::logic_error("Node #" + std::to_string(first_vertex) + " not exists");
    }
    if (!_nodes.contains(second_vertex))
    {
        throw std::logic_error("Node #" + std::to_string(first_vertex) + " not exists");
    }

    QSharedPointer<GraphNode> _node = _nodes[second_vertex];
    _nodes[first_vertex]->connect_node(std::move(_node));

    log("Node connected!");
}

void graph_data::connect_vertexes_to_vertex(unsigned int first_vertex, QList<unsigned int> other_vertexes)
{
    log((boost::format("Connecting node #%d to %d nodes") % first_vertex % other_vertexes.size()).str());

    if (!_nodes.contains(first_vertex))
    {
        boost::format _error_message = boost::format("Node %d is not exists") % first_vertex;
        throw std::logic_error(_error_message.str());
    }

    for (const unsigned int& to_connect_vertex_num : other_vertexes)
    {
        connect_vertex_to_vertex(first_vertex, to_connect_vertex_num);
    }
}

unsigned int graph_data::size()
{
	return _nodes.size();
}

void graph_data::clear()
{
    _name.clear();
	_nodes.clear();
}

QList<unsigned int> graph_data::get_nodes_numbers() const
{
    reindex_vertexes_numbers();
    return _nodes.keys();
}

void graph_data::reindex_vertexes_numbers() const
{
    if (_nodes.size() > 0) {
		QList<unsigned int> indexes = _nodes.keys();
		for (int i = 0; i < indexes.size(); i++)
		{
			unsigned int index = indexes[i];
			QSharedPointer<GraphNode> _node = _nodes[index];
			unsigned int real_index = _node->get_number();

			if (index != real_index && _node)
			{
				if (!_nodes.contains(real_index)) {
					log((boost::format("Reindexing node #%d to %d") % index % real_index).str());
					_nodes[real_index] = _node;
				}
				else {
					int free_index = get_min_free_index(indexes);

					log((boost::format("Reindexing node #%d to %d") % index % free_index).str());
					_node->set_number(free_index);
					_nodes[free_index] = _node;
				}

				log("Remove old node #" + std::to_string(index));
				_nodes.remove(index);
			}

		}

		for (const QSharedPointer<GraphNode>& node : _nodes.values())
		{
			node->reindex_connected_nodes();
		}
    }
}

void graph_data::set_name(const QString &name)
{
    if (!name.isEmpty()) {
		_name = name;
    }
}

QString graph_data::name()
{
	return _name;
}

void graph_data::set_additional_data_to_vertex(unsigned int node_num, QVariant _additional_data)
{
    if (_nodes.contains(node_num))
        _nodes[node_num]->set_additional_data(_additional_data);
}

QList<QSharedPointer<GraphNode>> graph_data::bfs(unsigned int start_node, std::function<void(QSharedPointer<GraphNode>&, int)> node_performer)
{
    reindex_vertexes_numbers();
    log("Doing bfs from start node #" + std::to_string(start_node));
    if (!_nodes.contains(start_node))
    {
        return QList<QSharedPointer<GraphNode>>();
    }

    auto first_node = get_vertex(start_node);
    log("Found node #" + std::to_string(first_node->get_number()));

    QQueue<QSharedPointer<GraphNode>> performed_vertexes;
    QQueue<int> _performers_level{{0}};
    QQueue<QSharedPointer<GraphNode>> vertexes_to_perform{{std::move(first_node)}};

    log_debug("Start step cycle...");
    while (!vertexes_to_perform.empty())
    {
        QSharedPointer<GraphNode> now_node = vertexes_to_perform.dequeue();
        int level = _performers_level.dequeue();

        log((boost::format("Step on node #%d with level %d.") % now_node->get_number() % level).str());

        try {
            if (now_node && !performed_vertexes.contains(now_node))
            {
                log("Performing node...");
                node_performer(now_node, level);

                log("Getting childs...");
                if (now_node->is_has_connected_nodes())
                {
                    QList<QWeakPointer<GraphNode>> connected_nodes = now_node->get_connected_nodes();

                    for (QWeakPointer<GraphNode>& _node : connected_nodes) {
                        vertexes_to_perform.enqueue(_node.toStrongRef());
                        _performers_level.enqueue(level + 1);
                    }
                }

                log("Adding node to performed nodes...");
                performed_vertexes.enqueue(std::move(now_node));
            }
        }
        catch (const std::exception& error)
        {
            log((boost::format("Error when performing bfs: %s") % error.what()).str());
        }
    }

    return performed_vertexes.toList();
}

void graph_data::bfs_no_result(unsigned int start_node, std::function<void (QSharedPointer<GraphNode>&, int)> node_performer)
{
    reindex_vertexes_numbers();

    if (_nodes.contains(start_node)) {
        bfs(start_node, node_performer);
    }
}

void graph_data::bfs_no_result(std::function<void(QSharedPointer<GraphNode>&, int)> node_performer)
{
    reindex_vertexes_numbers();

    QList<unsigned int> _vertexes = _nodes.keys();
    QQueue<unsigned int> _performed_nodes;
    for (const unsigned int& _index: _vertexes)
    {
        if (!_performed_nodes.contains(_index)) {
			QList<QSharedPointer<GraphNode>> _performed_vertexes_by_one = bfs(_index, node_performer);

			std::transform(_performed_vertexes_by_one.begin(), _performed_vertexes_by_one.end(),
                        std::back_inserter(_performed_nodes),
					    [](const QSharedPointer<GraphNode>& pointer){
							return pointer->get_number();
						}
            );
        }
    }
}
graph_data &graph_data::operator=(const graph_data &other)
{
    _name = other._name;
    _nodes = other._nodes;
    return *this;
}

bool graph_data::operator==(graph_data &other)
{
    log_debug(((boost::format("Start comparsion graph %s and %s") % _name.toStdString() % other.name().toStdString()).str()));
	bool names_comparsion = _name == other._name;

    QList<unsigned int> _indexes_1 = _nodes.keys();
    QList<unsigned int> _indexes_2 = _nodes.keys();

	bool is_vertex = false;
    bool vertex_count_comparsion = _indexes_1.size() == _indexes_2.size();
    if (vertex_count_comparsion && _indexes_1.size() > 0 && _indexes_2.size() > 0) {
        log_debug("Graphs are same size...");
		for (int i = 0; i < _nodes.size(); i++)
		{
			unsigned int index_1 = _indexes_1[i];
			unsigned int index_2 = _indexes_2[i];

            log_debug((boost::format("Comparing a node #%d with #%d of other graph") % index_1 % index_2).str());
            bool vertex_comparsion = index_1 == index_2 && _nodes[index_1] == _nodes[index_2];
            if (!vertex_comparsion)
			{
                log("No same nodes");
                is_vertex = false;
                break;
            }
            else
            {
                log("Same nodes");
                is_vertex = true;
            }
        }
    }

    return names_comparsion && is_vertex && vertex_count_comparsion;
}