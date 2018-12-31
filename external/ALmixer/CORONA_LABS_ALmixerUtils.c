#include <stdlib.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

# if 0
#
#	// Output debug information.
#	define _fprintf( ... )	fprintf( __VA_ARGS__ )
#
# else
#
#	// Don't output debug information.
#	define _fprintf( ... )
#
# endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void _SafeFree( const char *function_name,
				unsigned int line_number,
				void **p )
{
	if( p && *p )
	{
		free( *p );
		*p = NULL;
	}
	else
	{
		_fprintf( stderr,
					"%s() line %u: Invalid pointer: %p -> %p\n",
					function_name,
					line_number,
					p,
					( p ? *p : 0 ) );
	}
}

#define _free( p ) \
	do \
	{ \
		_SafeFree( __FUNCTION__, \
					__LINE__, \
					(void *)&( p ) ); \
	} \
	while( 0 )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void wrapper_alGenBuffers( const char *function_name,
								unsigned int line_number,
								ALsizei n,
								ALuint* buffers )
{
	ALenum error;

	assert( n );
	assert( buffers );

	alGenBuffers( n,
					buffers );

	if( ( error = alGetError() ) == AL_NO_ERROR )
	{
		ALsizei i = 0;

		for( ;
				i < n;
				++i )
		{
			_fprintf( stderr,
						"%s() line %u: alGenBuffers() OK: %u\n",
						function_name,
						line_number,
						buffers[ i ] );
		}
	}
	else
	{
		_fprintf( stderr,
					"%s() line %u: alGenBuffers() error: %s\n",
					function_name,
					line_number,
					alGetString( error ) );
	}
}

void wrapper_alGenSources( const char *function_name,
								unsigned int line_number,
								ALsizei n,
								ALuint* sources )
{
	ALenum error;

	assert( n );
	assert( sources );

	alGenSources( n,
					sources );

	if( ( error = alGetError() ) == AL_NO_ERROR )
	{
		ALsizei i = 0;

		for( ;
				i < n;
				++i )
		{
			_fprintf( stderr,
						"%s() line %u: alGenSources() OK: %u\n",
						function_name,
						line_number,
						sources[ i ] );
		}
	}
	else
	{
		_fprintf( stderr,
					"%s() line %u: alGenSources() error: %s\n",
					function_name,
					line_number,
					alGetString( error ) );
	}
}

void wrapper_alDeleteBuffers( const char *function_name,
								unsigned int line_number,
								ALsizei n,
								const ALuint* buffers )
{
	if( alIsBuffer( *buffers ) )
	{
		ALenum error;

		alDeleteBuffers( n,
							buffers );

		if( ( error = alGetError() ) == AL_NO_ERROR )
		{
			_fprintf( stderr,
						"%s() line %u: alDeleteBuffers() buffer id: %u, OK.\n",
						function_name,
						line_number,
						*buffers );
		}
		else
		{
			_fprintf( stderr,
						"%s() line %u: alDeleteBuffers() buffer id: %u, error: %s\n",
						function_name,
						line_number,
						*buffers,
						alGetString( error ) );
		}
	}
	else
	{
		_fprintf( stderr,
					"%s() line %u: Can't alDeleteBuffers() non-buffer id: %u.\n",
					function_name,
					line_number,
					*buffers );
	}
}

void wrapper_alDeleteSources( const char *function_name,
								unsigned int line_number,
								ALsizei n,
								const ALuint* sources )
{
	if( alIsSource( *sources ) )
	{
		ALenum error;

		alDeleteSources( 1,
							sources );

		if( ( error = alGetError() ) == AL_NO_ERROR )
		{
			_fprintf( stderr,
						"%s() line %u: alDeleteSources() source id: %u, OK.\n",
						function_name,
						line_number,
						*sources );
		}
		else
		{
			_fprintf( stderr,
						"%s() line %u: alDeleteSources() source id: %u, error: %s\n",
						function_name,
						line_number,
						*sources,
						alGetString( error ) );
		}
	}
	else
	{
		_fprintf( stderr,
					"%s() line %u: Can't alDeleteSources() non-source id: %u.\n",
					function_name,
					line_number,
					*sources );
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define _alGenBuffers( n, buffers ) \
	do \
	{ \
		wrapper_alGenBuffers( __FUNCTION__, \
								__LINE__, \
								( n ), \
								( buffers ) ); \
	} \
	while( 0 )

#define _alGenSources( n, sources ) \
	do \
	{ \
		wrapper_alGenSources( __FUNCTION__, \
								__LINE__, \
								( n ), \
								( sources ) ); \
	} \
	while( 0 )

#define _alDeleteBuffers( n, buffers ) \
	do \
	{ \
		wrapper_alDeleteBuffers( __FUNCTION__, \
									__LINE__, \
									( n ), \
									( buffers ) ); \
	} \
	while( 0 )

#define _alDeleteSources( n, sources ) \
	do \
	{ \
		wrapper_alDeleteSources( __FUNCTION__, \
									__LINE__, \
									( n ), \
									( sources ) ); \
	} \
	while( 0 )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
