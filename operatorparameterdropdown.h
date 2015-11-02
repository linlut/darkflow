#ifndef OPERATORPARAMETERDROPDOWN_H
#define OPERATORPARAMETERDROPDOWN_H

#include <QString>
#include <QPoint>

#include "operatorparameter.h"

class QObject;
class QMenu;
class QAction;

class OperatorParameterDropDown : public OperatorParameter
{
    Q_OBJECT
public:
    OperatorParameterDropDown(
            const QString& name,
            const QString& caption,
            const QString& currentValue,
            QObject *parent = 0);
    ~OperatorParameterDropDown();

    void addOption(const QString& option, QObject *obj, const char *slot);
    void dropDown(const QPoint& pos);

    QString currentValue() const;

    QJsonObject save();
    void load(const QJsonObject &obj);

signals:
    void valueChanged(const QString& value);

private slots:
    void actionTriggered(QAction *action);

private:
    QMenu *m_menu;
    QString m_currentValue;
};

#endif // OPERATORPARAMETERDROPDOWN_H
