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
#include "process.h"
#include "operatorparameterfilescollection.h"
#include "operatorparameterdropdown.h"
#include "oploadraw.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "workerloadraw.h"

static const char *ColorSpaceStr[] = {
    QT_TRANSLATE_NOOP("OpLoadRaw", "Linear"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "sRGB"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "IUT BT.709")
};
static const char *DebayerStr[] = {
    QT_TRANSLATE_NOOP("OpLoadRaw", "None"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "Half Size"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "Low"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "VNG"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "PPG"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "AHD")
};
static const char *WhiteBalanceStr[] = {
    QT_TRANSLATE_NOOP("OpLoadRaw", "None"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "Raw colors"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "Camera"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "Daylight")
};
static const char *ClippingStr[] = {
    QT_TRANSLATE_NOOP("OpLoadRaw", "Auto"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "16-bit"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "15-bit"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "14-bit"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "13-bit"),
    QT_TRANSLATE_NOOP("OpLoadRaw", "12-bit")
};

OpLoadRaw::OpLoadRaw(Process *parent) :
    Operator(OP_SECTION_ASSETS, QT_TRANSLATE_NOOP("Operator", "Raw photos"), Operator::NA, parent),
    m_filesCollection(new OperatorParameterFilesCollection(
                          "rawCollection",
                          tr("RAW photos"),
                          tr("Select RAW photos to add to the collection"),
                          m_process->baseDirectory(),
                          tr("RAW photos (*.nef *.cr2 *.dng *.mef *.3fr *.raf *.x3f *.pef *.arw *.nrw);;"
                             "All Files (*.*)"), this)),
    m_colorSpace(new OperatorParameterDropDown("colorSpace", tr("Color Space"),this, SLOT(setColorSpace(int)))),
    m_debayer(new OperatorParameterDropDown("debayer", tr("Debayer"), this, SLOT(setDebayer(int)))),
    m_whiteBalance(new OperatorParameterDropDown("whiteBalance", tr("White Balance"), this, SLOT(setWhiteBalance(int)))),
    m_clipping(new OperatorParameterDropDown("clip", tr("Clip Highlight"), this, SLOT(setClipping(int)))),
    m_colorSpaceValue(Linear),
    m_debayerValue(NoDebayer),
    m_whiteBalanceValue(Daylight),
    m_clippingValue(ClipAuto)
{
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[Linear]), Linear, true);
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[sRGB]), sRGB);
    m_colorSpace->addOption(DF_TR_AND_C(ColorSpaceStr[IUT_BT_709]), IUT_BT_709);

    m_debayer->addOption(DF_TR_AND_C(DebayerStr[NoDebayer]), NoDebayer, true);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[HalfSize]), HalfSize);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[Low]), Low);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[VNG]), VNG);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[PPG]), PPG);
    m_debayer->addOption(DF_TR_AND_C(DebayerStr[AHD]), AHD);

    m_whiteBalance->addOption(DF_TR_AND_C(WhiteBalanceStr[NoWhiteBalance]), NoWhiteBalance);
    m_whiteBalance->addOption(DF_TR_AND_C(WhiteBalanceStr[RawColors]), RawColors);
    m_whiteBalance->addOption(DF_TR_AND_C(WhiteBalanceStr[Camera]), Camera);
    m_whiteBalance->addOption(DF_TR_AND_C(WhiteBalanceStr[Daylight]), Daylight, true);

    m_clipping->addOption(DF_TR_AND_C(ClippingStr[ClipAuto]), ClipAuto, true);
    m_clipping->addOption(DF_TR_AND_C(ClippingStr[Clip16bit]), Clip16bit);
    m_clipping->addOption(DF_TR_AND_C(ClippingStr[Clip15bit]), Clip15bit);
    m_clipping->addOption(DF_TR_AND_C(ClippingStr[Clip14bit]), Clip14bit);
    m_clipping->addOption(DF_TR_AND_C(ClippingStr[Clip13bit]), Clip13bit);
    m_clipping->addOption(DF_TR_AND_C(ClippingStr[Clip12bit]), Clip12bit);

    addParameter(m_filesCollection);
    addParameter(m_colorSpace);
    addParameter(m_debayer);
    addParameter(m_whiteBalance);
    addParameter(m_clipping);

    addInput(new OperatorInput(tr("Bad pixels map"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("RAWs"), this));
}

OpLoadRaw::~OpLoadRaw()
{
}

OpLoadRaw *OpLoadRaw::newInstance()
{
    return new OpLoadRaw(m_process);
}

QStringList OpLoadRaw::getCollection() const
{
    return m_filesCollection->collection();
}

QString OpLoadRaw::getColorSpace() const
{
    return ColorSpaceStr[m_colorSpaceValue];
}

QString OpLoadRaw::getDebayer() const
{
    return DebayerStr[m_debayerValue];
}

QString OpLoadRaw::getWhiteBalance() const
{
    return WhiteBalanceStr[m_whiteBalanceValue];
}

void OpLoadRaw::setColorSpace(int v)
{
    if ( m_colorSpaceValue != v ) {
        m_colorSpaceValue = ColorSpace(v);
        setOutOfDate();
    }
}

void OpLoadRaw::setDebayer(int v)
{
    if ( m_debayerValue != v ) {
        m_debayerValue = Debayer(v);
        setOutOfDate();
    }
}

void OpLoadRaw::setWhiteBalance(int v)
{
    if ( m_whiteBalanceValue != v ) {
        m_whiteBalanceValue = WhiteBalance(v);
        setOutOfDate();
    }
}

void OpLoadRaw::setClipping(int v)
{
    if ( m_clippingValue != v ) {
        m_clippingValue = Clipping(v);
        setOutOfDate();
    }
}


void OpLoadRaw::filesCollectionChanged()
{
    setOutOfDate();
}

OperatorWorker *OpLoadRaw::newWorker() {
    return new WorkerLoadRaw(m_thread, this);
}
