#include <QThread>
#include <QJsonArray>

#include "operator.h"
#include "process.h"
#include "photo.h"
#include "operatorparameter.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorworker.h"

Operator::Operator(const QString& classSection, const QString& classIdentifier, Process *parent) :
    QObject(NULL),
    m_process(parent),
    m_enabled(true),
    m_upToDate(false),
    m_parameters(),
    m_inputs(),
    m_outputs(),
    m_waitingForParentUpToDate(false),
    m_uuid(Process::uuid()),
    m_classSection(classSection),
    m_classIdentifier(classIdentifier),
    m_name(classIdentifier),
    m_thread(new QThread(this)),
    m_worker(NULL)
{

}

Operator::~Operator()
{
    foreach(OperatorParameter *p, m_parameters)
        delete p;
    foreach(OperatorInput *i, m_inputs)
        delete i;
}

QVector<OperatorParameter *> Operator::getParameters()
{
    return m_parameters;
}

QVector<OperatorInput *> Operator::getInputs() const
{
    return m_inputs;
}

QVector<OperatorOutput *> Operator::getOutputs() const
{
    return m_outputs;
}

void Operator::abort()
{
    m_thread->requestInterruption();
}

void Operator::clone()
{
    Operator *op = newInstance();
    m_process->addOperator(op);
}

void Operator::workerProgress(int p, int c)
{
    emit progress(p,c);
}

void Operator::workerSuccess()
{
    setUpToDate(true);
    m_thread->quit();
    m_worker=NULL;
}

void Operator::workerFailure()
{
    setUpToDate(false);
    m_thread->quit();
    m_worker=NULL;
}

void Operator::parentUpToDate()
{
    if ( !m_waitingForParentUpToDate )
        return;
    m_waitingForParentUpToDate=false;
    play();
}

QString Operator::getName() const
{
    return m_name;
}

bool Operator::spotLoop(const QString &uuid)
{
    if ( m_uuid == uuid )
        return true;
    foreach(OperatorOutput *output, m_outputs) {
        foreach(OperatorInput *input, output->sinks()) {
            bool spotted = input->m_operator->spotLoop(uuid);
            if ( spotted )
                return true;
        }
    }

    return false;
}

void Operator::setName(const QString &name)
{
    m_name = name;
}

QString Operator::getClassIdentifier() const
{
    return m_classIdentifier;
}
QString Operator::getUuid() const
{
    return m_uuid;
}

void Operator::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}


void Operator::play() {
    if (!m_worker) {
        setUpToDate(false);
        m_worker = newWorker();
        m_worker->start();
    }
}

bool Operator::isUpToDate() const
{
    return m_upToDate;
}

void Operator::setUpToDate(bool b)
{
    m_upToDate = b;
    if (!m_upToDate) {
        foreach(OperatorOutput *output, m_outputs) {
            foreach(OperatorInput *remoteInput, output->sinks())
                remoteInput->m_operator->setUpToDate(false);
            output->m_result.clear();
        }
        emit progress(0, 1);
    }
    else {
        emit progress(1, 1);
        emit upToDate();
    }
}

QString Operator::getClassSection() const
{
    return m_classSection;
}

bool Operator::isEnabled() const
{
    return m_enabled;
}

void Operator::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void Operator::operator_connect(OperatorOutput *output, OperatorInput *input)
{
    output->addSink(input);
    input->addSource(output);
    connect(output->m_operator, SIGNAL(upToDate()), input->m_operator, SLOT(parentUpToDate()));
    input->m_operator->setUpToDate(false);
}

void Operator::operator_disconnect(OperatorOutput *output, OperatorInput *input)
{
    disconnect(output->m_operator, SIGNAL(upToDate()), input->m_operator, SLOT(parentUpToDate()));
    output->removeSink(input);
    input->removeSource(output);
    input->m_operator->setUpToDate(false);
}

void Operator::save(QJsonObject &obj)
{
    QJsonArray parameters;
    obj["enabled"] = m_enabled;
    obj["uuid"] = m_uuid;
    obj["classIdentifier"] = getClassIdentifier();
    obj["name"] = getName();
    foreach(OperatorParameter *parameter, m_parameters) {
        qWarning("saving a parameter");
        parameters.push_back(parameter->save());
    }
    obj["parameters"] = parameters;
}

void Operator::load(QJsonObject &obj)
{
    QJsonArray array = obj["parameters"].toArray();
    m_name = obj["name"].toString();
    m_enabled = obj["enabled"].toBool();
    m_uuid = obj["uuid"].toString();
    for (int i = 0 ; i < m_parameters.count(); ++i ) {
        QJsonObject obj = array[i].toObject();
        m_parameters[i]->load(obj);
    }
}
