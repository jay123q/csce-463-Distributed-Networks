/*
Joshua Clapp
Csce 463-500
*/
#include "pch.h"
#include <iostream>
#include <string.h>
#include <ostream>
#include <fstream>

#define DEBUG;

void printer(std::string line)
{

}

int main(void)
{
#ifdef DEBUG
	std::ifstream file;
	file.open("100url.txt");
	std::string fileLine;
	while (!file.eof())
	{
		file >> fileLine;
		printer(fileLine);

	}

#endif // DEBUG

	return 0;
}
