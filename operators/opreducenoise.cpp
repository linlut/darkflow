#include "opreducenoise.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterslider.h"
#include <Magick++.h>

using Magick::Quantum;

class WorkerReduceNoise : public OperatorWorker {
public:
    WorkerReduceNoise(qreal order, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_order(order)
    {}
    Photo process(const Photo &photo, int, int) {
        Photo newPhoto(photo);
        newPhoto.image().reduceNoise(m_order);
        return newPhoto;
    }
private:
    qreal m_order;
};

OpReduceNoise::OpReduceNoise(Process *parent) :
    Operator(OP_SECTION_COSMETIC, "Reduce Noise", Operator::NonHDR, parent),
    m_order(new OperatorParameterSlider("order", "order", "Reduce Noise Order", Slider::Value, Slider::Linear, Slider::Real, 0, 20, 3, 0, 100, Slider::FilterNothing, this))
{
    addInput(new OperatorInput("Images","Images",OperatorInput::Set, this));
    addOutput(new OperatorOutput("Images", "Images", this));
    addParameter(m_order);
}

OpReduceNoise *OpReduceNoise::newInstance()
{
    return new OpReduceNoise(m_process);
}

OperatorWorker *OpReduceNoise::newWorker()
{
    return new WorkerReduceNoise(m_order->value(),
                                 m_thread, this);
}
