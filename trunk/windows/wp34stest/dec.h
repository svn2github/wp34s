#pragma once

extern "C"
{
#include "decn.h"
};

// use a class rather than tedious decnumber

extern decContext Ctx;

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
        decNumberFromString(&_v, buf, &Ctx);
    }

    // operators
    friend Dec operator+(const Dec& a, const Dec& b)
    { decNumber r; dn_add(&r, &a._v, &b._v); return r; }

    friend Dec operator-(const Dec& a, const Dec& b)
    { decNumber r; dn_subtract(&r, &a._v, &b._v); return r; }

    friend Dec operator*(const Dec& a, const Dec& b)
    { decNumber r; dn_multiply(&r, &a._v, &b._v); return r; }

    friend Dec operator/(const Dec& a, const Dec& b)
    { decNumber r; dn_divide(&r, &a._v, &b._v); return r; }

    Dec& operator+=(const Dec& a)
    { dn_add(&_v, &_v, &a._v);  return *this; }

    Dec& operator-=(const Dec& a)
    { dn_subtract(&_v, &_v, &a._v);  return *this; }

    Dec& operator*=(const Dec& a)
    { dn_multiply(&_v, &_v, &a._v);  return *this; }

    Dec& operator/=(const Dec& a)
    { dn_divide(&_v, &_v, &a._v);  return *this; }

    Dec operator-() const
    { decNumber r; dn_minus(&r, &_v); return r; }

    const char* asString() const
    {
        // value only valid immediately after call and not threadsafe
        static char buf[64];
        decNumberToString(&_v, buf);        
        return buf;
    }

    Dec sqrt() const
    { decNumber r; dn_sqrt(&r, &_v); return r; }

    Dec ln() const { decNumber r; dn_ln(&r, &_v); return r; }
    Dec sin() const { decNumber r; decNumberSin(&r, &_v); return r; }
    Dec cos() const { decNumber r; decNumberCos(&r, &_v); return r; }
    Dec tan() const { decNumber r; decNumberTan(&r, &_v); return r; }
    Dec exp() const { decNumber r; dn_exp(&r, &_v); return r; }

    Dec asin() const { decNumber r; decNumberArcSin(&r, &_v); return r; }
    Dec acos() const { decNumber r; decNumberArcCos(&r, &_v); return r; }
    Dec atan() const { decNumber r; decNumberArcTan(&r, &_v); return r; }

    Dec sinh() const { decNumber r; decNumberSinh(&r, &_v); return r; }
    Dec cosh() const { decNumber r; decNumberCosh(&r, &_v); return r; }
    Dec tanh() const { decNumber r; decNumberTanh(&r, &_v); return r; }
    Dec factorial() const { decNumber r; decNumberFactorial(&r, &_v); return r; }


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
inline Dec sinh(const Dec& v) { return v.sinh(); }
inline Dec cosh(const Dec& v) { return v.cosh(); }
inline Dec tanh(const Dec& v) { return v.tanh(); }
inline Dec factorial(const Dec& v) { return v.factorial(); }

