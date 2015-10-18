#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>

#include "process.h"
#include "processscene.h"
#include "processnode.h"

#include "operatorexnihilo.h"
#include "operatorpaththrough.h"

Process::Process(ProcessScene *scene, QObject *parent) :
    QObject(parent),
    m_projectName(),
    m_notes(),
    m_outputDirectory(),
    m_temporaryDirectory(),
    m_scene(scene),
    m_dirty(false),
    m_availableOperators(),
    m_lastMousePosition()
{
    reset();
    connect(m_scene, SIGNAL(contextMenuSignal(QGraphicsSceneContextMenuEvent*)),
            this, SLOT(contextMenuSignal(QGraphicsSceneContextMenuEvent*)));
    m_scene->installEventFilter(this);

    m_availableOperators.push_back(new OperatorExNihilo(this));
    m_availableOperators.push_back(new OperatorPathThrough(this));
}


Process::~Process() {
    foreach(Operator *op, m_availableOperators) {
        delete op;
    }

    disconnect(m_scene, SIGNAL(contextMenuSignal(QGraphicsSceneContextMenuEvent*)),
            this, SLOT(contextMenuSignal(QGraphicsSceneContextMenuEvent*)));

}

QString Process::projectName() const
{
    return m_projectName;
}

void Process::setProjectName(const QString &projectName)
{
    if ( m_projectName != projectName ) {
        m_projectName = projectName;
        setDirty(true);
    }
}

QString Process::notes() const
{
    return m_notes;
}

void Process::setNotes(const QString &notes)
{
    if ( m_notes != notes ) {
        m_notes = notes;
     setDirty(true);
    }
}

QString Process::outputDirectory() const
{
    return m_outputDirectory;
}

void Process::setOutputDirectory(const QString &outputDirectory)
{
    if ( m_outputDirectory != outputDirectory ) {
        m_outputDirectory = outputDirectory;
        setDirty(true);
    }
}

QString Process::temporaryDirectory() const
{
    return m_temporaryDirectory;
}

void Process::setTemporaryDirectory(const QString &temporaryDirectory)
{
    if ( m_temporaryDirectory != temporaryDirectory ) {
        m_temporaryDirectory = temporaryDirectory;
        setDirty(true);
    }
}

bool Process::dirty() const
{
    return m_dirty;
}

void Process::setDirty(bool dirty)
{
    m_dirty = dirty;
    emit stateChanged();
}

void Process::addOperator(Operator *op)
{
    ProcessNode *node = new ProcessNode(m_lastMousePosition.x(),
                                        m_lastMousePosition.y(),
                                        op, this);
    this->m_nodes.insert(node);
    m_scene->addItem(node);
    setDirty(true);
}

QString Process::projectFile() const
{
    return m_projectFile;
}

void Process::setProjectFile(const QString &projectFile)
{
    m_projectFile = projectFile;
}

void Process::save()
{
    QJsonObject obj;
    QJsonDocument doc;
    obj["projectName"]=projectName();
    obj["notes"]=notes();
    obj["outputDirectory"]=outputDirectory();
    obj["temporaryDirectory"]=temporaryDirectory();
    doc.setObject(obj);
    QFile saveFile(projectFile());
    if (!saveFile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save file.");
       return;
    }

    saveFile.write(doc.toBinaryData());
    setDirty(false);
}

void Process::open(const QString& filename)
{
    QFile loadFile(filename);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
            qWarning("Couldn't open load file.");
            return;
    }
    QByteArray data = loadFile.readAll();
    QJsonDocument doc(QJsonDocument::fromBinaryData(data));
    QJsonObject obj = doc.object();
    setProjectFile(filename);
    setProjectName(obj["projectName"].toString());
    setNotes(obj["notes"].toString());
    setOutputDirectory(obj["outputDirectory"].toString());
    setTemporaryDirectory(obj["temporaryDirectory"].toString());
    setDirty(false);
}

bool Process::eventFilter(QObject *obj, QEvent *event)
{
    QGraphicsSceneMouseEvent *me =
            dynamic_cast<QGraphicsSceneMouseEvent*>(event);
    if ( NULL == me )
    {
        return QObject::eventFilter(obj, event);
    }
    m_lastMousePosition = me->scenePos();
/*
 *     qWarning("it's a mouse event");
    m_scene->addRect(me->scenePos().x()-5,
                     me->scenePos().y()-5,
                     10,
                     10);
*/
    return QObject::eventFilter(obj, event);

}

void Process::reset()
{
    setProjectName("");
    setNotes("");
    setProjectFile("");
    setOutputDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    setTemporaryDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    m_scene->clear();
    setDirty(true);
}

void Process::contextMenuSignal(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu(event->widget());
    foreach(Operator *op, m_availableOperators) {
        menu.addAction(QIcon(), op->getClassIdentifier(),op,SLOT(clone()));
    }
    menu.exec(event->screenPos());
}
