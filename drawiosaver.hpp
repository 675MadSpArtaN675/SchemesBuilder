#ifndef DRAWIOSAVER_HPP
#define DRAWIOSAVER_HPP

constexpr unsigned int multiply_elements = 2;

#include <QObject>
#include <QQmlEngine>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <boost/format.hpp>

#include "abstractsaver.hpp"
#include "graph_painter.hpp"

class DrawioSaver : public AbstractSaver
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(unsigned int width READ box_width WRITE set_box_width BINDABLE bindable_box_width);
    Q_PROPERTY(unsigned int height READ box_height WRITE set_box_height BINDABLE bindable_box_height);

    Q_OBJECT_BINDABLE_PROPERTY(DrawioSaver, unsigned int, _width)
    Q_OBJECT_BINDABLE_PROPERTY(DrawioSaver, unsigned int, _height)

public:
    explicit DrawioSaver(QObject* parent = nullptr);

    unsigned int box_width();
    void set_box_width(unsigned int width);
    QBindable<unsigned int> bindable_box_width();

    unsigned int box_height();
    void set_box_height(unsigned int width);
    QBindable<unsigned int> bindable_box_height();

protected:
    virtual void save_logic(QFile& file) override;
    QString node_to_string(GraphNode& _node, const unsigned int& multiply);
};

#endif // DRAWIOSAVER_HPP
