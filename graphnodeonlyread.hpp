#ifndef GRAPHNODEONLYREAD_HPP
#define GRAPHNODEONLYREAD_HPP

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSharedPointer>
#include <QList>
#include <QMap>
#include <QPair>
#include <QWeakPointer>

struct GraphNodeOnlyRead
{
    unsigned int number;
    QString name;
    QVariant additional_data;
    QList<unsigned int> connected_nodes;
    
    GraphNodeOnlyRead();
    GraphNodeOnlyRead(unsigned int _number, QString _name, QVariant additional_data, QList<unsigned int> connected_nodes);
    GraphNodeOnlyRead(GraphNodeOnlyRead& other);
    
private:
    Q_GADGET
};

#endif // GRAPHNODEONLYREAD_HPP
