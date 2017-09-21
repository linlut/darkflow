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
#include "discretefouriertransform.h"
#include <QObject>
#include <Magick++.h>
#include "photo.h"
#include "algorithm.h"
#include "console.h"
#include "hdr.h"
#include "preferences.h"

using Magick::Quantum;

Q_STATIC_ASSERT( sizeof(fftw_complex) == sizeof(std::complex<double>));

#ifndef ANDROID
static struct RunThisOnce {
    RunThisOnce() {
        fftw_init_threads();
    }
} once;
#endif

DiscreteFourierTransform::DiscreteFourierTransform(Magick::Image &image, Photo::Gamma scale)
    : m_w(image.columns()),
      m_h(image.rows()),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
#ifndef ANDROID
    fftw_plan_with_nthreads(preferences->getNumThreads());
#endif
    std::complex<double> *input = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    //std::complex<double> *output = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    Ordinary::Pixels cache(image);
    const Magick::PixelPacket *pixels = cache.getConst(0, 0, m_w, m_h);
    for (int c = 0 ; c < 3 ; ++c ) {
        for ( int y = 0 ; y < m_h ; ++y ) {
            for ( int x = 0 ; x < m_w ; ++x ) {
                quantum_t p = 0;
                double pixel;
                switch(c) {
                case 0: p = pixels[y*m_w+x].red; break;
                case 1: p = pixels[y*m_w+x].green; break;
                case 2: p = pixels[y*m_w+x].blue; break;
                }
                if ( Photo::HDR == scale )
                    pixel = fromHDR(p)/QuantumRange;
                else
                    pixel = double(p)/QuantumRange;
                input[y*m_w+x] = std::complex<double>(pixel, 0);
            }
        }
        std::complex<double> *plane = 0;
        switch(c) {
        case 0: plane = red; break;
        case 1: plane = green; break;
        case 2: plane = blue; break;
        }
        fftw_plan plan = fftw_plan_dft_2d(m_h, m_w,
                                          reinterpret_cast<fftw_complex*>(input),
                                          reinterpret_cast<fftw_complex*>(plane), FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
//        memcpy(plane, output, sizeof(fftw_complex)*m_h*m_w);
        fftw_destroy_plan(plan);
    }
    //fftw_free(output);
    fftw_free(input);
}

DiscreteFourierTransform::DiscreteFourierTransform(Magick::Image &magnitude,
                                                   Magick::Image &phase,
                                                   Photo::Gamma scale,
                                                   double normalization)
    : m_w(magnitude.columns()),
      m_h(magnitude.rows()),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
    int p_w = phase.columns();
    int p_h = phase.rows();
    Ordinary::Pixels mCache(magnitude);
    Ordinary::Pixels pCache(phase);
    const Magick::PixelPacket *mPixels = mCache.getConst(0, 0, m_w, m_h);
    const Magick::PixelPacket *pPixels = pCache.getConst(0, 0, p_w, p_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            for ( int c = 0 ; c < 3 ; ++c ) {
                quantum_t q_mag = 0;
                quantum_t q_pha = 0;
                int xx = (m_w+x-m_w/2)%m_w;
                int yy = (m_h+y-m_h/2)%m_h;
                int px = xx%p_w;
                int py = yy%p_h;
                std::complex<double> *plane = 0;
                switch (c) {
                case 0: plane = red; q_mag = mPixels[yy*m_w+xx].red; q_pha = pPixels[py*p_w+px].red; break;
                case 1: plane = green; q_mag = mPixels[yy*m_w+xx].green; q_pha = pPixels[py*p_w+px].green; break;
                case 2: plane = blue; q_mag = mPixels[yy*m_w+xx].blue; q_pha = pPixels[py*p_w+px].blue; break;
                }
                double r_mag = normalization * (Photo::HDR == scale ? fromHDR(q_mag) : q_mag);
                double r_pha = (2.*M_PI*double(q_pha)/QuantumRange)-M_PI;
                plane[y*m_w+x] = std::polar(r_mag, r_pha);
            }
        }
    }
}

DiscreteFourierTransform::~DiscreteFourierTransform()
{
    fftw_free(red);
    fftw_free(green);
    fftw_free(blue);
}

Magick::Image DiscreteFourierTransform::reverse(double luminosity, ReverseType type)
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    std::complex<double> *input = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    std::complex<double> *output = reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w));
    for ( int c = 0 ; c < 3 ; ++c ) {
        std::complex<double> *plane = 0;
        switch(c) {
        case 0: plane = red; break;
        case 1: plane = green; break;
        case 2: plane = blue; break;
        }
        memcpy(input, plane, sizeof(fftw_complex)*m_h*m_w);
        fftw_plan plan = fftw_plan_dft_2d(m_h, m_w,
                                          reinterpret_cast<fftw_complex*>(input),
                                          reinterpret_cast<fftw_complex*>(output), FFTW_BACKWARD, FFTW_ESTIMATE);
        fftw_execute(plan);
        for ( int y = 0 ; y < m_h ; ++y ) {
            for ( int x = 0 ; x < m_w ; ++x ) {
                std::complex<double> cV = output[y*m_w+x];
                double v = 0;
                switch (type) {
                case ReverseMagnitude: v = std::abs(cV); break;
                case ReversePhase: v = std::arg(cV); break;
                case ReverseReal: v = cV.real(); break;
                case ReverseImaginary: v = cV.imag(); break;
                }

                quantum_t pixel = clamp(luminosity*v*QuantumRange/(m_w*m_h));
                switch(c) {
                case 0: pixels[y*m_w+x].red = pixel; break;
                case 1: pixels[y*m_w+x].green = pixel; break;
                case 2: pixels[y*m_w+x].blue = pixel; break;
                }
            }
        }
        fftw_destroy_plan(plan);
    }
    cache.sync();
    fftw_free(output);
    fftw_free(input);
    return image;
}

Magick::Image DiscreteFourierTransform::imageMagnitude(Photo::Gamma scale, double *normalizationp)
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    double max = 0;
    for (int i = 0, s = m_w*m_h ; i < s ; ++i) {
        max = qMax(max, qMax(std::abs(red[i]), qMax(std::abs(green[i]), std::abs(blue[i]))));
    }
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            int xx = (x+m_w/2)%m_w;
            int yy = (y+m_h/2)%m_h;
            double r = std::abs(red[y*m_w+x]) * QuantumRange / max,
                   g = std::abs(green[y*m_w+x]) * QuantumRange / max,
                   b = std::abs(blue[y*m_w+x]) * QuantumRange / max;
            if ( scale == Photo::HDR ) {
                pixels[yy*m_w+xx].red = toHDR(r);
                pixels[yy*m_w+xx].green = toHDR(g);
                pixels[yy*m_w+xx].blue = toHDR(b);
            }
            else {
                pixels[yy*m_w+xx].red = clamp<quantum_t>(r);
                pixels[yy*m_w+xx].green = clamp<quantum_t>(g);
                pixels[yy*m_w+xx].blue = clamp<quantum_t>(b);
            }
        }
    }
    cache.sync();
    if (normalizationp)
        *normalizationp = max / QuantumRange;
    return image;
}

Magick::Image DiscreteFourierTransform::imagePhase()
{
    Magick::Image image(Magick::Geometry(m_w, m_h), Magick::Color(0, 0, 0));
    image.modifyImage();
    Ordinary::Pixels cache(image);
    Magick::PixelPacket *pixels = cache.get(0, 0, m_w, m_h);
    for ( int y = 0 ; y < m_h ; ++y ) {
        for ( int x = 0 ; x < m_w ; ++x ) {
            int xx = (x+m_w/2)%m_w;
            int yy = (y+m_h/2)%m_h;
            pixels[yy*m_w+xx].red = clamp<quantum_t>( (std::arg(red[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
            pixels[yy*m_w+xx].green = clamp<quantum_t>( (std::arg(green[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
            pixels[yy*m_w+xx].blue = clamp<quantum_t>( (std::arg(blue[y*m_w+x])+M_PI) /(M_PI*2.) * QuantumRange );
        }
    }
    cache.sync();
    return image;
}

DiscreteFourierTransform &DiscreteFourierTransform::operator/=(const DiscreteFourierTransform &other)
{
    const double min = 1e-12;
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] /=  ( std::abs(other.red[i]) < min ? min : other.red[i]);
        green[i] /= ( std::abs(other.green[i]) < min ? min : other.green[i]);
        blue[i] /= ( std::abs(other.blue[i]) < min ? min : other.blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::operator*=(const DiscreteFourierTransform &other)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] *= other.red[i];
        green[i] *= other.green[i];
        blue[i] *= other.blue[i];
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::conj()
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = std::conj(red[i]);
        green[i] = std::conj(green[i]);
        blue[i] = std::conj(blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::inv()
{
    const double min = 1e-12;
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = 1. / ( std::abs(red[i]) < min ? min : red[i]);
        green[i] = 1. / ( std::abs(green[i]) < min ? min : green[i]);
        blue[i] = 1. / ( std::abs(blue[i]) < min ? min : blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::abs()
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = std::abs(red[i]);
        green[i] = std::abs(green[i]);
        blue[i] = std::abs(blue[i]);
    }
    return *this;
}

DiscreteFourierTransform &DiscreteFourierTransform::wienerFilter(double k)
{
    for (int i = 0, s = m_h*m_w ; i < s ; ++i ) {
        red[i] = std::conj(red[i])/(pow(std::abs(red[i]),2)+k);
        green[i] = std::conj(green[i])/(pow(std::abs(green[i]),2)+k);
        blue[i] = std::conj(blue[i])/(pow(std::abs(blue[i]),2)+k);
    }
    return *this;
}

DiscreteFourierTransform::DiscreteFourierTransform(const DiscreteFourierTransform &other)
    : m_w(other.m_w),
      m_h(other.m_h),
      red(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      green(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w))),
      blue(reinterpret_cast<std::complex<double>*>(fftw_alloc_complex(m_h*m_w)))
{
    memcpy(red, other.red, sizeof(fftw_complex)*m_h*m_w);
    memcpy(green, other.green, sizeof(fftw_complex)*m_h*m_w);
    memcpy(blue, other.blue, sizeof(fftw_complex)*m_h*m_w);
}

Magick::Image DiscreteFourierTransform::normalize(Magick::Image &image, int w, bool center)
{
    int h = w;
    int k_w = image.columns();
    int k_h = image.rows();
    Magick::Image nk(Magick::Geometry(w, h), Magick::Color(0,0,0));
    int o_x = (w-k_w)/2;
    int o_y = (h-k_h)/2;
    std::shared_ptr<Ordinary::Pixels> i_cache(new Ordinary::Pixels(image));
    std::shared_ptr<Ordinary::Pixels> n_cache(new Ordinary::Pixels(nk));
    dfl_parallel_for(y, 0, k_h, 4, (image, nk), {
        const Magick::PixelPacket * k_pixel = i_cache->getConst(0, y, k_w, 1);
        Magick::PixelPacket * n_pixel;
        if (center)
            n_pixel = n_cache->get(o_x, o_y+y, k_w, 1);
        else
            n_pixel = n_cache->get(0, y, k_w, 1);
        for ( int x = 0 ; x < k_w ; ++x ) {
            n_pixel[x] = k_pixel[x];
        }
        n_cache->sync();
    });
    return nk;
}

Magick::Image DiscreteFourierTransform::roll(Magick::Image &image, int o_x, int o_y)
{
    int w = image.columns();
    int h = image.rows();
    Magick::Image nk(Magick::Geometry(w, h), Magick::Color(0,0,0));
    std::shared_ptr<Ordinary::Pixels> i_cache(new Ordinary::Pixels(image));
    std::shared_ptr<Ordinary::Pixels> n_cache(new Ordinary::Pixels(nk));
    dfl_parallel_for(y, 0, h, 4, (image, nk), {
        const Magick::PixelPacket * k_pixel = i_cache->getConst(0, y, w, 1);
        Magick::PixelPacket * n_pixel= n_cache->get(0, (y+o_y+h)%h, w, 1);
        for ( int x = 0 ; x < w ; ++x ) {
            n_pixel[(x+o_x+w)%w] = k_pixel[x];
        }
        n_cache->sync();
    });
    return nk;
}

static double
Hamming(int n, int N)
{
    const double alpha = .54;
    const double beta = 1. - .54;
    return alpha - beta * cos ((2. * M_PI * n)/double(N - 1));
}

static double
Hann(int n, int N)
{
    return 0.5 * ( 1. - cos ((2. * M_PI * n)/double(N - 1)) );
}


static double
None(int, int)
{
    throw 0;
}

static double
Nuttal(int n, int N)
{
    const double a0 = 0.355768;
    const double a1 = 0.487396;
    const double a2 = 0.144232;
    const double a3 = 0.012604;
    return a0
            - a1 * cos ((2. * M_PI * n)/double(N - 1))
            + a2 * cos ((4. * M_PI * n)/double(N - 1))
            - a3 * cos ((6. * M_PI * n)/double(N - 1));
}

static double
BlackmanNuttal(int n, int N)
{
    const double a0 = 0.3635819;
    const double a1 = 0.4891775;
    const double a2 = 0.1365995;
    const double a3 = 0.0106411;
    return a0
            - a1 * cos ((2. * M_PI * n)/double(N - 1))
            + a2 * cos ((4. * M_PI * n)/double(N - 1))
            - a3 * cos ((6. * M_PI * n)/double(N - 1));
}

static double
BlackmanHarris(int n, int N)
{
    const double a0 = 0.35875;
    const double a1 = 0.48829;
    const double a2 = 0.14128;
    const double a3 = 0.01168;
    return a0
            - a1 * cos ((2. * M_PI * n)/double(N - 1))
            + a2 * cos ((4. * M_PI * n)/double(N - 1))
            - a3 * cos ((6. * M_PI * n)/double(N - 1));
}

static double
windowFunction(DiscreteFourierTransform::WindowFunction function, int n, int N, double opening)
{
    double (*func)(int, int) = None;
    switch(function) {
    default:
    case DiscreteFourierTransform::WindowNone: return 1;
    case DiscreteFourierTransform::WindowHamming: func = Hamming; break;
    case DiscreteFourierTransform::WindowHann: func = Hann; break;
    case DiscreteFourierTransform::WindowNuttal: func = Nuttal; break;
    case DiscreteFourierTransform::WindowBlackmanNuttal: func = BlackmanNuttal; break;
    case DiscreteFourierTransform::WindowBlackmanHarris: func = BlackmanHarris; break;
    }

    int m = (1-opening) * N;

    if ( n < m/2)
        return func(n, m);
    else if ( n >= m/2 && n < N-(m/2) )
        return 1;
    else
        return func(N-n, m);
    /*
    int m = opening * N/2;
    if ( n >= m && n < (N-m))
        return 1;
    if ( n < m )
        return func(n, 2*m);
    n = N/2 - N-n;
    return func(n, 2*m);
    */
}

Magick::Image DiscreteFourierTransform::window(Magick::Image& image,
                                               Photo::Gamma scale,
                                               DiscreteFourierTransform::WindowFunction function,
                                               double opening)
{
    int w = image.columns();
    int h = image.rows();
    Magick::Image dstImage(Magick::Geometry(w, h), Magick::Color(0, 0, 0));
    std::shared_ptr<Ordinary::Pixels> srcCache(new Ordinary::Pixels(image));
    std::shared_ptr<Ordinary::Pixels> dstCache(new Ordinary::Pixels(dstImage));
    dfl_parallel_for(y, 0, h, 4, (image, dstImage), {
                         const Magick::PixelPacket *srcPixels = srcCache->getConst(0, y, w, 1);
                         Magick::PixelPacket *dstPixels = dstCache->get(0, y, w, 1);
                         for (int x = 0 ; x < w ; ++x) {
                             double coef = windowFunction(function, x, w, opening) *
                                           windowFunction(function, y, h, opening);
                             Triplet<double> rgb;
                             if ( scale == Photo::HDR ) {
                                 rgb.red = fromHDR(srcPixels[x].red);
                                 rgb.green = fromHDR(srcPixels[x].green);
                                 rgb.blue = fromHDR(srcPixels[x].blue);
                             }
                             else {
                                 rgb.red = srcPixels[x].red;
                                 rgb.green = srcPixels[x].green;
                                 rgb.blue = srcPixels[x].blue;
                             }
                             rgb *=  coef;
                             if ( scale == Photo::HDR ) {
                                 dstPixels[x].red = toHDR(rgb.red);
                                 dstPixels[x].green = toHDR(rgb.green);
                                 dstPixels[x].blue = toHDR(rgb.blue);
                             }
                             else {
                                 dstPixels[x].red = clamp<quantum_t>(DF_ROUND(rgb.red));
                                 dstPixels[x].green = clamp<quantum_t>(DF_ROUND(rgb.green));
                                 dstPixels[x].blue = clamp<quantum_t>(DF_ROUND(rgb.blue));
                             }
                         }
                         dstCache->sync();
                     });
    return dstImage;
}
