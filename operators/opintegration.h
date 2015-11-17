#ifndef OPINTEGRATION_H
#define OPINTEGRATION_H

#include "operator.h"
#include <QObject>

class OperatorParameterSlider;
class OperatorParameterDropDown;

class OpIntegration : public Operator
{
    Q_OBJECT
public:
    typedef enum {
        NoRejection,
        SigmaClipping,
        Winsorized,
        MedianPercentil,
    } RejectionType;

    typedef enum {
        NoNormalization,
        HighestValue,
        Custom
    } NormalizationType;

    OpIntegration(Process *parent);

    OpIntegration *newInstance();
    OperatorWorker *newWorker();

public slots:
    void setNoRejection();
    void setMedianPercentil();
    void setSigmaClip();
    void setWinsorized();

    void setNoNormalization();
    void setHighestValue();
    void setCustom();

private:
    RejectionType m_rejectionType;
    OperatorParameterDropDown *m_rejectionTypeDropDown;
    OperatorParameterSlider *m_upper;
    OperatorParameterSlider *m_lower;
    NormalizationType m_normalizationType;
    OperatorParameterDropDown *m_normalizationTypeDropDown;
    OperatorParameterSlider *m_customNormalization;

};

#endif // OPINTEGRATION_H
