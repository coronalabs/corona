#include <iostream>
#include "TimXmlRpc.h"


// main.cpp:   includes both a little unit test and a sample client program.


static void BasicTest()
{
	// Replace this URL with your own URL:
	XmlRpcClient Connection("https://61.95.191.232:9600/arumate/rpc/xmlRpcServer.php");
				// Does anyone know of an XmlRpc server that I could insert here 
				// as a test machine that's always available?
	Connection.setIgnoreCertificateAuthority();

	//  Call:  arumate.getKilowatts(string, integer)   :
	XmlRpcValue args, result;
	args[0] = "test";
	args[1] = 1;

	// Replace this function name with your own:
	if (! Connection.execute("arumate.getKilowatts", args, result)) {
		std::cout << Connection.getError();
	}
	else if (result.getType() == XmlRpcValue::TypeString)
		std::cout << std::string(result);
	else std::cout << "Success\n";
}


static void AdvancedTest()
{
	XmlRpcValue args, result;

	// Passing datums:
	args[0] = "a string";
	args[1] = 1;
	args[2] = true;
	args[3] = 3.14159;
	struct tm timeNow;
	args[4] = &timeNow;

	// Passing an array:
	XmlRpcValue array;
	array[0] = 4;
	array[1] = 5;
	array[2] = 6;
	args[5] = array;
	// Note: if there's a chance that the array contains zero elements,
	// you'll need to call:
	//      array.initAsArray();
	// ...because otherwise the type will never get set to "TypeArray" and
	// the value will be a "TypeInvalid".

	// Passing a struct:
	XmlRpcValue record;
	record["SOURCE"] = "a";
	record["DESTINATION"] = "b";
	record["LENGTH"] = 5;
	args[6] = record;
	// We don't support zero-size struct's...Surely no-one needs these?

	// Make the call:
	XmlRpcClient Connection("https://61.95.191.232:9600/arumate/rpc/xmlRpcServer.php");
	Connection.setIgnoreCertificateAuthority();
	if (! Connection.execute("arumate.getMegawatts", args, result)) {
		std::cout << Connection.getError();
		return;
	}

	// Pull the data out:
	if (result.getType() != XmlRpcValue::TypeStruct) {
		std::cout << "I was expecting a struct.";
		return;
	}
	int i = result["n"];
	std::string s = result["name"];
	array = result["A"];
	for (int i=0; i < array.size(); i++)
		std::cout << (int)array[i] << "\n";
	record = result["subStruct"];
	std::cout << (std::string)record["foo"] << "\n";
}



//---------------------------- main(): -------------------------

int main(int argc, char* argv[])
{
	//XmlRpcUnitTest();
	BasicTest();
	AdvancedTest();
	return 0;
}

