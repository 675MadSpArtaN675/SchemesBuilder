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
    if (_nodes.contains(node_num)) {
        return _nodes[node_num];
    }

    return nullptr;
}

QSharedPointer<GraphNode> graph_data::get_vertex(QString node_name) const
{
    for (const QSharedPointer<GraphNode>& item : _nodes.values())
    {
        if (item->get_name() == node_name)
        {
            return item;
        }
    }

    return nullptr;
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
    return _nodes.keys();
}

void graph_data::recalculate_vertexes_numbers()
{
    QList<unsigned int> keys_in_nodes = _nodes.keys();

    for (int i = 1; i <= keys_in_nodes.size(); i++)
    {

    }
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

int graph_data::get_min_free_index()
{
    for (int i = 1; i <= _nodes.size(); i++)
    {
        if (!_nodes.contains(i))
        {
            log("Min free index = " + std::to_string(i));
            return i;
        }
    }

    log("Min free index = " + std::to_string(_nodes.size() + 1));
    return _nodes.size() + 1;
}

GraphNode GraphNode::create(unsigned int node_num, QString name)
{
    return GraphNode(node_num, name);
}

GraphNode GraphNode::create(unsigned int node_num, QString name, QMap<unsigned int, QWeakPointer<GraphNode> > &_nodes)
{
    GraphNode _node(node_num, name);

    for (QWeakPointer<GraphNode>& num_value : _nodes.values())
    {
        _node.connect_node(num_value);
    }

    return _node;
}

GraphNode::GraphNode(unsigned int node_num, QString node_name_)  : node_name{node_name_}, nodes{}, CoreLogger()
{
    if (node_num == 0)
    {
        node_number = 1;
    }
    else
    {
        node_number = node_num;
    }

    log((boost::format("Create %d with name %s") % node_number % node_name_.toStdString()).str());
}

GraphNode::GraphNode(const GraphNode &other)
{
    log((boost::format("Copying %d with name %s") % other.node_number % other.node_name.toStdString()).str());

    node_name = other.node_name;
    node_number = other.node_number;
    nodes = other.nodes;
}

GraphNode::GraphNode(const GraphNode &&other)
{
    log((boost::format("Moving %d with name %s") % other.node_number % other.node_name.toStdString()).str());

    node_name = std::move(other.node_name);
    node_number = std::move(other.node_number);
    nodes = std::move(other.nodes);
}

GraphNode::~GraphNode()
{
    log("Destroying node #" + std::to_string(node_number));

    nodes.clear();
}


void GraphNode::connect_node(QSharedPointer<GraphNode> _node)
{
    log((boost::format("Unconnecting node #%d from node #%d") % _node->get_number() % node_number).str());
    int node_num = _node->get_number();

    if (!nodes.contains(node_num)) {
        nodes.insert(node_num, _node.toWeakRef());
    }
}

void GraphNode::connect_node(QWeakPointer<GraphNode> _node)
{
    connect_node(_node.toStrongRef());
}

void GraphNode::unconnect_node(unsigned int node_num)
{
    log((boost::format("Unconnecting node #%d from node #%d") % node_num % node_number).str());

    if (nodes.contains(node_num))
    {
        nodes.remove(node_num);
    }
}

void GraphNode::unconnect_all_nodes()
{
    log((boost::format("Unconnecting all nodes from node #%d") % node_number).str());
    nodes.clear();
}

bool GraphNode::is_has_connected_nodes()
{
    return !nodes.empty();
}

bool GraphNode::is_vertex_connected(unsigned int node_num)
{
    return nodes.contains(node_num);
}

GraphNode &GraphNode::operator=(const GraphNode &other)
{
    node_name = other.node_name;
    node_number = other.node_number;
    nodes = other.nodes;

    return *this;
}

GraphNode& GraphNode::operator=(const GraphNode&& other)
{
    node_name = std::move(other.node_name);
    node_number = std::move(other.node_number);
    nodes = std::move(other.nodes);

    return *this;
}

unsigned int GraphNode::get_number() const
{
    return node_number;
}

void GraphNode::set_number(unsigned int newNode_number)
{
    node_number = newNode_number;
}

QList<QWeakPointer<GraphNode>> GraphNode::get_connected_nodes()
{
    return nodes.values();
}

QList<unsigned int> GraphNode::get_connected_nodes_nums()
{
    return nodes.keys();
}

QString GraphNode::get_name() const
{
    return node_name;
}

void GraphNode::set_name(const QString &newNode_name)
{
    node_name = newNode_name;
}
