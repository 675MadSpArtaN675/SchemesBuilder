#ifndef DRAWIOSAVER_HPP
#define DRAWIOSAVER_HPP

#include <QObject>
#include <QQmlEngine>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "abstractsaver.hpp"

class DrawioSaver : public AbstractSaver
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit DrawioSaver(QObject* parent = nullptr);

protected:
    virtual void save_logic(QFile& file) override;
};

#endif // DRAWIOSAVER_HPP
