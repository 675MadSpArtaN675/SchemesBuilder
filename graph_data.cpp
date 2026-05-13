#include "graph_data.hpp"

#include <QSet>
#include <QQueue>

#include <boost/format.hpp>

graph_data::graph_data() {}

graph_data::~graph_data()
{
    _nodes.clear();
}

void graph_data::add_vertex(int node_num, QString node_name, QList<int> connect_to_vertexes)
{
    if (_nodes.contains(node_num))
    {
        throw std::logic_error("Vertex already exists");
    }

    QSet<int> dublicate_delete(connect_to_vertexes.begin(), connect_to_vertexes.end());

    if (dublicate_delete.contains(node_num))
    {
        dublicate_delete.remove(node_num);
    }

    GraphNode* node = new GraphNode(node_num, node_name);

    for (int node_num_to_connect: dublicate_delete)
    {
        if (!_nodes.contains(node_num_to_connect))
        {
            std::string message = (boost::format("Vertex with num \'%d\' is not exists! You can't connect unexcepted vertex to %d") % node_num_to_connect % node_num).str();
            delete node;

            throw std::logic_error(message);
        }

        node->connect_node(_nodes[node_num]);
    }

    _nodes.insert(node_num, QSharedPointer<GraphNode>(node));
}

QSharedPointer<GraphNode> graph_data::get_vertex(int node_num)
{
    if (_nodes.contains(node_num)) {
        return _nodes[node_num];
    }

    return nullptr;
}

QSharedPointer<GraphNode> graph_data::get_vertex(QString node_name)
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

QList<QSharedPointer<GraphNode>> graph_data::bfs(int start_node, std::function<void(QSharedPointer<GraphNode>)> node_performer)
{
    QQueue<QSharedPointer<GraphNode>> performed_vertexes;
    QQueue<int> _performers_level{{0}};
    QQueue<QSharedPointer<GraphNode>> vertexes_to_perform{{get_vertex(start_node)}};

    while (!vertexes_to_perform.empty())
    {
        QSharedPointer<GraphNode> now_node = vertexes_to_perform.dequeue();
        int level = _performers_level.dequeue();

        if (now_node && !performed_vertexes.contains(now_node))
        {
            node_performer(now_node);

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


GraphNode::GraphNode(int node_num, QString node_name_)  : node_number{node_num}, node_name{node_name_}, nodes{}
{ }

GraphNode::~GraphNode()
{
    nodes.clear();
}


void GraphNode::connect_node(QSharedPointer<GraphNode> _node)
{
    int node_num = _node->get_number();

    if (!nodes.contains(node_num)) {
        nodes.insert(node_num, _node.toWeakRef());
    }
}

void GraphNode::connect_node(QWeakPointer<GraphNode> _node)
{
    int node_num = _node.toStrongRef()->get_number();

    if (!nodes.contains(node_num)) {
        nodes.insert(node_num, _node);
    }
}

void GraphNode::unconnect_node(int node_num)
{
    if (nodes.contains(node_num))
    {
        nodes.remove(node_num);
    }
}

bool GraphNode::is_has_connected_nodes()
{
    return !nodes.empty();
}

bool GraphNode::is_vertex_connected(int node_num)
{
    return nodes.contains(node_num);
}

int GraphNode::get_number() const
{
    return node_number;
}

void GraphNode::set_number(int newNode_number)
{
    node_number = newNode_number;
}

QList<QWeakPointer<GraphNode>> GraphNode::get_connected_nodes()
{
    return nodes.values();
}

QString GraphNode::get_name() const
{
    return node_name;
}

void GraphNode::set_name(const QString &newNode_name)
{
    node_name = newNode_name;
}
