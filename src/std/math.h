#ifndef STDLIB_MATH_H
    #define STDLIB_MATH_H
    #include <cmath>
    #include <numeric>
    constexpr double _obn_abs(double n)
    {
        return std::abs(n);
    }
    constexpr double _obn_sin(double n)
    {
        return std::sin(n);
    }
    constexpr double _obn_cos(double n)
    {
        return std::cos(n);
    }
    constexpr double _obn_tan(double n)
    {
        return std::tan(n);
    }
    constexpr double _obn_min(double n, double m)
    {
        return std::min(n, m);
    }
    constexpr double _obn_max(double n, double m)
    {
        return std::max(n, m);
    }
    constexpr double _obn_sqrt(double n)
    {
        return std::sqrt(n);
    }
    constexpr double _obn_cbrt(double n)
    {
        return std::cbrt(n);
    }
    constexpr double _obn_hypot(double x, double y)
    {
        return std::hypot(x, y);
    }
    constexpr double _obn_fmod(double n, double m)
    {
        return std::fmod(n, m);
    }
    constexpr double _obn_remainder(double n, double m)
    {
        return std::remainder(n, m);
    }
    constexpr double _obn_copysign(double n, double sign)
    {
        return std::copysign(n, sign);
    }
    constexpr double _obn_floor(double n)
    {
        return std::floor(n);
    }
    constexpr double _obn_ceil(double n)
    {
        return std::ceil(n);
    }
    constexpr double _obn_round(double n, unsigned to = 1)
    {
        return std::round(n / to) * to;
    }
    constexpr double _obn_trunc(double n)
    {
        return std::trunc(n);
    }
    constexpr double _obn_exp(double n)
    {
        return std::exp(n);
    }
    constexpr double _obn_exp2(double n)
    {
        return std::exp2(n);
    }
    constexpr double _obn_expm1(double n)
    {
        return std::expm1(n);
    }
    constexpr double _obn_ln(double n)
    {
        return std::log(n);
    }
    constexpr double _obn_log(double base, double n)
    {
        return std::log(n) / std::log(base);
    }
    constexpr double _obn_log10(double n)
    {
        return std::log10(n);
    }
    constexpr double _obn_log2(double n)
    {
        return std::log2(n);
    }
    constexpr double _obn_log1p(double n)
    {
        return std::log1p(n);
    }
    constexpr double _obn_erf(double n)
    {
        return std::erf(n);
    }
    constexpr double _obn_erfc(double n)
    {
        return std::erfc(n);
    }
    constexpr bool _obn_isfinite(double n)
    {
        return std::isfinite(n);
    }
    constexpr bool _obn_isinf(double n)
    {
        return std::isinf(n);
    }
    constexpr bool _obn_isnan(double n)
    {
        return std::isnan(n);
    }
    constexpr bool _obn_signbit(double n)
    {
        return std::signbit(n);
    }
    constexpr double _obn_asin(double n)
    {
        return std::asin(n);
    }
    constexpr double _obn_acos(double n)
    {
        return std::acos(n);
    }
    constexpr double _obn_atan(double n)
    {
        return std::atan(n);
    }
    constexpr double _obn_atan2(double y, double x)
    {
        return std::atan2(y, x);
    }
    constexpr double _obn_gcd(double n, double m)
    {
        return std::gcd(n, m);
    }
    constexpr double _obn_lcm(double n, double m)
    {
        return std::lcm(n, m);
    }
    constexpr double _obn_sinh(double n)
    {
        return std::sinh(n);
    }
    constexpr double _obn_cosh(double n)
    {
        return std::cosh(n);
    }
    constexpr double _obn_tanh(double n)
    {
        return std::tanh(n);
    }
    constexpr double _obn_asinh(double n)
    {
        return std::asinh(n);
    }
    constexpr double _obn_acosh(double n)
    {
        return std::acosh(n);
    }
    constexpr double _obn_atanh(double n)
    {
        return std::atanh(n);
    }
    constexpr double _obn_factorial(double n)
    {
        return std::tgamma(n + 1);
    }
    constexpr double _obn_E = 2.718281828459045;
    constexpr double _obn_PI = 3.141592653589793;
    constexpr double _obn_ROOT2 = 1.414213562373095;
#endif