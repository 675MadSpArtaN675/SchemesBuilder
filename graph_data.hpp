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
#include "graphnode.hpp"



class graph_data : protected CoreLogger
{
    Q_GADGET
    Q_PROPERTY(QString name READ name WRITE set_name)

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
    Q_INVOKABLE void remove_vertex(unsigned int num);
    Q_INVOKABLE void remove_vertex(QString node_name);
    Q_INVOKABLE void transform_links(QList<QList<int>>& _links);

    Q_INVOKABLE void connect_vertex_to_vertex(unsigned int first_vertex, unsigned int second_vertex);
    Q_INVOKABLE void connect_vertexes_to_vertex(unsigned int first_vertex, QList<unsigned int> other_vertexes);

    Q_INVOKABLE unsigned int size();
    Q_INVOKABLE void clear();
    Q_INVOKABLE QList<unsigned int> get_nodes_numbers() const;

    Q_INVOKABLE void set_additional_data_to_vertex(unsigned int node_num, QVariant _additional_data);
    QList<QSharedPointer<GraphNode>> bfs(unsigned int start_node, std::function<void(QSharedPointer<GraphNode>&, int)> node_performer);
    Q_INVOKABLE void bfs_no_result(unsigned int start_node, std::function<void(QSharedPointer<GraphNode>&, int)> node_performer);
    Q_INVOKABLE void bfs_no_result(std::function<void(QSharedPointer<GraphNode>&, int)> node_performer);

    Q_INVOKABLE void reindex_vertexes_numbers() const;

    void set_name(const QString& name);
    QString name();

    graph_data& operator=(const graph_data& other);
    bool operator==(graph_data& other);

protected:
    QString _name;
    mutable QMap<unsigned int, QSharedPointer<GraphNode>> _nodes;
};

#endif // GRAPH_DATA_HPP
