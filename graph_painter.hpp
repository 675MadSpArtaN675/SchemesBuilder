#ifndef GRAPH_PAINTER_H
#define GRAPH_PAINTER_H

#include <QObject>
#include <QQuickWidget>
#include <QList>

#include <DataFrame/DataFrame.h>

#include "graph_data.hpp"
#include "CoreLogger.hpp"

struct GraphOptions
{
    double text_box_w;
    double text_box_h;
    double distance;
    double heigth;

    GraphOptions(double _text_box_w, double _text_box_h, double _distance, double _height);
private:
    Q_GADGET
};

class GraphPainter : public QObject, protected CoreLogger
{
    Q_OBJECT
    Q_PROPERTY(graph_data* graph_to_transform READ graph_to_transform WRITE set_graph_to_transform)

public:
    GraphPainter(graph_data* _graph, GraphOptions* _options = nullptr);
    GraphPainter();
    GraphPainter(GraphPainter&& other);

    Q_INVOKABLE void transform_graph(QList<QList<int>> table);
    Q_INVOKABLE void transform_graph(hmdf::Matrix<int> table);

    QWidget* parent_widget();
    void set_parent_widget(QWidget* _widget);

    graph_data* graph_to_transform() const;
    void set_graph_to_transform(graph_data* new_graph_to_transform);

protected:
    QQuickWidget* create_box(QList<GraphNode> _nodes, QList<QString> descr = QList<QString>(), QWidget* parent = nullptr);
    QQuickWidget* create_box(GraphNode _node, QString descr = QString(), QWidget* parent = nullptr);

    std::unique_ptr<graph_data> _graph_to_transform;

    QList<QList<int>> _graph_table;
    std::unique_ptr<GraphOptions> _options;
};

#endif // GRAPH_PAINTER_H
