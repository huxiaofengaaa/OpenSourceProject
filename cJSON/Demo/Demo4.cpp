#include "cJSON.h"
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

extern cJSON* getAnDefaultObject();
extern string getAnDefaultString();

int main(int argc, char** argv)
{
	cJSON* obj = getAnDefaultObject();
	cout << cJSON_Print(obj) << endl;
	
	// 获取JSON对象或数组大小，一个子JSON对象也被认为是一个元素
	cout << "obj size : " << cJSON_GetArraySize(obj) << endl;
	
	// for循环遍历JSON对象每一个元素
	for(auto i = 0; i < cJSON_GetArraySize(obj) ; i++)
	{
		// 根据索引获取JSON对象或array元素
		// CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index);
		// 参数array：一个JSON 数组或对象
		// 参数index：元素索引，从0~size-1；
		cJSON* item = cJSON_GetArrayItem(obj, i); // item不应该使用cJSON_Delete释放，否则造成double free
		if(item)
		{
			if(true == cJSON_IsBool(item))// boolean类型
			{
				cout << item->string << " : " << item->valueint << endl;
			}
			else if(true == cJSON_IsNull(item))   //null类型
			{
				cout << item->string << " : " <<  item->valueint << endl;
			}
			else if(true == cJSON_IsNumber(item)) // number类型
			{
				cout << item->string << " : " <<  item->valueint << endl;
			}
			else if(true == cJSON_IsString(item)) // string类型
			{
				cout << item->string << " : " <<  item->valuestring << endl;
			}
			else if(true == cJSON_IsArray(item))  // array类型
			{
				cout << item->string << " : " <<  cJSON_Print(item) << endl;	
			}
			else if(true == cJSON_IsObject(item)) // JSON object类型
			{
				cout << item->string << " : " <<  cJSON_Print(item) << endl;	
			}
			else if(true == cJSON_IsRaw(item))    // raw类型
			{
				cout << item->string << " : " <<  item->valuestring << endl;
			}
		}
	}
	
	// 根据元素名称，获取值，如果不存在，则返回NULL；string大小写不敏感
	// CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string);
	cJSON* item2 = cJSON_GetObjectItem(obj, "subobjvalue");
	if(item2 && true == cJSON_IsObject(item2))
	{
		cout << "get subobjvalue; InCaseSensitive" << endl;
		cout << item2->string << " : " <<  cJSON_Print(item2) << endl;
	}
	
	// 根据元素名称，获取值，如果不存在，则返回NULL；string大小写敏感
	// CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string);
	cJSON* item3 = cJSON_GetObjectItemCaseSensitive(obj, "subobjvalue");
	if(item3 && true == cJSON_IsObject(item3))
	{
		cout << item3->string << " : " <<  cJSON_Print(item3) << endl;
	}
	else
	{
		cout << "not get subobjvalue; CaseSensitive" << endl;
	}
	
	// 判断object中是否存在string指定的元素，如果存在返回true，不存在返回false
	// CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string);
	if(true == cJSON_HasObjectItem(obj, "SubObjValue"))
	{
		cout << "contained SubObjValue " << endl;
	}
	else
	{
		cout << "not contained SubObjValue" << endl;
	}
	cJSON_Delete(obj);
	return 0;
}