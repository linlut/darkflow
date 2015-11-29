#include "opexposure.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "exposure.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerExposure : public OperatorWorker {
public:
    WorkerExposure(qreal value, QThread *thread, OpExposure *op) :
        OperatorWorker(thread, op),
        m_exposure(value)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        m_exposure.applyOn(newPhoto);
        m_exposure.applyOnImage(newPhoto.curve());
        return newPhoto;
    }
private:
    Exposure m_exposure;
};

OpExposure::OpExposure(Process *parent) :
    Operator(OP_SECTION_CURVE, "Exposure", parent),
    m_value(new OperatorParameterSlider("value", "Exposure", "Modulate Exposure",Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<8, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this))
{
    m_inputs.push_back(new OperatorInput("Images","Images",OperatorInput::Set, this));
    m_outputs.push_back(new OperatorOutput("Images", "Images", this));
    m_parameters.push_back(m_value);

}

OpExposure *OpExposure::newInstance()
{
    return new OpExposure(m_process);
}

OperatorWorker *OpExposure::newWorker()
{
    return new WorkerExposure(m_value->value(), m_thread, this);
}