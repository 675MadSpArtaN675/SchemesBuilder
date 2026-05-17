#include <boost/format.hpp>

#include "graphnode.hpp"



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

bool GraphNode::is_has_connected_nodes() const
{
    return !nodes.empty();
}

bool GraphNode::is_vertex_connected(unsigned int node_num) const
{
    return nodes.contains(node_num);
}

void GraphNode::reindex_connected_nodes()
{
    for (QPair<unsigned int, QSharedPointer<GraphNode>> _pair : nodes.asKeyValueRange())
    {
        unsigned int index = _pair.first;
        QSharedPointer<GraphNode> _node = _pair.second;
        unsigned int real_index = _node->get_number();

        if (index != real_index)
        {
            nodes[real_index] = _node;
        }
    }

}

GraphNodeOnlyRead GraphNode::to_only_readable() const
{
    return GraphNodeOnlyRead(node_number, node_name, _add_data, nodes.keys());
}

GraphNode &GraphNode::operator=(const GraphNode &other)
{
    node_name = other.node_name;
    node_number = other.node_number;
    nodes = other.nodes;
    
    return *this;
}

QVariant& GraphNode::additional_data() const
{
    return _add_data;
}

void GraphNode::set_additional_data(const QVariant &add_data)
{
    _add_data = add_data;
}

GraphNode& GraphNode::operator=(GraphNode&& other)
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

QList<QWeakPointer<GraphNode>> GraphNode::get_connected_nodes() const
{
    return nodes.values();
}

QList<unsigned int> GraphNode::get_connected_nodes_nums() const
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