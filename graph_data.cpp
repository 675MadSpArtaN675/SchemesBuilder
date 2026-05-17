#include "graph_data.hpp"

#include <QSet>
#include <QQueue>

#include <boost/format.hpp>

graph_data::graph_data() : graph_data("Name")
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

    _nodes.clear();
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

    GraphNode* node = new GraphNode(node_num, node_name);

    for (int node_num_to_connect: dublicate_delete)
    {
        log("Connecting node #" + std::to_string(node_num_to_connect) + " to node #" + std::to_string(node_num));
        if (!_nodes.contains(node_num_to_connect))
        {
            std::string message = (boost::format("Vertex with num \'%d\' is not exists! You can't connect unexcepted vertex to %d") % node_num_to_connect % node_num).str();
            delete node;

            throw std::logic_error(message);
        }

        node->connect_node(_nodes[node_num]);
        log("Node #" + std::to_string(node_num_to_connect) + " successfully connected!");
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
        log("Connecting node #" + std::to_string(connected_vertex_num) + " to node #" + std::to_string(num));

        if (_nodes.contains(connected_vertex_num)) {
            log("Node #" + std::to_string(connected_vertex_num) + " successfully connected!");
            QSharedPointer<GraphNode> connected_node = get_vertex(connected_vertex_num);

            _node_ptr->connect_node(connected_node.toWeakRef());
        }
    }
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
    reindex_vertexes_numbers();
    if (_nodes.contains(num))
    {
        _nodes.remove(num);
    }
}

void graph_data::remove_vertex(QString node_name)
{
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
    for (int i = 0; i < _links.size(); i++)
    {
        _nodes[i]->unconnect_all_nodes();
        for (int j = 0; j < _links[0].size(); j++)
        {
            int element = _links[i][j];
            if (i != j && element != 0)
            {
                log((boost::format("Connected node #%d, to #%d") % i % j).str());
                _nodes[i]->connect_node(_nodes[j]);
            }
        }
    }
}


void graph_data::connect_vertex_to_vertex(unsigned int first_vertex, unsigned int second_vertex)
{
    log((boost::format("Connecting node #%d, to #%d") % first_vertex % second_vertex).str());

    if (_nodes.contains(first_vertex) && _nodes.contains(second_vertex))
    {
        _nodes[first_vertex]->connect_node(_nodes[second_vertex]);
    }

    boost::format _error_message = boost::format("Node %d can not connected to node %d") % first_vertex % second_vertex;

    throw std::logic_error(_error_message.str());
}

void graph_data::connect_vertexes_to_vertex(unsigned int first_vertex, QList<unsigned int> other_vertexes)
{
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

QList<unsigned int> graph_data::get_nodes_numbers() const
{
    reindex_vertexes_numbers();
    return _nodes.keys();
}

void graph_data::reindex_vertexes_numbers() const
{
    QList<unsigned int> indexes = _nodes.keys();
    for (QPair<unsigned int, QSharedPointer<GraphNode>> _pair : _nodes.asKeyValueRange())
    {
        unsigned int index = _pair.first;
        QSharedPointer<GraphNode> _node = _pair.second;
        unsigned int real_index = _node->get_number();

        if (index != real_index && !_nodes.contains(real_index))
        {
            _nodes[real_index] = _node;
        }
        else if (_nodes.contains(real_index))
        {
            int free_index = get_min_free_index(indexes);
            _node->set_number(free_index);
            _nodes[free_index] = _node;
        }
    }

    for (const QSharedPointer<GraphNode>& node : _nodes.values())
    {
        node->reindex_connected_nodes();
    }
}

void graph_data::set_additional_data_to_vertex(unsigned int node_num, QVariant _additional_data)
{
    if (_nodes.contains(node_num))
        _nodes[node_num]->set_additional_data(_additional_data);
}

QList<QSharedPointer<GraphNode>> graph_data::bfs(unsigned int start_node, std::function<void(QSharedPointer<GraphNode>, int)> node_performer)
{
    log("Doing bfs from start node #" + std::to_string(start_node));
    if (!_nodes.contains(start_node))
    {
        return QList<QSharedPointer<GraphNode>>();
    }

    QQueue<QSharedPointer<GraphNode>> performed_vertexes;
    QQueue<int> _performers_level{{0}};
    QQueue<QSharedPointer<GraphNode>> vertexes_to_perform{{get_vertex(start_node)}};

    while (!vertexes_to_perform.empty())
    {
        QSharedPointer<GraphNode> now_node = vertexes_to_perform.dequeue();
        int level = _performers_level.dequeue();

        if (now_node && !performed_vertexes.contains(now_node))
        {
            node_performer(now_node, level);

            if (now_node->is_has_connected_nodes())
            {
                QList<QWeakPointer<GraphNode>> connected_nodes = now_node->get_connected_nodes();

                for (QWeakPointer<GraphNode>& _node : connected_nodes) {
                    vertexes_to_perform.enqueue(_node.toStrongRef());
                    _performers_level.enqueue(level + 1);
                }
            }

            performed_vertexes.enqueue(std::move(now_node));
        }
    }

    return performed_vertexes.toList();
}

void graph_data::bfs_no_result(unsigned int start_node, std::function<void (QSharedPointer<GraphNode>, int)> node_performer)
{
    if (_nodes.contains(start_node)) {
        bfs(start_node, node_performer);
    }
}

graph_data &graph_data::operator=(const graph_data &other)
{
    _name = other._name;
    _nodes = other._nodes;
    return *this;
}











































