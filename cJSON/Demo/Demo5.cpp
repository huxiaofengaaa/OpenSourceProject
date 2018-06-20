#include "cJSON.h"
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

extern cJSON* getAnDefaultObject();
extern string getAnDefaultString();

int main(int argc, char** argv)
{
	{
		// 从 JSON对象parent中删除指定的元素
		cJSON* obj = getAnDefaultObject();
		cout << cJSON_Print(obj) << endl;
		cout << "obj size : " << cJSON_GetArraySize(obj) << endl;
		
		// cJSON_DetachItemViaPointer删除指定指向item，返回删除的对象，需要使用cJSON_Delete释放
		// CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item);
		cJSON* item = cJSON_GetArrayItem(obj, cJSON_GetArraySize(obj)-1);
		cJSON* del = cJSON_DetachItemViaPointer(obj, item);
		if(del)
		{
			cJSON_Delete(del);
		}
		
		// 根据元素名称，删除指定元素，并返回被删除的元素，大小写不敏感
		// CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string);
		cJSON* del2 = cJSON_DetachItemFromObject(obj, "nullvalue");
		if(del2)
		{
			cJSON_Delete(del2);
		}
		
		// 根据元素名称，删除指定元素，并返回被删除的元素，大小敏感
		// CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string);
		cJSON* del3 = cJSON_DetachItemFromObjectCaseSensitive(obj, "TrueValue");
		if(del3)
		{
			cJSON_Delete(del3);
		}
		
		// 根据元素名称，删除指定元素，不返回被删除的元素，大小写不敏感
		// CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string);
		cJSON_DeleteItemFromObject(obj, "falsevalue");
		
		// 根据元素名称，删除指定元素，不返回被删除的元素，大小写敏感
		// CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string);
		cJSON_DeleteItemFromObjectCaseSensitive(obj, "BoolValue");

		cout << "obj size : " << cJSON_GetArraySize(obj) << endl;
		cJSON_Delete(obj);
	}
	
	{
		// 从JSON数组array中删除指定的元素，
		int number[5] = {0, 1, 2 ,3, 4};
		cJSON* ary = cJSON_CreateIntArray(number, 5);
		
		// cJSON_DetachItemFromArray 返回删除的元素，需要使用cJSON_Delete释放
		// CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which);
		cJSON* item1 = cJSON_DetachItemFromArray(ary, cJSON_GetArraySize(ary)-1);
		if(item1)
		{
			cJSON_Delete(item1);
		}
		
		// cJSON_DeleteItemFromArray 不返回删除的元素
		// CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which);
		cJSON_DeleteItemFromArray(ary, cJSON_GetArraySize(ary)-1);
		
		cout << "ary size : " << cJSON_GetArraySize(ary) << endl;
		cJSON_Delete(ary);
	}
	

	return 0;
}