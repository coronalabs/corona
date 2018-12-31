// RSA public key encryption

#include "vlong.hpp"

class public_key
{
	protected:
		public_key(); // Cannot directly instantiate this

	public:
		public_key( const vlong& inM, const vlong& inE );

	public:
		vlong encrypt( const vlong& plain ) const; // Requires 0 <= plain < m

	public:
		vlong get_m() const { return m; } // product of p and q
		vlong get_e() const { return e; } // exponent

	protected:
		vlong m,e;
};

class private_key : public public_key
{
	public:
		typedef public_key	Super;

	public:
		// Create new key
		// r1 and r2 should be null terminated random strings
		// each of length around 35 characters (for a 500 bit modulus)
		private_key( const char * r1, const char * r2 );

		// Create from existing primes
		private_key( const vlong& inP, vlong& inQ );

	protected:
		void initialize();

	public:
		vlong decrypt( const vlong& cipher ) const;
		vlong get_d() const; // private key
		vlong get_p() const { return p; } // prime #
		vlong get_q() const { return q; } // prime #

	private:
		vlong p,q;
};
