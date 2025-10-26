#pragma once

namespace vstd
{
template <typename IntType>
IntType round_down(IntType number, IntType factor) { return number - number % factor; }

template <typename IntType>
IntType round_down_pow2(IntType number, IntType factor) { return number - number & (factor - 1); }

template <typename IntType>
IntType round_up(IntType number, IntType factor) { return number + factor - 1 - (number - 1) % factor; }

template <typename IntType>
IntType round_up_pow2(IntType number, IntType factor)
{
    --factor;
    return (number + factor) & ~factor;
}
}
