#pragma once

extern "C"
{
#include "decn.h"
};

// use a class rather than tedious decnumber

extern decContext* Ctx;

class Dec
{
    
public:

    // constructors
    Dec() {}
    Dec(const decNumber& v) : _v(v) {}
    Dec(int v)
    {
        char buf[32];
        sprintf(buf, "%d", v);
        decNumberFromString(&_v, buf, Ctx);
    }

    // operators
    friend Dec operator+(const Dec& a, const Dec& b)
    { decNumber r; decNumberAdd(&r, &a._v, &b._v, Ctx); return r; }

    friend Dec operator-(const Dec& a, const Dec& b)
    { decNumber r; decNumberSubtract(&r, &a._v, &b._v, Ctx); return r; }

    friend Dec operator*(const Dec& a, const Dec& b)
    { decNumber r; decNumberMultiply(&r, &a._v, &b._v, Ctx); return r; }

    friend Dec operator/(const Dec& a, const Dec& b)
    { decNumber r; decNumberDivide(&r, &a._v, &b._v, Ctx); return r; }

    Dec& operator+=(const Dec& a)
    { decNumberAdd(&_v, &_v, &a._v, Ctx);  return *this; }

    Dec& operator-=(const Dec& a)
    { decNumberSubtract(&_v, &_v, &a._v, Ctx);  return *this; }

    Dec& operator*=(const Dec& a)
    { decNumberMultiply(&_v, &_v, &a._v, Ctx);  return *this; }

    Dec& operator/=(const Dec& a)
    { decNumberDivide(&_v, &_v, &a._v, Ctx);  return *this; }

    Dec operator-() const
    { decNumber r; decNumberMinus(&r, &_v, Ctx); return r; }

    const char* asString() const
    {
        // value only valid immediately after call and not threadsafe
        static char buf[64];
        decNumberToString(&_v, buf);        
        return buf;
    }

    Dec sqrt() const
    { decNumber r; decNumberSquareRoot(&r, &_v, Ctx); return r; }

    Dec ln() const { decNumber r; decNumberLn(&r, &_v, Ctx); return r; }
    Dec sin() const { decNumber r; decNumberSin(&r, &_v, Ctx); return r; }
    Dec cos() const { decNumber r; decNumberCos(&r, &_v, Ctx); return r; }
    Dec tan() const { decNumber r; decNumberTan(&r, &_v, Ctx); return r; }
    Dec exp() const { decNumber r; decNumberExp(&r, &_v, Ctx); return r; }

    Dec asin() const { decNumber r; decNumberArcSin(&r, &_v, Ctx); return r; }
    Dec acos() const { decNumber r; decNumberArcCos(&r, &_v, Ctx); return r; }
    Dec atan() const { decNumber r; decNumberArcTan(&r, &_v, Ctx); return r; }


private:

    decNumber   _v;
};

inline Dec sqrt(const Dec& v) { return v.sqrt(); }
inline Dec ln(const Dec& v) { return v.ln(); }
inline Dec sin(const Dec& v) { return v.sin(); }
inline Dec cos(const Dec& v) { return v.cos(); }
inline Dec tan(const Dec& v) { return v.tan(); }
inline Dec exp(const Dec& v) { return v.exp(); }
inline Dec asin(const Dec& v) { return v.asin(); }
inline Dec acos(const Dec& v) { return v.acos(); }
inline Dec atan(const Dec& v) { return v.atan(); }

