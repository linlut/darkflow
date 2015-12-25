#ifndef HDR_H
#define HDR_H

#include "lutbased.h"

static inline
quantum_t toHDR(double v)
{
#if 1
  if ( v <= 0 ) return 0;
  return clamp<quantum_t>(round(log2(1+v)*4096-1));
#else
    if ( v < 1 ) return 0;
    return clamp<quantum_t>(round(log2(v)*4096));
#endif
}

static inline
double fromHDR(quantum_t v)
{
#if 1
  if ( v == 0 ) return 0;
  return pow(2,(double)(v+1)/4096)-1;
#else
    if ( v == 0 ) return 0;
    return pow(2,(double)(v)/4096);
#endif
}

class HDR : public LutBased
{
    Q_OBJECT
public:
    HDR(bool revert, QObject *parent = 0);
};

#endif // HDR_H