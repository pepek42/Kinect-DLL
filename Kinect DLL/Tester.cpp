#include "Tester.h"


Tester::Tester()
{
}


Tester::~Tester()
{
}

void Tester::logToFile(std::string fileName, std::string text){
	std::ofstream outfile;

	outfile.open(fileName, std::ios_base::app);
	outfile << std::endl << text;
}