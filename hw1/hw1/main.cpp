/*
Joshua Clapp
Csce 463-500
*/
#include "pch.h"
#include <string>
#include "socket.h"
#include "parsedHTML.h"
#include "HTMLParserBase.h"
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
	/*
	if (argc != 2)
	{
		cout << " invalid form! " << std::endl;
		cout << " should be ./main name.hmtl " << std::endl;
		return 0;
	}
	std::string url = argv[1];
	*/
	


	// this was used for all testing locally
	std::string url = "http://s2.irl.cs.tamu.edu/IRL7";

	
	parsedHtml parser;
	//parser.parseTXTFile(url.argv[1]);
	parser.parseString(url.c_str());


	//if (strlen(parser.host.c_str()) < MAX_HOST_LEN) >????
	//{



		// handle socketing
		Socket* webSocket = new Socket();
		// cout << " whole link " << parser.wholeLink << std::endl;
		parser.generateRequesttoSend("GET");
		bool socketCheck = webSocket->Send(parser.total, parser.wholeLink, parser.host, parser.port, parser.printPathQueryFragment());

		if (socketCheck)
		{
			// now try to read
			if (webSocket->Read() )
			{
				// HERE && strlen(webSocket->printBuf().c_str()) < MAX_REQUEST_LEN ???
				webSocket->closeSocket(); // maybe move this into read? 
				// so now the html should return the buffer soo
				const char * result = webSocket->printBuf();
				// cout << " print the result and junk \n " << webSocket->printBuf() << std::endl;
				// cout << " the result  is " << this->printBuf() << std::endl;
				string status(webSocket->printBuf());
				double statusCode = stod(status.substr(9.3).c_str() );
				cout << "\t   Verifying header... ";
				string stringResult(result);
				// parse header now
				if (statusCode > 199 && statusCode < 300)
				{
					cout << "status code " << statusCode << std::endl;
					clock_t start = clock();
					clock_t finish = clock();
					cout << "\t + Parsing page... ";

					const char* pastHeaderPtr = strchr(result, '\r\n\r\n');

					int bytes_recieved = webSocket->getCurPos();
					int bytes_header = pastHeaderPtr - result; // header bytes
					int bytes_file = bytes_recieved - bytes_header;


					// int numberBytesToParse = htmlPointer - webSocket->getCurPos();
					//asdf
					int nLinks = 0;
					HTMLParserBase htmlLinkRipper;
					char* linkCounter = htmlLinkRipper.Parse((char*)stringResult.c_str(), bytes_file,
						(char*)parser.wholeLink.c_str() , strlen((char*)parser.wholeLink.c_str()), &nLinks);  // 43


					finish = clock();
					double timer = (double)(finish - start) / CLOCKS_PER_SEC;
					printf("done in %.1f ms with %d links\n", timer * 1000, nLinks);
					printf("=======================================================\n");

					// int httpPointer = webSocket->printBuf().find("HTTP/");

				}
				else
				{
					//int skipBadLinks = webSocket->printBuf().find("\r\n\r\n");
					cout << "status code " << statusCode << std::endl;
					printf("=======================================================\n");
					//cout << "web socket " << webSocket->printBuf() << std::endl;

					//cout << " html pointer is " << htmlPointer << std::endl;
				}
				std::string resultButString(result);
				int findHeader = resultButString.find("\r\n\r\n");
				std::string header = resultButString.substr(0, findHeader);
					cout << header;
				// cout << "status code " << statusCode << std::endl;
				// cout << webSocket->printBuf() << std::endl;
				// const char* httpStatus = strstr(webSocket->printBuf(), "HTTP/1.1");
				// llook for \r\n\r\n to parse
				// int skipBadLinks = this->printBuf().find("\r\n\r\n");
				// now modify the pointer/string so we can remove the essentail first half

			}
		}
	//}

	return 0;
}
