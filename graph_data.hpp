#ifndef GRAPH_DATA_HPP
#define GRAPH_DATA_HPP

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QList>
#include <QMap>
#include <QWeakPointer>

class GraphNode : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int node_number READ get_number WRITE set_number)
    Q_PROPERTY(QString node_name READ get_name WRITE set_name)

public:
    GraphNode(int node_num, QString node_name_);
    ~GraphNode();

    QString get_name() const;
    void set_name(const QString &newNode_name);

    int get_number() const;
    void set_number(int newNode_number);

    Q_INVOKABLE QList<QWeakPointer<GraphNode>> get_connected_nodes();
    Q_INVOKABLE void connect_node(QSharedPointer<GraphNode> _node);
    Q_INVOKABLE void connect_node(QWeakPointer<GraphNode> _node);

    Q_INVOKABLE void unconnect_node(int node_num);

    Q_INVOKABLE bool is_has_connected_nodes();
    Q_INVOKABLE bool is_vertex_connected(int node_num);

protected:
    int node_number;
    QString node_name;

    QMap<int, QWeakPointer<GraphNode>> nodes;

};

class graph_data : public QObject
{
    Q_OBJECT
public:
    graph_data();
    ~graph_data();

    Q_INVOKABLE void add_vertex(int node_num, QString node_name = QString(), QList<int> connect_to_vertexes = QList<int>());

    Q_INVOKABLE QSharedPointer<GraphNode> get_vertex(int node_num);
    Q_INVOKABLE QSharedPointer<GraphNode> get_vertex(QString node_name);

    QQueue<QSharedPointer<GraphNode>> bfs(int start_node, std::function<void(QSharedPointer<GraphNode>)> node_performer);

protected:
    QMap<int, QSharedPointer<GraphNode>> _nodes;
};

#endif // GRAPH_DATA_HPP
