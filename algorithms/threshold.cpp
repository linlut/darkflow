#include "threshold.h"

using Magick::Quantum;

Threshold::Threshold(qreal high, qreal low, QObject *parent) :
    LutBased(parent)
{
    quantum_t h = high * QuantumRange;
    quantum_t l = low * QuantumRange;

    quantum_t up = QuantumRange;
    quantum_t down = 0;
    if ( low > high ) {
        down = QuantumRange;
        up = 0;
        quantum_t tmp = h;
        h = l;
        l = tmp;
    }
#pragma omp parallel for
    for ( quantum_t i = 0 ; i <= quantum_t(QuantumRange) ; ++i ) {
        if ( i >= l && i <= h )
            m_lut[i] = up;
        else
            m_lut[i] = down;
    }
}
