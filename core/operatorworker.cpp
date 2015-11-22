#include <QThread>

#include "operatorworker.h"
#include "operator.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "photo.h"

static struct AtStart {
    AtStart() {
        qRegisterMetaType<QVector<QVector<Photo> > >("QVector<QVector<Photo> >");
    }
} foo;

OperatorWorker::OperatorWorker(QThread *thread, Operator *op) :
    QObject(NULL),
    m_thread(thread),
    m_operator(op),
    m_inputs(),
    m_n_outputs(0),
    m_outputs(),
    m_signalEmited(false)
{
    moveToThread(thread);
    connect(m_thread, SIGNAL(finished()), this, SLOT(finished()));
    connect(this, SIGNAL(start(QVector<QVector<Photo> >, int)), this, SLOT(play(QVector<QVector<Photo> >, int)));
    connect(this, SIGNAL(progress(int,int)), m_operator, SLOT(workerProgress(int,int)));
    connect(this, SIGNAL(success(QVector<QVector<Photo> >)), m_operator, SLOT(workerSuccess(QVector<QVector<Photo> >)));
    connect(this, SIGNAL(failure()), m_operator, SLOT(workerFailure()));
    m_thread->start();
}

void OperatorWorker::play(QVector<QVector<Photo> > inputs, int n_outputs)
{
    m_inputs = inputs;
    play_prepareOutputs(n_outputs);
    qDebug("OperatorWorker::play()");

    if ( !play_inputsAvailable() )
        return;
    if (!play_outputsAvailable())
        return;

    play_analyseSources();

    play_onInput(0);
    if (!m_signalEmited) {
        qWarning("BUG: No signal sent!!!");
        emitFailure();
    }
}

void OperatorWorker::finished()
{
    if ( !m_signalEmited) {
        qDebug("OperatorWorker: not signal sent, sending failure");
        emitFailure();
    }
}

bool OperatorWorker::aborted() {
    return m_thread->isInterruptionRequested();
}

void OperatorWorker::emitFailure() {
    m_signalEmited = true;
    qDebug(QString("Worker of " + m_operator->uuid() + "emit progress(0, 1)").toLatin1());
    emit progress(0, 1);
    qDebug(QString("Worker of " + m_operator->uuid() + "emit failure").toLatin1());
    emit failure();
    qDebug(QString("Worker of " + m_operator->uuid() + "emit done").toLatin1());
}

void OperatorWorker::emitSuccess()
{
    m_signalEmited = true;
    qDebug(QString("Worker of " + m_operator->uuid() + "emit progress(1, 1)").toLatin1());
    emit progress(1, 1);
    qDebug(QString("Worker of " + m_operator->uuid() + "emit success").toLatin1());
    emit success(m_outputs);
    qDebug(QString("Worker of " + m_operator->uuid() + "emit done").toLatin1());
}

void OperatorWorker::emitProgress(int p, int c, int sub_p, int sub_c)
{
    emit progress( p * sub_c + sub_p , c * sub_c);
}

bool OperatorWorker::play_inputsAvailable()
{
    if ( 0 == m_inputs.size() ) {
        qWarning("OperatorWorker::play() not overloaded for no input");
        emitFailure();
        return false;
    }
    return true;
}

void OperatorWorker::play_prepareOutputs(int n_outputs)
{
    for (int i = 0 ; i < n_outputs ; ++i )
        m_outputs.push_back(QVector<Photo>());
    m_n_outputs = n_outputs;
}

bool OperatorWorker::play_outputsAvailable()
{
    if ( 0 == m_n_outputs ) {
        qWarning("OperatorWorker::play() not overloaded for #output != 1");
        emitFailure();
        return false;
    }
    return true;
}

void OperatorWorker::play_analyseSources()
{

}

bool OperatorWorker::play_onInput(int idx)
{
    int c = 0;
    int p = 0;
    c = m_inputs[idx].count();

    foreach(Photo photo, m_inputs[idx]) {
        if ( aborted() ) {
            qDebug("OperatorWorker aborted, sending failure");
            emitFailure();
            return false;
        }
        emit progress(p, c);
        Photo newPhoto = this->process(photo, p++, c);
        if ( !newPhoto.isComplete() ) {
            qWarning("OperatorWorker: photo is not complete, sending failure");
            emitFailure();
            return false;
        }
        m_outputs[0].push_back(newPhoto);
    }

    emitSuccess();
    return true;
}
