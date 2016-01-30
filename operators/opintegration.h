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
    void setRejectionType(int type);

    void setNormalizationType(int type);
    void setOutputHDR(int type);

private:
    RejectionType m_rejectionType;
    OperatorParameterDropDown *m_rejectionTypeDropDown;
    OperatorParameterSlider *m_upper;
    OperatorParameterSlider *m_lower;
    NormalizationType m_normalizationType;
    OperatorParameterDropDown *m_normalizationTypeDropDown;
    OperatorParameterSlider *m_customNormalization;
    OperatorParameterDropDown *m_outputHDR;
    bool m_outputHDRValue;

};

#endif // OPINTEGRATION_H
