class prime_factory
{
  public:
  unsigned np;
  unsigned *pl;
  prime_factory( unsigned MP = 2000 ); // sieve size
  ~prime_factory();
  vlong find_prime( vlong start );
  int make_prime( vlong & r, vlong &k, const vlong & rmin );
  vlong find_special( vlong start, vlong base=0 );
};

int is_probable_prime( const vlong &p );


