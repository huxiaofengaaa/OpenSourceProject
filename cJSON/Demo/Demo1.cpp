#include "cJSON.h"
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

extern cJSON* getAnDefaultObject();
extern string getAnDefaultString();

int main(int argc, char** argv)
{
	cout << "cJSON version : " << cJSON_Version() << endl; 
	cout << "=============================================" << endl;	
	
	return 0;
}
