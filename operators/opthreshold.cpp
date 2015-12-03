#include "opthreshold.h"
#include "threshold.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerThreshold : public OperatorWorker {
public:
    WorkerThreshold(qreal high, qreal low, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_threshold(high, low)
    {
    }
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        m_threshold.applyOn(newPhoto);
        m_threshold.applyOnImage(newPhoto.curve());
        return newPhoto;
    }

private:
    Threshold m_threshold;
};

OpThreshold::OpThreshold(Process *parent) :
    Operator(OP_SECTION_COLOR, "Threshold", parent),
    m_high(new OperatorParameterSlider("high", "High", "Threshold High",Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./QuantumRange, 1, 1, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this)),
    m_low(new OperatorParameterSlider("low", "Low", "Threshold Low",Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./QuantumRange, 1, 1, 1./QuantumRange, 1, Slider::FilterExposureFromOne, this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Images", "Images", this));
    m_parameters.push_back(m_high);
    m_parameters.push_back(m_low);
}

OpThreshold *OpThreshold::newInstance()
{
    return new OpThreshold(m_process);
}

OperatorWorker *OpThreshold::newWorker()
{
    return new WorkerThreshold(m_high->value(),
                               m_low->value(),
                               m_thread, this);
}
