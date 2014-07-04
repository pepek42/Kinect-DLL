#pragma once
#include <string>
#include <fstream>

class Tester
{
public:
	Tester();
	~Tester();

	void static logToFile(std::string fileName, std::string text);
};

