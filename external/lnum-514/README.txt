
==============
  LNUM PATCH
==============

Copyright (c) 2006-08, Asko Kauppi <akauppi@gmail.com>

The LNUM patch, also known as "integer patch" allows Lua 5.1 to treat pure
integer operations with full accuracy and more speed on non-FP platforms,
without changing the external interfaces.

Lua's internal number mode can be easily set to a combination of any of the 
following:

Floating point values ('lua_Number'):

  LNUM_DOUBLE (default)         double precision FP (52 bit mantissa)
  LNUM_FLOAT                    single precision FP (23 bit mantissa)
  LNUM_LDOUBLE                  extra precision FP (80+ bit mantissa, varies)

Integer values ('lua_Integer'):

  LNUM_INT32                    -2^31..2^31-1 integer optimized
  LNUM_INT64                    -2^63..2^63-1 integer optimized

Complex numbers (together with any of the above):

  LNUM_COMPLEX                  Complex (a+bi) number mode (needs C99)

One shall combine these defines, i.e. LNUM_DOUBLE and LNUM_INT64 gives normal
Lua range, with signed 64-bit integer accuracy. The number mode used shall be
written into the 'luaconf.h' file so that it remains consistent for applications
using Lua.

With complex numbers, the general number type gives type for both real and
imaginary components. Integer number type will be applied for scalar integers.


  Implications of using the patch
===================================

Using the patch does NOT change Lua's external behaviour in any way, only 
numerical accuracy is enhanced (depends on modes selected). On non-FPU platforms
a performance boost of around 30% can be expected, depending on the application
in question (for loops and integer table references will see most benefits). 
On FPU platforms, performance should remain the same as a non-patched Lua's.
See attached performance results for real world results.


  Changes in LNUM2 revise
===========================

Since LNUM2, results of FP operations (where one or all parameters are FP) are
no longer checked for producing an integer result. The result is stored in the
FP part of a Lua value, which may in certain configurations (double+int64,
float+int32/64) cause accuracy loss. 

Example:
    a= 2.5
    b= a*a  -- FP result, gets stored as FP even though value is 5
    ...
    b= b+1  -- still FP since 'b' is FP

Normally, this does not matter. Typically operations are dealing with either
floats or integers, and hardly ever need to bring things back to high accuracy
mode. Also, using configuration s.a. 'double+int32' (the default) this whole
issue is not there, since 'double' is able to carry any integer value anyways.

The FP conversion may happen via integer operations, too:

    a= 5
    b= a/2  -- 2.5 is FP
    c= b*2  -- 5.0 still FP
    d= tonumber(c)  - 5 stored as integer

Note that 'c' and 'd' are equal numbers above, although they are internally
stored differently. If one needs to ensure a value is stored (back) as integer,
this can be done via 'tonumber()' as shown.


  Use of unsigned 32/64-bit integers
======================================

Even though the optimized (bit accurate) integer range is signed, using 
hexadecimal notation for the values, they will behave as unsigned 0 .. 
0x[ffffffff]ffffffff range. Care has to be applied, though, and no arithmetic
applied on the values; they are stored as signed values internally.

To output the values, use 'string.format('%x',v)'.


  Use with -ffast-math flag
=============================

LNUM patch tries to make use of gcc '-ffast-math' (or other non-IEEE floating
point math) seamless for Lua users. Plain Lua would have issues with it, i.e.
in dealing with NaN. Grep '__FAST_MATH__' to see the affected places.

'-ffast-math' improves performance on x86 platform essentially (around 10%).
Intel C++ compiler is said to have it on by default.


  Platform notes: Windows
===========================

The Visual C++ 2008 Express development environment does not have full support
for number modes other than LNUM_DOUBLE. What they lack is 'strtof()' and
'strtold()' functions, or equivalents.

Users wishing to use LNUM_LDOUBLE should either compile using 'MinGW' or 
provide their own 'strtold()' function.


  Additions to Lua VM and Lua/C API
=====================================

- integer constant VM instruction

This allows integer constants within a Lua script not to lose accuracy,
regardless of the FP number mode used.

- '_LNUM' global that carries the number mode as a string:
"[complex] double|float|ldouble [int32|int64]"
    
This is mostly for testing purposes, normally an application would not care
to check what the number mode is (or, it can be checked as numerical expressions,
but that is a bit tricky).

The same string is also appeneded to the Lua "opening screen" printout,
alongside version, copyright etc.

- 'lua_isinteger()' addition to the API

To accompany 'lua_tointeger()' already in the API (Lua 5.1).

- 'lua_tocomplex()', 'lua_iscomplex()' and 'luaL_checkcomplex()' consistent
with the number and integer counterparts (only available with LNUM_COMPLEX,
of course).

- 'file:read()' has a new mode "i" for reading in integers. Reading as "n" is
still valid, but may cause accuracy issues (floating point accuracy used).

- Following C99 complex functions added to 'math' namespace:

    complex= math.acosh()
    complex= math.asinh()
    complex= math.atanh()
    scalar= math.arg()
    scalar= math.imag(c)
    scalar= math.real(c)
    complex= math.conj(c)
    complex= math.proj(c)

Note that using LNUM_COMPLEX does _not_ break Lua API that uses 'lua_Number'. 
It remains a scalar, and thus existing Lua modules should be usable also with
Complex internal number mode (of course, as long as imaginary numbers are not
fed to such modules, which will cause an error).

The LNUM_COMPLEX code relies exclusively on the C99 <complex.h> implementation
of complex numbers. Thus, in order to use LNUM_COMPLEX, also enable C99 mode
(i.e. '--std=c99' in gcc).

- 'math.hugeint' giving the largest accurately stored integer value

- '0x...' number constants that are larger than 'lua_Integer' range are NOT
read in as floating point values. Unpatched Lua does so, at least on some
platforms (really depends on the underlying 'strtod()' implementation; i.e.
Visual C++ 2008 does not read in '0x...' as a floating point value).


  Use of the patch
====================

  tar xvzf lua-5.1.4
  cd lua-5.1.4/src
  patch < ../../lua-5.1.4-lnum.patch
  cd ..
  make linux / make macosx / your regular Lua make spell :)

This will make a default (double+int32) configuration. To use others, edit
lines in 'luaconf.h' to include:

  #define LNUM_INT64
  #define LNUM_FLOAT / LNUM_LDOUBLE
  #define LNUM_COMPLEX


  Remake of the patch (for developers)
========================================

  make diff PATCH=xxx.patch


-- AKa 9-Jan-2007 (started 26.8.2006) (updated 13 Aug 2007) (updated 1-2 Oct 2007)
                    (updated 28 Feb 2008) (added 23 Apr 2008)
