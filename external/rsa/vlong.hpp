

class flex_unit // Provides storage allocation and index checking
{
  unsigned * a; // array of units
  unsigned z; // units allocated
public:
  unsigned n; // used units (read-only)
  flex_unit();
  ~flex_unit();
  void clear(); // set n to zero
  unsigned get( unsigned i ) const;     // get ith unsigned
  void set( unsigned i, unsigned x );   // set ith unsigned
  void reserve( unsigned x );           // storage hint

  // Time critical routine
  void fast_mul( flex_unit &x, flex_unit &y, unsigned n );
};

class vlong_value : public flex_unit
{
  public:
  unsigned share; // share count, used by vlong to delay physical copying
  int is_zero() const;
  unsigned bit( unsigned i ) const;
  void setbit( unsigned i );
  void clearbit( unsigned i );
  unsigned bits() const;
  int cf( vlong_value& x ) const;
  int product( vlong_value &x ) const;
  void shl();
  int  shr(); // result is carry
  void shr( unsigned n );
  void add( vlong_value& x );
// apps/corona/project/jni/external/rsa/vlong.hpp:37: error: expected unqualified-id before '^' token
// apps/corona/project/jni/external/rsa/vlong.hpp:38: error: expected unqualified-id before '&&' token
// and, xor are reserved C++ keywords. This also breaks in llvm.
void vlong_xor( vlong_value& x );
void vlong_and( vlong_value& x );
  void subtract( vlong_value& x );
  void init( unsigned x );
  void copy( vlong_value& x );
  unsigned to_unsigned(); // Unsafe conversion to unsigned
  vlong_value();
  void mul( vlong_value& x, vlong_value& y );
  void divide( vlong_value& x, vlong_value& y, vlong_value& rem );
};

class vlong // very long integer - can be used like long
{
public:
  // Standard arithmetic operators
  friend vlong operator +( const vlong& x, const vlong& y );
  friend vlong operator -( const vlong& x, const vlong& y );
  friend vlong operator *( const vlong& x, const vlong& y );
  friend vlong operator /( const vlong& x, const vlong& y );
  friend vlong operator %( const vlong& x, const vlong& y );
  friend vlong operator ^( const vlong& x, const vlong& y );
  friend vlong pow2( unsigned n );
  friend vlong operator &( const vlong& x, const vlong& y );

  friend vlong operator <<( const vlong& x, unsigned n );

  vlong& operator +=( const vlong& x );
  vlong& operator -=( const vlong& x );
  vlong& operator >>=( unsigned n );

  // Standard comparison operators
  friend int operator !=( const vlong& x, const vlong& y );
  friend int operator ==( const vlong& x, const vlong& y );
  friend int operator >=( const vlong& x, const vlong& y );
  friend int operator <=( const vlong& x, const vlong& y );
  friend int operator > ( const vlong& x, const vlong& y );
  friend int operator < ( const vlong& x, const vlong& y );

  // Absolute value
  friend vlong abs( const vlong & x );

  // Construction and conversion operations
  vlong ( unsigned x=0 );
  vlong ( const vlong& x );
  ~vlong();
  friend unsigned to_unsigned( const vlong &x );
  vlong& operator =(const vlong& x);

  // Bit operations
  unsigned bits() const;
  unsigned bit(unsigned i) const;
  void setbit(unsigned i);
  void clearbit(unsigned i);
  vlong& operator ^=( const vlong& x );
  vlong& operator &=( const vlong& x );
  vlong& ror( unsigned n ); // single  bit rotate
  vlong& rol( unsigned n ); // single bit rotate
  friend int product( const vlong & x, const vlong & y ); // parity of x&y

  void load( const unsigned * a, unsigned n ); // load value, a[0] is lsw
  void store( unsigned * a, unsigned n ) const; // low level save, a[0] is lsw

  void pad( unsigned n ); // if vlong is < n words, then pad with random numbers

// void loadBytes( const unsigned char * a, unsigned n ); // load value, a[0] is lsw
// void storeBytes( unsigned char * a, unsigned n ) const; // low level save, a[0] is lsw

  unsigned numWords() const { return value->n; }

private:
  class vlong_value * value;
  int negative;
  int cf( const vlong & x ) const;
  void docopy();
  friend class monty;
};

vlong modexp( const vlong & x, const vlong & e, const vlong & m ); // m must be odd

vlong gcd( const vlong &X, const vlong &Y ); // greatest common denominator
vlong modinv( const vlong &a, const vlong &m ); // modular inverse

vlong lucas ( vlong P, vlong Z, vlong k, vlong p ); // P^2 - 4Z != 0
vlong sqrt( vlong g, vlong p ); // square root mod p
