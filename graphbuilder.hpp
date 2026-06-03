#ifndef GRAPHBUILDER_H
#define GRAPHBUILDER_H

#define STANDART_GRAPH_NAME "Graph"

#include <QObject>
#include <QSet>

#include <DataFrame/DataFrame.h>

#include "graph_data.hpp"
#include "tableformer.hpp"

#include "CoreLogger.hpp"

class GraphBuilder : public QObject, protected CoreLogger
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(graph_data* last_created_graph READ last_created)

public:
    using vector2d = std::vector<std::vector<int>>;
    using int_hmdf_matrix = hmdf::Matrix<int>;

    explicit GraphBuilder(QObject* parent = nullptr);
    explicit GraphBuilder(QString _name, QObject* parent = nullptr);
    GraphBuilder(const GraphBuilder& other_builder);
    GraphBuilder(GraphBuilder&& other_builder);
    ~GraphBuilder();

    GraphBuilder& create_nodes_from_matrix(int_hmdf_matrix& _matrix, QList<QString> names = QList<QString>());
    GraphBuilder& create_nodes_from_matrix(vector2d& _matrix, QList<QString> names = QList<QString>());
    Q_INVOKABLE GraphBuilder& create_nodes_from_matrix(QList<QList<int>> _matrix, QList<QString> names);
    Q_INVOKABLE GraphBuilder& create_nodes_from_preprocessor(TableFormer* _matrix);

    Q_INVOKABLE GraphBuilder& get_nodes_from_other_graph(const graph_data& _other_graph);

    Q_INVOKABLE GraphBuilder& add_node(GraphNode _node);
    Q_INVOKABLE GraphBuilder& add_node(int num, QString name);
    Q_INVOKABLE GraphBuilder& connect_node_to_node(unsigned int first_node_num, unsigned int second_node_num);
    Q_INVOKABLE GraphBuilder& unconnect_node_from_node(unsigned int first_node_num, unsigned int second_node_num);
    Q_INVOKABLE GraphBuilder& remove_node(unsigned int num);

    Q_INVOKABLE bool is_empty();
    Q_INVOKABLE GraphBuilder& clear();

    Q_INVOKABLE graph_data* build_ptr();
    Q_INVOKABLE graph_data build();

    GraphBuilder& operator=(const GraphBuilder& _other);
    GraphBuilder& operator=(const GraphBuilder&& _other);

    QString name() const;
    void setName(const QString &newName);

    graph_data* last_created();

protected:
    unsigned int get_max_index();
    QString _name;

    QMap<unsigned int, GraphNode> _temp_nodes;
    QMap<unsigned int, QSet<unsigned int>> _nodes_links;

    std::unique_ptr<graph_data> _cache;
};

#endif // GRAPHBUILDER_H