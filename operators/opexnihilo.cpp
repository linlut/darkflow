#include <QVector>
#include "operatoroutput.h"
#include "opexnihilo.h"
#include "operatorworker.h"
#include "process.h"
#include "photo.h"

#include <Magick++.h>

class ExNihilo : public OperatorWorker
{
public:
    ExNihilo(QThread *thread, OpExNihilo *op) :
        OperatorWorker(thread, op)
    {}
private slots:
    Photo process(const Photo &, int, int) { throw 0; }
    void play() {
        qDebug("play!!");
        Photo photo(Photo::Linear);
        photo.setIdentity(m_operator->uuid());
        photo.createImage(1000,1000);
        if (photo.isComplete()) {
            Magick::Image& image = photo.image();
            image.modifyImage();
            Magick::Pixels cache(image);
            size_t w = image.columns();
            size_t h = image.rows();
            for (size_t y = 0 ; y < h ; ++y) {
                emit progress(y, h);
                if ( aborted() ) {
                    emitFailure();
                    return;
                }
                Magick::PixelPacket *pixels = cache.get(0,y,w,1);
                for (size_t x = 0 ; x < w ; ++x ) {
                    using Magick::Quantum;
                    pixels[x].red = qrand()%QuantumRange;
                    pixels[x].green = qrand()%QuantumRange;
                    pixels[x].blue = qrand()%QuantumRange;
                }
            }
            cache.sync();
            photo.setTag("Name", "Random Image");
            outputPush(0, photo);
            emitSuccess();
        }
        else {
            emitFailure();
        }
    }
};

OpExNihilo::OpExNihilo(Process *parent) :
    Operator(OP_SECTION_DEPRECATED, "Ex Nihilo", parent)
{
    addOutput(new OperatorOutput("Random image", "Random Image", this));
}

OpExNihilo::~OpExNihilo()
{
    //qDebug((QString("Delete of ")+getClassIdentifier()).toLatin1().data());
}

OpExNihilo *OpExNihilo::newInstance()
{
    return new OpExNihilo(m_process);
}

OperatorWorker *OpExNihilo::newWorker()
{
    return new ExNihilo(m_thread, this);
}
