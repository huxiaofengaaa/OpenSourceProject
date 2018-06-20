#include "cJSON.h"
#include <iostream>
#include <string>
#include <string.h>
using namespace std;


cJSON* getAnDefaultObject()
{
	// Root JSON Object
	cJSON* obj = cJSON_CreateObject();

	// add Basic JSON type to Root JSON Object
	{
		cJSON_AddItemToObject(obj, "NullValue", cJSON_CreateNull());
		cJSON_AddItemToObject(obj, "TrueValue", cJSON_CreateTrue());
		cJSON_AddItemToObject(obj, "FalseValue", cJSON_CreateFalse());
		cJSON_AddItemToObject(obj, "BoolValue", cJSON_CreateBool(1));
		cJSON_AddItemToObject(obj, "NumberValue", cJSON_CreateNumber(12));
		cJSON_AddItemToObject(obj, "StringValue", cJSON_CreateString("Hello world"));
	}
	
	// add Arrary Type to Root JSON Object
	{
		cJSON* ary = cJSON_CreateArray();
		cJSON_AddItemToArray(ary, cJSON_CreateNull());
		cJSON_AddItemToArray(ary, cJSON_CreateTrue());
		cJSON_AddItemToArray(ary, cJSON_CreateNumber(12));
		cJSON_AddItemToObject(obj, "ArrayValue", ary);		
	}
	
	// add int Arrary Type to Root JSON Object
	{
		int namelist[3] = {0, 1, 2};
		cJSON* intAry = cJSON_CreateIntArray(namelist, 3);
		cJSON_AddItemToObject(obj, "IntArrayValue", intAry);		
	}
	
	// add JSON Object Type to Root JSON Object
	{
		cJSON* subobj = cJSON_CreateObject();
		cJSON_AddItemToObject(subobj, "NullValue", cJSON_CreateNull());
		cJSON_AddItemToObject(subobj, "TrueValue", cJSON_CreateTrue());
		cJSON_AddItemToObject(subobj, "FalseValue", cJSON_CreateFalse());
		cJSON_AddItemToObject(obj, "SubObjValue", subobj);		
	}
	
	// add Raw Type to Root JSON Object
	{
		cJSON* raw = cJSON_CreateRaw("0x234567abcdedf");
		cJSON_AddItemToObject(obj, "RawValue", raw);		
	}
	
	return obj;	
}

string getAnDefaultString()
{
	/* 双引号需要使用转义字符，花括号不需要使用转义字符 */
	return "{\"Number\": 12, \"Vaild\": false, \"SubObject\": {\"Name\": \"Tom\", \"Age\": 14}}";
}