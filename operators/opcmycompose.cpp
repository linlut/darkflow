#include "opcmycompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "algorithm.h"

static Photo
blackDot()
{
    return Photo(Magick::Image(Magick::Geometry(1,1), Magick::Color(0,0,0)), Photo::Linear);
}

class WorkerCMYCompose : public OperatorWorker {
public:
    WorkerCMYCompose(QThread *thread, Operator *op) :
        OperatorWorker(thread, op)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int l_count = m_inputs[0].count();
        int c_count = m_inputs[1].count();
        int m_count = m_inputs[2].count();
        int y_count = m_inputs[3].count();
        int photo_count = qMax(qMax(qMax(l_count, c_count), m_count), y_count);

        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pLuminance;
            Photo pCyan;
            Photo pMagenta;
            Photo pYellow;

            if ( l_count )
                pLuminance = m_inputs[0][i%l_count];
            else
                pLuminance = blackDot();

            if ( c_count )
                pCyan = m_inputs[1][i%c_count];
            else
                pCyan = blackDot();

            if ( m_count )
                pMagenta = m_inputs[2][i%m_count];
            else
                pMagenta = blackDot();

            if ( y_count )
                pYellow = m_inputs[3][i%y_count];
            else
                pYellow = blackDot();

            Magick::Image& iLuminance = pLuminance.image();
            Magick::Image& iCyan = pCyan.image();
            Magick::Image& iMagenta = pMagenta.image();
            Magick::Image& iYellow = pYellow.image();

            unsigned w = qMax(qMax(qMax(iLuminance.columns(),iCyan.columns()), iMagenta.columns()), iYellow.columns());
            unsigned h = qMax(qMax(qMax(iLuminance.rows(),iCyan.rows()), iMagenta.rows()), iYellow.rows());
            if ( iLuminance.columns() != w || iLuminance.rows() != h )
                iLuminance.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            if ( iCyan.columns() != w || iCyan.rows() != h )
                iCyan.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            if ( iMagenta.columns() != w || iMagenta.rows() != h )
                iMagenta.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
            if ( iYellow.columns() != w || iYellow.rows() != h )
                iYellow.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);

            try {
                Magick::Pixels iCyan_cache(iCyan);
                Magick::Pixels iMagenta_cache(iMagenta);
                Magick::Pixels iYellow_cache(iYellow);
                Magick::Pixels iLuminance_cache(iLuminance);


                Photo photo(Photo::Linear);
                photo.createImage(w, h);
                photo.setSequenceNumber(i);
                photo.setIdentity(m_operator->uuid() + ":" + QString::number(i));
                photo.setTag(TAG_NAME, "LCMY Composition");
                Magick::Pixels iPhoto_cache(photo.image());
                Magick::PixelPacket *pxl = iPhoto_cache.get(0, 0, w, h);
                int line = 0;
#pragma omp parallel for
                for ( unsigned y = 0 ; y < h ; ++y ) {
                    const Magick::PixelPacket *pxl_Cyan = iCyan_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Magenta = iMagenta_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Yellow = iYellow_cache.getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Luminance = iLuminance_cache.getConst(0, y, w, 1);
                    for ( unsigned x = 0 ; x < w ; ++x ) {
                        quantum_t rgb[3];
                        quantum_t cyan = 0;
                        quantum_t magenta = 0;
                        quantum_t yellow = 0;
                        if ( pxl_Cyan ) cyan = (pxl_Cyan[x].green+pxl_Cyan[x].blue)/2;
                        if ( pxl_Magenta ) magenta = (pxl_Magenta[x].red+pxl_Magenta[x].blue)/2;
                        if ( pxl_Yellow) yellow = (pxl_Yellow[x].red+pxl_Yellow[x].green)/2;

                        rgb[0] =  - cyan + magenta + yellow;
                        rgb[1] =    cyan - magenta + yellow;
                        rgb[2] =    cyan + magenta - yellow;
                        if ( l_count ) {
                            double lum = .2126L*(pxl_Luminance?pxl_Luminance[x].red:0) +
                                    .7152L*(pxl_Luminance?pxl_Luminance[x].green:0) +
                                    .0722L*(pxl_Luminance?pxl_Luminance[x].blue:0);
                            double cur = .2126L*rgb[0] +
                                    .7152L*rgb[1] +
                                    .0722L*rgb[2];
                            double mul = lum/cur;
                            rgb[0] = round(mul*rgb[0]);
                            rgb[1] = round(mul*rgb[1]);
                            rgb[2] = round(mul*rgb[2]);
                        }
                        pxl[y*w+x].red = clamp(rgb[0]);
                        pxl[y*w+x].green = clamp(rgb[1]);
                        pxl[y*w+x].blue = clamp(rgb[2]);
                    }
#pragma omp critical
                    {
                        emitProgress(i, photo_count,line++, h);
                    }
                }
                iPhoto_cache.sync();
                outputPush(0, photo);
                emitProgress(i, photo_count, 1, 1);
            }
            catch (std::exception &e) {
                if (l_count)
                    setError(pLuminance, e.what());
                if (c_count)
                    setError(pCyan, e.what());
                if (m_count)
                    setError(pMagenta, e.what());
                if (y_count)
                    setError(pYellow, e.what());
                emitFailure();
                return;
            }
        }
        if (aborted())
            emitFailure();
        else
            emitSuccess();
    }
};

OpCMYCompose::OpCMYCompose(Process *parent) :
    Operator(OP_SECTION_COLOR, "LCMY Compose", Operator::NonHDR, parent)
{
    addInput(new OperatorInput("Luminance", "Luminance", OperatorInput::Set, this));
    addInput(new OperatorInput("Cyan", "Cyan", OperatorInput::Set, this));
    addInput(new OperatorInput("Magenta", "Magenta", OperatorInput::Set, this));
    addInput(new OperatorInput("Yellow", "Yellow", OperatorInput::Set, this));
    addOutput(new OperatorOutput("RGB", "RGB", this));
}

OpCMYCompose *OpCMYCompose::newInstance()
{
    return new OpCMYCompose(m_process);
}

OperatorWorker *OpCMYCompose::newWorker()
{
    return new WorkerCMYCompose(m_thread, this);
}
