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
#include "operatorparameterslider.h"
#include "operatorparameterdropdown.h"
#include "operatorinput.h"
#include "operatoroutput.h"
#include "opintegration.h"
#include "workerintegration.h"
#include "Magick++.h"

static const char *RejectionTypeStr[] = {
    QT_TRANSLATE_NOOP("OpIntegration", "None"),
    QT_TRANSLATE_NOOP("OpIntegration", "Sigma clipping"),
    QT_TRANSLATE_NOOP("OpIntegration", "Winsorized"),
    QT_TRANSLATE_NOOP("OpIntegration", "Median Percentil")
};
static const char *NormalizationTypeStr[] = {
    QT_TRANSLATE_NOOP("OpIntegration", "None"),
    QT_TRANSLATE_NOOP("OpIntegration", "Highest Value"),
    QT_TRANSLATE_NOOP("OpIntegration", "Custom")
};

using Magick::Quantum;

OpIntegration::OpIntegration(Process *parent) :
    Operator(OP_SECTION_BLEND, QT_TRANSLATE_NOOP("Operator", "Integration"), Operator::All, parent),
    m_rejectionType(NoRejection),
    m_rejectionTypeDropDown(new OperatorParameterDropDown("rejectionType", tr("Rejection"), this, SLOT(setRejectionType(int)))),
    m_upper(new OperatorParameterSlider("upper", tr("Upper mul."), tr("Integration Upper Limit"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_lower(new OperatorParameterSlider("lower", tr("Lower div."), tr("Integration Lower Limit"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1./(1<<4), 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_normalizationType(NoNormalization),
    m_normalizationTypeDropDown(new OperatorParameterDropDown("normalizationType", tr("Normalization"), this, SLOT(setNormalizationType(int)))),
    m_customNormalization(new OperatorParameterSlider("normalizationValue", tr("Custom Norm."), tr("Integration Custom Normalization"), Slider::ExposureValue, Slider::Logarithmic, Slider::Real, 1, 1<<4, 1, 1./QuantumRange, QuantumRange, Slider::FilterExposureFromOne, this)),
    m_outputHDR(new OperatorParameterDropDown("outputHDR", tr("Output HDR"), this, SLOT(setOutputHDR(int)))),
    m_outputHDRValue(false)
{
    addInput(new OperatorInput(tr("Images"), OperatorInput::Set, this));
    addOutput(new OperatorOutput(tr("Integrated Image"), this));

    m_rejectionTypeDropDown->addOption(DF_TR_AND_C(RejectionTypeStr[NoRejection]), NoRejection, true);
    m_rejectionTypeDropDown->addOption(DF_TR_AND_C(RejectionTypeStr[SigmaClipping]), SigmaClipping);
    m_rejectionTypeDropDown->addOption(DF_TR_AND_C(RejectionTypeStr[Winsorized]), Winsorized);
    m_rejectionTypeDropDown->addOption(DF_TR_AND_C(RejectionTypeStr[MedianPercentil]), MedianPercentil);

    m_normalizationTypeDropDown->addOption(DF_TR_AND_C(NormalizationTypeStr[NoNormalization]), NoNormalization, true);
    m_normalizationTypeDropDown->addOption(DF_TR_AND_C(NormalizationTypeStr[HighestValue]), HighestValue);
    m_normalizationTypeDropDown->addOption(DF_TR_AND_C(NormalizationTypeStr[Custom]), Custom);

    m_outputHDR->addOption(DF_TR_AND_C("No"), false, true);
    m_outputHDR->addOption(DF_TR_AND_C("Yes"), true);

    addParameter(m_rejectionTypeDropDown);
    addParameter(m_upper);
    addParameter(m_lower);
    addParameter(m_normalizationTypeDropDown);
    addParameter(m_customNormalization);
    addParameter(m_outputHDR);
}

OpIntegration *OpIntegration::newInstance()
{
    return new OpIntegration(m_process);
}

OperatorWorker *OpIntegration::newWorker()
{
    return new WorkerIntegration(m_rejectionType,
                                 m_upper->value(),
                                 m_lower->value(),
                                 m_normalizationType,
                                 m_customNormalization->value(),
                                 m_outputHDRValue,
                                 m_thread, this);
}

void OpIntegration::setRejectionType(int type)
{
    if ( m_rejectionType != type ) {
        m_rejectionType = RejectionType(type);
        setOutOfDate();
    }
}

void OpIntegration::setNormalizationType(int type)
{
    if ( m_normalizationType != type ) {
        m_normalizationType = NormalizationType(type);
        setOutOfDate();
    }
}

void OpIntegration::setOutputHDR(int type)
{
    if ( m_outputHDRValue != !!type ) {
        m_outputHDRValue = !!type;
        setOutOfDate();
    }
}
