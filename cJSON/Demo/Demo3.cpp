#include "cJSON.h"
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

extern cJSON* getAnDefaultObject();
extern string getAnDefaultString();

int main(int argc, char** argv)
{
	string str = getAnDefaultString();
	cJSON* obj = cJSON_Parse(str.c_str());
	if(obj)
	{
		cout << cJSON_Print(obj) << endl;
		cJSON_Delete(obj);
	}
	cout << "=============================================" << endl;
	
	return 0;
}