#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ports.h"
#include <QDialog>

namespace Ui {
class Preferences;
}
class QAbstractButton;
class QSemaphore;
class QMutex;
class OperatorWorker;

class Preferences : public QDialog
{
    Q_OBJECT

public:
    typedef enum {
        Linear,
        sRGB,
        IUT_BT_709,
        SquareRoot,
        TargetNone
    } TransformTarget;
    typedef enum {
        Ignore,
        Warning,
        Error
    } IncompatibleAction;
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

    QString baseDir();

    bool acquireWorker(OperatorWorker *worker);
    void releaseWorker();

    TransformTarget getCurrentTarget() const;
    IncompatibleAction getIncompatibleAction() const;
    int getNumThreads() const;
    int getMagickNumThreads() const;
    int getLabSelectionSize() const;
    QString getAppConfigLocation() const;
    QColor color(QPalette::ColorRole role);
    void incrAtWork();
    void decrAtWork();
    unsigned long getAtWork();

private:
    void getDefaultMagickResources();
    void getMagickResources();
    void setMagickResources();
    bool load(bool create=true);
    void save();
    QJsonObject saveStyle();
    void loadStyle(QJsonObject& obj);

public slots:
    void clicked(QAbstractButton * button);
    void accept();
    void reject();
    void tmpDirClicked();
    void baseDirClicked();

private:
    Ui::Preferences *ui;
    u_int64_t m_defaultArea;
    u_int64_t m_defaultMemory;
    u_int64_t m_defaultMap;
    u_int64_t m_defaultDisk;
    u_int64_t m_defaultThreads;
    QSemaphore *m_sem;
    QMutex *m_mutex;
    u_int64_t m_currentMaxWorkers;
    u_int64_t m_scheduledMaxWorkers;
    u_int64_t m_OpenMPThreads;
    TransformTarget m_currentTarget;
    IncompatibleAction m_incompatibleAction;
    int m_labSelectionSize;
    QPalette m_palette;
    unsigned long m_atWork;
};

extern Preferences *preferences;
extern QPalette dflOriginalPalette;

#endif // PREFERENCES_H
