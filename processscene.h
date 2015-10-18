#ifndef PROCESSSCENE_H
#define PROCESSSCENE_H

#include <QGraphicsScene>
#include <QPointF>


class QGraphicsSceneContextMenuEvent;
class ProcessNode;

class ProcessScene : public QGraphicsScene
{
    Q_OBJECT
public:

    enum {
        UserTypeNode = 1,
        UserTypeInput,
        UserTypeOutput,
        UserTypeConnection,
        UserTypeButton
    };

    explicit ProcessScene(QWidget *parent = 0);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

signals:
    void contextMenuSignal(QGraphicsSceneContextMenuEvent *event);

public slots:

};

#endif // PROCESSSCENE_H
