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
#ifndef MANIPULATION_H
#define MANIPULATION_H

#include <QObject>


template<typename t> t clamp(t v,t min = 0, t max = 65535 /* ARgg! */ ) {
  if ( v < min )
    return min;
  else if ( v > max )
    return max;
  else
    return v;
}

template<typename t> void
kernel_1D_to_2D(const t *in, t *out, int kOrder)
{
   t sum = 0;
   for (int i = 0 ; i < kOrder ; ++i) {
       for (int j = 0 ; j < kOrder ; ++j) {
           //Kronecker product
           sum += out[i*kOrder+j] = in[i]*in[j];
       }
   }
   for (int i = 0, s = kOrder * kOrder ; i < s ; ++i) {
       out[i]/=sum;
   }
}

extern const double b3SplineWavelet[5];
extern const double linearWavelet[3];
extern const double downsampleKernel[4];
extern const double upsampleKernel[2];
class Photo;
namespace Magick {
class Image;
}

class Algorithm : public QObject
{
    Q_OBJECT
public:
    explicit Algorithm(bool alterCurve, QObject *parent);

    virtual void applyOnImage(Magick::Image& image, bool hdr);
    virtual void applyOn(Photo& photo);

signals:

public slots:
protected:
    bool m_alterCurve;
private:
    Q_DISABLE_COPY(Algorithm);
};

#endif // MANIPULATION_H
