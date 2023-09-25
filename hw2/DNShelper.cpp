#include "pch.h"
#include "DNShelper.h"
#include <iostream>
#include <string>
/* DNS query types */

class FixedDNSheader;
class QueryHeader;




 string DnsHelper::replacePeriodWithNumber( string query )
{
	 this->query = query;
	 string total = "";
	 int find = 0;
	 int indexParse = 0;
	 while (true)
	 {
		 find = query.find('.');
		 if (find == string::npos || find > query.size() )
		 {
			 break;
		 }
		 total = query.substr(indexParse, find);
		 total += find + 1;



	 }
	 this->modifiedQuery = total;
	 return total;
	
}