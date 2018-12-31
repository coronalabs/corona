// prime factory implementation

#include "vlong.hpp"
#include "prime.hpp"
#include "stdlib.h"
#include "time.h"
#include "stdio.h"

extern void print( vlong x );

static int maybe_prime( const vlong & p )
{
  return modexp( 2, p-1, p ) == 1;
}

static int fermat_is_probable_prime( const vlong &p )
{
  // Test based on Fermats theorem a**(p-1) = 1 mod p for prime p
  // For 1000 bit numbers this can take quite a while
  const unsigned rep = 4;
  const unsigned any[rep] = { 2,3,5,7 /*,11,13,17,19,23,29,31,37..*/ };
  for ( unsigned i=0; i<rep; i+=1 )
    if ( modexp( any[i], p-1, p ) != 1 )
      return 0;
  return 1;
}

static vlong random( const vlong & n )
{
  vlong x = 0;
  while (x < n )
    x = x * RAND_MAX + rand();
  return x % n;
}

static int miller_rabin_is_probable_prime( const vlong & n )
{
  srand( (unsigned) time(0) );

  unsigned T = 100;
  vlong w = n-1;
  unsigned v = 0;
  while ( w % 2 != 0 ) 
  {
    v += 1;
    w = w / 2;
  }
  for (unsigned j=1;j<=T;j+=1)
  {
    vlong a = 1 + random( n );
    vlong b = modexp( a, w, n );
    if ( b != 1 && b != n-1 )
    {
      unsigned i = 1;
      while (1)
      {
        if ( i == v ) return 0;
        b = (b*b) % n;
        if ( b == n-1 ) break;
        if ( b == 1 ) return 0;
        i += 1;
      }
    }
  }
  return 1;
}

int is_probable_prime( const vlong & n )
{
  return fermat_is_probable_prime(n) &&
    miller_rabin_is_probable_prime(n);
}

prime_factory::prime_factory( unsigned MP )
{
  np = 0;

  // Initialise pl
  char * b = new char[MP+1]; // one extra to stop search
  for (unsigned i=0;i<=MP;i+=1) b[i] = 1;
  unsigned p = 2;
  while (1)
  {
    // skip composites
    while ( b[p] == 0 ) p += 1;
    if ( p == MP ) break;
    np += 1;
    // cross off multiples
    unsigned c = p*2;
    while ( c < MP )
    {
      b[c] = 0;
      c += p;
    }
    p += 1;
  }
  pl = new unsigned[np];
  np = 0; for (p=2;p<MP;p+=1) if ( b[p] ) { pl[np] = p; np += 1; }
  delete [] b;
}

prime_factory::~prime_factory()
{
  delete [] pl;
}

vlong prime_factory::find_prime( vlong start )
{
  unsigned SS = 1000; // should be enough unless we are unlucky
  char * b = new char[SS]; // bitset of candidate primes
  while (1)
  {
    unsigned i;
    for (i=0;i<SS;i+=1)
      b[i] = 1;
    for (i=0;i<np;i+=1)
    {
      unsigned p = pl[i];
      unsigned r = to_unsigned(start % p); // not as fast as it should be - could do with special routine
      if (r) r = p - r;
      // cross off multiples of p
      while ( r < SS )
      {
        b[r] = 0;
        r += p;
      }
    }
    // now test candidates
    for (i=0;i<SS;i+=1)
    {
      if ( b[i] )
      {        
        if ( is_probable_prime(start) )
        {
          delete [] b;
          return start;
        }
      }
      start += 1;
    }
  }
}

vlong prime_factory::find_special( vlong start, vlong base )
{
  // Returns a (probable) prime number x > start such that
  // x and x*2+1 are both probable primes,
  // i.e. x is a probable Sophie-Germain prime
  unsigned SS = 40000; // should be enough unless we are unlucky
  char * b1 = new char[SS]; // bitset of candidate primes
  char * b2 = new char[2*SS];
  while (1)
  {
    unsigned i;
    for (i=0;i<SS;i+=1)
      b1[i] = 1;
    for (i=0;i<2*SS;i+=1)
      b2[i] = 1;    
    for (i=0;i<np;i+=1)
    {      
      unsigned p = pl[i]; 
      unsigned r = to_unsigned(start % p); // not as fast as it should be - could do with special routine
      if (r) r = p - r;
      // cross off multiples of p
      while ( r < SS )
      {
        b1[r] = 0;
        r += p;
      }
      r = to_unsigned( (start*2+1) % p );
      if (r) r = p - r;
      while ( r < 2*SS )
      {
        b2[r] = 0;
        r += p;
      }
    }
    // now test candidates
    for (i=0;i<SS;i+=1)
    {
      if ( b1[i] && b2[i*2] )
      {           
        printf("D=%u\n", to_unsigned(start*2+1-base));
        if ( maybe_prime(start)
             && maybe_prime(start*2+1) 
             && is_probable_prime(start) 
             && is_probable_prime(start*2+1)
           )
        {
          delete [] b1;
          delete [] b2;
          return start;
        }
      }
      start += 1;
    }
  }
}

int prime_factory::make_prime( vlong & r, vlong &k, const vlong & min_p )
// Divide out small factors or r
{
  k = 1;
  for (unsigned i=0;i<np;i+=1)
  {
     unsigned p = pl[i];
     // maybe pre-computing product of several primes
     // and then GCD(r,p) would be faster ?
     while ( r % p == 0 )
     {
       if ( r == p )
         return 1; // can only happen if min_p is small
       r = r / p;
       k = k * p;
       if ( r < min_p )
         return 0;
     }
  }
  return is_probable_prime( r );
}
