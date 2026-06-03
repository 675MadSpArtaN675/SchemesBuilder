#ifndef GRAPH_PAINTER_H
#define GRAPH_PAINTER_H

#include <QObject>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlComponent>
#include <QList>
#include <QQueue>

#include <DataFrame/DataFrame.h>
#include <boost/bind.hpp>

#include "graph_data.hpp"
#include "CoreLogger.hpp"

class GraphOptions : public QObject
{
private:
    Q_OBJECT

    Q_PROPERTY(double box_width READ getText_box_w WRITE setText_box_w BINDABLE bindable_w)
    Q_PROPERTY(double box_height READ getText_box_h WRITE setText_box_h BINDABLE bindable_h)

    Q_PROPERTY(double distance READ getDistance WRITE setDistance BINDABLE bindable_distance)
    Q_PROPERTY(double h_spacing READ getH_spacing WRITE setH_spacing BINDABLE bindable_h_spacing)

	Q_OBJECT_BINDABLE_PROPERTY(GraphOptions, double, text_box_w)
	Q_OBJECT_BINDABLE_PROPERTY(GraphOptions, double, text_box_h)
	Q_OBJECT_BINDABLE_PROPERTY(GraphOptions, double, distance)
	Q_OBJECT_BINDABLE_PROPERTY(GraphOptions, double, h_spacing)

public:
    explicit GraphOptions(QObject* parent = nullptr);
    GraphOptions(double _text_box_w, double _text_box_h, double _distance, double _h_spacing, QObject* parent = nullptr);
    GraphOptions(GraphOptions& _other);
    GraphOptions(GraphOptions&& _other);

    double getText_box_w() const;
    void setText_box_w(double newText_box_w);

    double getText_box_h() const;
    void setText_box_h(double newText_box_h);

    double getDistance() const;
    void setDistance(double newDistance);

    double getH_spacing() const;
    void setH_spacing(double newH_spacing);

    QBindable<double> bindable_w() { return QBindable<double>(&text_box_w); }
    QBindable<double> bindable_h() { return QBindable<double>(&text_box_h); }
    QBindable<double> bindable_distance() { return QBindable<double>(&distance); }
    QBindable<double> bindable_h_spacing() { return QBindable<double>(&h_spacing); }

    GraphOptions& operator=(const GraphOptions& other) = default;
    GraphOptions& operator=(GraphOptions&& other) = default;
};

class NodePaintOptions
{
    Q_GADGET
    Q_PROPERTY(double x READ getX WRITE setX)
    Q_PROPERTY(double y READ getY WRITE setY)
    Q_PROPERTY(QString description READ description WRITE setDescription)

public:
    NodePaintOptions() : NodePaintOptions(0, 0)
    { }

    NodePaintOptions(double _x, double _y) : NodePaintOptions(_x, _y, QString())
    { }

    NodePaintOptions(double _x, double _y, QString _description)
    {
        x = _x;
        y = _y;

        this->_description = _description;
    }

    NodePaintOptions(const NodePaintOptions& other)
    {
        x = other.x;
        y = other.y;
    }
    NodePaintOptions(NodePaintOptions&& other)
    {
        x = std::move(other.x);
        y = std::move(other.y);
    }

    NodePaintOptions& operator=(const NodePaintOptions& _node) = default;
    NodePaintOptions& operator=(NodePaintOptions&& _node) = default;

    double getX() const;
    void setX(double X);

    double getY() const;
    void setY(double Y);

    QString description() const;
    void setDescription(const QString &description);

protected:
    double x;
    double y;
    QString _description;
};



class GraphPainter : public QObject, protected CoreLogger
{
    Q_OBJECT
    Q_PROPERTY(graph_data* graph_to_transform READ graph_to_transform WRITE set_graph_to_transform)
    Q_PROPERTY(GraphOptions* options READ options WRITE set_options)
    Q_PROPERTY(double start_point_dx READ start_point_difference WRITE set_start_point_difference)

public:
    explicit GraphPainter(graph_data* _graph, GraphOptions* _options, QObject* parent = nullptr);
    explicit GraphPainter(QObject* parent = nullptr);
    GraphPainter(GraphPainter&& other);
    ~GraphPainter();

    Q_INVOKABLE void transform_graph(QList<QList<int>> table);
    void transform_graph(hmdf::Matrix<int> table);

    Q_INVOKABLE void get_engine(QObject* from_get = nullptr);
    Q_INVOKABLE void calculate_positions(double start_x, double start_y);
    Q_INVOKABLE void paint_on(QQuickItem* widget);
	Q_INVOKABLE QList<QList<QPointF>> paint_lines(QQuickItem* canvas);

    Q_INVOKABLE void clear_cache();

    void set_start_point_difference(double difference);
    double start_point_difference();

    graph_data* graph_to_transform() const;
    void set_graph_to_transform(graph_data* new_graph_to_transform);

    GraphOptions* options();
    void set_options(GraphOptions* _options);

protected:
    enum class _Side {
    	Left,
        Right
	};

    QPointF calculate_center_of_side(QQuickItem* _widget, _Side _side);

    QQuickItem* create_box(QList<GraphNode> _nodes, QList<QString> descr = QList<QString>(), QQuickItem* parent = nullptr, QMap<unsigned int, QQuickItem*>* exit_widgets = nullptr);
    QQuickItem* create_box(GraphNode _node, QString descr = QString(), QQuickItem* parent = nullptr);

    void calculate_one_point(QSharedPointer<GraphNode>& _node, int level, QQueue<NodePaintOptions>& positions, double& dx, NodePaintOptions* start_position);
    void paint_one_node(QSharedPointer<GraphNode>& _node, int level, QQuickItem* parent_widget, QMap<unsigned int, QQuickItem*>* _nodes);
    void create_lines_for_node(QSharedPointer<GraphNode>& _node, int level, QList<QList<QPointF>>* points, QMap<unsigned int, QQuickItem*>* nodes, QQuickItem* canvas);

	QList<GraphNode> get_nodes(QList<QWeakPointer<GraphNode>> _child_nodes);

	void set_pos(QQuickItem* _to_transform, QQuickItem* parent, NodePaintOptions& options);
	void set_size(QQuickItem* _to_transform);
    QList<QPointF> config_line(QQuickItem* _first_widget, QQuickItem* _second_widget, QQuickItem* canvas);

    std::unique_ptr<graph_data> _graph_to_transform;
    std::unique_ptr<GraphOptions> _options;

    QMap<unsigned int, QQuickItem*> _nodes;

    QList<QList<int>> _graph_table;
    QQmlEngine* _engine;

    double _difference_between_start_points;
};

#endif // GRAPH_PAINTER_H
