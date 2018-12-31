#include "Core/Rtt_Build.h"

#include "rsa.hpp"
#include "prime.hpp"

public_key::public_key()
{
}

public_key::public_key( const vlong& inM, const vlong& inE )
:	m( inM ),
	e( inE )
{
}

vlong
public_key::encrypt( const vlong& plain ) const
{
	return modexp( plain, e, m );
}

static vlong
from_str( const char * s )
{
	vlong x = 0;
	while (*s)
	{
		x = x * 256 + (unsigned char)*s;
		s += 1;
	}
	return x;
}

private_key::private_key( const char * r1, const char * r2 )
:	Super()
{
	// Choose primes
	{
		prime_factory pf;
		p = pf.find_prime( from_str(r1) );
		q = pf.find_prime( from_str(r2) );
		if ( p > q ) { vlong tmp = p; p = q; q = tmp; }
	}
	initialize();
}

private_key::private_key( const vlong& inP, vlong& inQ )
:	Super(),
	p( inP ),
	q( inQ )
{
	initialize();
}

void
private_key::initialize()
{
	// Calculate public key
	m = p*q;
	e = 50001; // must be odd since p-1 and q-1 are even
	while ( gcd(p-1,e) != 1 || gcd(q-1,e) != 1 ) e += 2;
}

vlong
private_key::decrypt( const vlong& cipher ) const
{
	// Calculate values for performing decryption
	// These could be cached, but the calculation is quite fast
	//vlong d = modinv( e, (p-1)*(q-1) );
	vlong d = get_d();
	vlong u = modinv( p, q );
	vlong dp = d % (p-1);
	vlong dq = d % (q-1);

	// Apply chinese remainder theorem
	vlong a = modexp( cipher % p, dp, p );
	vlong b = modexp( cipher % q, dq, q );
	if ( b < a ) b += q;
	return a + p * ( ((b-a)*u) % q );
}

vlong private_key::get_d() const
{
	return modinv( e, (p-1)*(q-1) );
}

