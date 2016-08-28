/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include "oprgbcompose.h"
#include "operatorworker.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "operatorparameterdropdown.h"
#include "algorithm.h"
#include "console.h"
#include "cielab.h"
#include "hdr.h"

static Photo
blackDot()
{
    return Photo(Magick::Image(Magick::Geometry(1,1), Magick::Color(0,0,0)), Photo::Linear);
}

class WorkerRGBCompose : public OperatorWorker {
    bool m_outputHDR;
public:
    WorkerRGBCompose(bool outputHDR, QThread *thread, Operator *op) :
        OperatorWorker(thread, op),
        m_outputHDR(outputHDR)
    {}
    Photo process(const Photo &, int , int ) {
        throw 0;
    }
    void play() {
        int l_count = m_inputs[0].count();
        int r_count = m_inputs[1].count();
        int g_count = m_inputs[2].count();
        int b_count = m_inputs[3].count();
        int photo_count = qMax(qMax(qMax(l_count, r_count), g_count), b_count);

        for( int i = 0 ; i < photo_count ; ++i ) {
            if ( aborted() )
                continue;
            Photo pLuminance;
            Photo pRed;
            Photo pGreen;
            Photo pBlue;
            if ( l_count )
                pLuminance = m_inputs[0][i%l_count];
            else
                pLuminance = blackDot();

            if ( r_count )
                pRed = m_inputs[1][i%r_count];
            else
                pRed = blackDot();

            if ( g_count )
                pGreen = m_inputs[2][i%g_count];
            else
                pGreen = blackDot();

            if ( b_count )
                pBlue = m_inputs[3][i%b_count];
            else
                pBlue = blackDot();

            try {
                Magick::Image& iLuminance = pLuminance.image();
                Magick::Image& iRed = pRed.image();
                Magick::Image& iGreen = pGreen.image();
                Magick::Image& iBlue = pBlue.image();

                unsigned w = qMax(qMax(qMax(iLuminance.columns(),iRed.columns()), iGreen.columns()), iBlue.columns());
                unsigned h = qMax(qMax(qMax(iLuminance.rows(),iRed.rows()), iGreen.rows()), iBlue.rows());

                if ( iLuminance.columns() != w || iLuminance.rows() != h )
                    iLuminance.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
                if ( iRed.columns() != w || iRed.rows() != h )
                    iRed.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
                if ( iGreen.columns() != w || iGreen.rows() != h )
                    iGreen.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);
                if ( iBlue.columns() != w || iBlue.rows() != h )
                    iBlue.extent(Magick::Geometry(w, h), Magick::Color(0,0,0), Magick::NorthWestGravity);

                std::shared_ptr<Ordinary::Pixels> iRed_cache(new Ordinary::Pixels(iRed));
                std::shared_ptr<Ordinary::Pixels> iGreen_cache(new Ordinary::Pixels(iGreen));
                std::shared_ptr<Ordinary::Pixels> iBlue_cache(new Ordinary::Pixels(iBlue));
                std::shared_ptr<Ordinary::Pixels> iLuminance_cache(new Ordinary::Pixels(iLuminance));

                Photo photo(Photo::Linear);
                photo.createImage(w, h);
                photo.setSequenceNumber(i);
                photo.setIdentity(m_operator->uuid() + ":" + QString::number(i));
                photo.setTag(TAG_NAME, tr("LRGB Composition"));
                Ordinary::Pixels iPhoto_cache(photo.image());
                Magick::PixelPacket *pxl = iPhoto_cache.get(0, 0, w, h);
                dfl_block int line = 0;
                bool hdrLuminance = pLuminance.getScale() == Photo::HDR;
                bool hdrRed = pRed.getScale() == Photo::HDR;
                bool hdrGreen = pGreen.getScale() == Photo::HDR;
                bool hdrBlue = pBlue.getScale() == Photo::HDR;
                dfl_parallel_for(y, 0, int(h), 4, (iLuminance, iRed, iGreen, iBlue), {
                    const Magick::PixelPacket *pxl_Red = iRed_cache->getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Green = iGreen_cache->getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Blue = iBlue_cache->getConst(0, y, w, 1);
                    const Magick::PixelPacket *pxl_Luminance = iLuminance_cache->getConst(0, y, w, 1);
                    if ( m_error || !pxl_Red || !pxl_Green || !pxl_Blue || !pxl_Luminance ) {
                        if ( !m_error )
                            dflError(DF_NULL_PIXELS);
                        continue;
                    }
                    for ( unsigned x = 0 ; x < w ; ++x ) {
                        double red, green, blue;
                        red = pxl_Red?(hdrRed?fromHDR(pxl_Red[x].red):pxl_Red[x].red):0;
                        green = pxl_Green?(hdrGreen?fromHDR(pxl_Green[x].green):pxl_Green[x].green):0;
                        blue = pxl_Blue?(hdrBlue?fromHDR(pxl_Blue[x].blue):pxl_Blue[x].blue):0;

                        if ( l_count ) {
                            double lum = LUMINANCE(pxl_Luminance?(hdrLuminance?fromHDR(pxl_Luminance[x].red):pxl_Luminance[x].red):0,
                                                   pxl_Luminance?(hdrLuminance?fromHDR(pxl_Luminance[x].green):pxl_Luminance[x].green):0,
                                                   pxl_Luminance?(hdrLuminance?fromHDR(pxl_Luminance[x].blue):pxl_Luminance[x].blue):0);
                            double cur = LUMINANCE(red,
                                                   green,
                                                   blue);
                            double mul = lum/cur;
                            red = mul*red;
                            green =  mul*green;
                            blue = mul*blue;
                        }
                        if (m_outputHDR) {
                            pxl[y*w+x].red = toHDR(red);
                            pxl[y*w+x].green = toHDR(green);
                            pxl[y*w+x].blue = toHDR(blue);
                        }
                        else {
                            pxl[y*w+x].red=clamp<quantum_t>(DF_ROUND(red));
                            pxl[y*w+x].green=clamp<quantum_t>(DF_ROUND(green));
                            pxl[y*w+x].blue=clamp<quantum_t>(DF_ROUND(blue));
                        }
                    }
                    dfl_critical_section(
                    {
                        emitProgress(i, photo_count,line++, h);
                    });
                });
                iPhoto_cache.sync();
                if (m_outputHDR)
                    photo.setScale(Photo::HDR);
                outputPush(0, photo);
                emitProgress(i, photo_count, 1, 1);
            }
            catch (std::exception &e) {
                if (l_count)
                    setError(pLuminance, e.what());
                if (r_count)
                    setError(pRed, e.what());
                if (g_count)
                    setError(pGreen, e.what());
                if (b_count)
                    setError(pBlue, e.what());
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

OpRGBCompose::OpRGBCompose(Process *parent) :
    Operator(OP_SECTION_COLOR, QT_TRANSLATE_NOOP("Operator", "LRGB Compose"), Operator::All, parent),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    addInput(new OperatorInput(tr("Luminance"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Red"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Green"), OperatorInput::Set, this));
    addInput(new OperatorInput(tr("Blue"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("RGB"), this));

    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);
    addParameter(m_outputHDR);
}

OpRGBCompose *OpRGBCompose::newInstance()
{
    return new OpRGBCompose(m_process);
}

OperatorWorker *OpRGBCompose::newWorker()
{
    return new WorkerRGBCompose(m_outputHDRValue, m_thread, this);
}

void OpRGBCompose::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}
