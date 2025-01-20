//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ProgramFactory_H__
#define _Rtt_ProgramFactory_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

class Program;

// ----------------------------------------------------------------------------

// ...
class ProgramFactory
{
public:
	ProgramFactory();
	~ProgramFactory();

	// ...
    Program* Create( const char* vertexFunction, const char* vertexSkeleton,
                    const char* fragmentFunction, const char* fragmentSkeleton );

	bool Initialize( const char* vertexSkeletonSource, const char* fragmentSkeletonSource );
	Program *Create( const char* vertexFunction, const char* fragmentFunction );

private:
	char* fVertSource;
	char* fFragSource;
	unsigned int fVertLength;
	unsigned int fFragLength;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ProgramFactory_H__
