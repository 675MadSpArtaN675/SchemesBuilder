#ifndef GRAPHNODE_HPP
#define GRAPHNODE_HPP

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSharedPointer>
#include <QList>
#include <QMap>
#include <QPair>
#include <QWeakPointer>

#include "graphnodeonlyread.hpp"
#include "CoreLogger.hpp"

template<typename T>
requires std::integral<T>
int get_min_free_index(const QList<T>& _nodes)
{
    for (int i = 1; i <= _nodes.size(); i++)
    {
        if (!_nodes.contains(i))
        {
            return i;
        }
    }

    return _nodes.size() + 1;
}

class GraphNode : protected CoreLogger
{
    Q_GADGET

    Q_PROPERTY(int node_number READ get_number WRITE set_number)
    Q_PROPERTY(QString node_name READ get_name WRITE set_name)
    Q_PROPERTY(QVariant additional_data READ additional_data WRITE set_additional_data)

public:
    static GraphNode create(unsigned int node_num, QString name);
    static GraphNode create(unsigned int node_num, QString name, QMap<unsigned int, QWeakPointer<GraphNode>>& _nodes);

    GraphNode();
    GraphNode(unsigned int node_num, QString node_name_);
    GraphNode(const GraphNode& other);
    GraphNode(const GraphNode&& other);
    ~GraphNode();

    QString get_name() const;
    void set_name(const QString &newNode_name);

    unsigned int get_number() const;
    void set_number(unsigned int newNode_number);

    Q_INVOKABLE QList<QWeakPointer<GraphNode>> get_connected_nodes() const;
    Q_INVOKABLE QList<unsigned int> get_connected_nodes_nums() const;
    Q_INVOKABLE void connect_node(QSharedPointer<GraphNode> _node);
    Q_INVOKABLE void connect_node(QWeakPointer<GraphNode> _node);

    Q_INVOKABLE void unconnect_node(unsigned int node_num);
    Q_INVOKABLE void unconnect_all_nodes() noexcept;

    Q_INVOKABLE bool is_has_connected_nodes() const;
    Q_INVOKABLE bool is_vertex_connected(unsigned int node_num) const;
    Q_INVOKABLE void reindex_connected_nodes();

    Q_INVOKABLE GraphNodeOnlyRead to_only_readable() const;

    GraphNode& operator=(const GraphNode& other);
    GraphNode &operator=(GraphNode&& other);

    QVariant& additional_data() const;
    void set_additional_data(const QVariant &newAdd_data);


protected:
    unsigned int node_number;
    QString node_name;

    mutable QMap<unsigned int, QWeakPointer<GraphNode>> nodes;
    mutable QVariant _add_data;
};



#endif // GRAPHNODE_HPP
