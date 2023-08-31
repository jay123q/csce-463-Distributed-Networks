/*
Joshua Clapp
Csce 463-500
*/
#include "pch.h"
#include <string>
#include "socket.h"
#include "parsedHTML.h"
#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;
/*

*/

void winsock_test(void);





int main(int argc, char* argv[])
{

	// ayo these are guess, I cant get the vs community to open the properties tab and then show the defaul to throw in the texts
	// no idea tf is happening.
	int numberThreads = 0;
	bool runVector = false;
	parsedHtml parser;

	// std::string url = "http://s2.irl.cs.tamu.edu/IRL8";
	/*
	if (argc == 2)
	{
		std::string filename = argv[1];
	}
	else if (argc == 3)
	{
		numberThreads = stoi(argv[1]);
		std::string filename = argv[2];
		std::vector<std::string> totalVector = parser.parseTXTFile(filename);
		for (int i = 0; i < totalVector.size() ; i++)
		{

		}

	}
	else
	{
		cout << " invalid form! " << std::endl;
		cout << " should be ./main name.hmtl " << std::endl;
		return 0;
	}
	


	*/
	
	parser.parseTXTFile("100url.txt");

	// this was used for all testing locally


	
	//parser.parseTXTFile(url.argv[1]);

	//}

	return 0;
}
