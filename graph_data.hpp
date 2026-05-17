#ifndef GRAPH_DATA_HPP
#define GRAPH_DATA_HPP

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSharedPointer>
#include <QList>
#include <QMap>
#include <QPair>
#include <QWeakPointer>

#include "CoreLogger.hpp"

class GraphNode : public QObject, protected CoreLogger
{
    Q_OBJECT

    Q_PROPERTY(int node_number READ get_number WRITE set_number)
    Q_PROPERTY(QString node_name READ get_name WRITE set_name)

public:
    static GraphNode create(unsigned int node_num, QString name);
    static GraphNode create(unsigned int node_num, QString name, QMap<unsigned int, QWeakPointer<GraphNode>>& _nodes);

    GraphNode(unsigned int node_num, QString node_name_);
    GraphNode(const GraphNode& other);
    GraphNode(const GraphNode&& other);
    ~GraphNode();

    QString get_name() const;
    void set_name(const QString &newNode_name);

    unsigned int get_number() const;
    void set_number(unsigned int newNode_number);

    Q_INVOKABLE QList<QWeakPointer<GraphNode>> get_connected_nodes();
    Q_INVOKABLE QList<unsigned int> get_connected_nodes_nums();
    Q_INVOKABLE void connect_node(QSharedPointer<GraphNode> _node);
    Q_INVOKABLE void connect_node(QWeakPointer<GraphNode> _node);

    Q_INVOKABLE void unconnect_node(unsigned int node_num);
    Q_INVOKABLE void unconnect_all_nodes();

    Q_INVOKABLE bool is_has_connected_nodes();
    Q_INVOKABLE bool is_vertex_connected(unsigned int node_num);

    GraphNode& operator=(const GraphNode& other);
    GraphNode& operator=(const GraphNode&& other);

protected:
    unsigned int node_number;
    QString node_name;

    QMap<unsigned int, QWeakPointer<GraphNode>> nodes;

};

class graph_data : public QObject, protected CoreLogger
{
    Q_OBJECT
public:
    graph_data();
    graph_data(QString name);
    graph_data(const graph_data& other);
    graph_data(const graph_data&& other);
    ~graph_data();

    Q_INVOKABLE void add_vertex(unsigned int node_num, QString node_name = QString(), QList<unsigned int> connect_to_vertexes = QList<unsigned int>());
    void add_vertex(GraphNode&& _node, QList<unsigned int> connected_vertexes = QList<unsigned int>());

    Q_INVOKABLE QSharedPointer<GraphNode> get_vertex(unsigned int node_num) const;
    Q_INVOKABLE QSharedPointer<GraphNode> get_vertex(QString node_name) const;

    Q_INVOKABLE void transform_links(QList<QList<int>>& _links);

    Q_INVOKABLE void connect_vertex_to_vertex(unsigned int first_vertex, unsigned int second_vertex);
    Q_INVOKABLE void connect_vertexes_to_vertex(unsigned int first_vertex, QList<unsigned int> other_vertexes);

    Q_INVOKABLE QList<unsigned int> get_nodes_numbers() const;
    Q_INVOKABLE void recalculate_vertexes_numbers();

    QList<QSharedPointer<GraphNode>> bfs(unsigned int start_node, std::function<void(QSharedPointer<GraphNode>, int)> node_performer);
    Q_INVOKABLE void bfs_no_result(unsigned int start_node, std::function<void(QSharedPointer<GraphNode>, int)> node_performer);

    graph_data& operator=(const graph_data& other);

protected:
    int get_min_free_index();

    QString _name;
    QMap<unsigned int, QSharedPointer<GraphNode>> _nodes;
};

#endif // GRAPH_DATA_HPP
