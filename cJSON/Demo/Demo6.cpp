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
		
		// 使用replacement替换元素item，下面不可以对subobj和old调用cJSON_Delete
		// CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement);
		cJSON* subobj = cJSON_CreateObject();     
		cJSON* old = cJSON_GetArrayItem(obj, cJSON_GetArraySize(obj)-1);
		cJSON_ReplaceItemViaPointer(obj, old, subobj);
		
		// 使用newitem替换指定名称string的元素，大小写不敏感
		// CJSON_PUBLIC(void) cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);
		cJSON* subobj2 = cJSON_CreateObject();    // 不可以复用前面的subobj1
		cJSON_ReplaceItemInObject(obj, "nullvalue", subobj2);
		
		// 使用newitem替换指定名称string的元素，大小写敏感
		// CJSON_PUBLIC(void) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object,const char *string,cJSON *newitem);
		cJSON* subobj3 = cJSON_CreateObject();    // 不可以复用前面的subobj1
		cJSON_ReplaceItemInObjectCaseSensitive(obj, "TrueValue", subobj3);

		cout << cJSON_Print(obj) << endl;
		cJSON_Delete(obj);		
	}

	{
		int number[5] = {0, 1, 2 ,3, 4};
		cJSON* ary = cJSON_CreateIntArray(number, 5);
		cout << cJSON_Print(ary) << endl;
		
		// 在JSON数组指定位置插入元素，不可以对下面subobj调用cJSON_Delete
		// CJSON_PUBLIC(void) cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem);
		cJSON* subobj = cJSON_CreateObject();     
		cJSON_InsertItemInArray(ary, 0, subobj);
		
		// 使用newitem替换JSON数组指定位置元素，不可以对下面subobj2调用cJSON_Delete
		// CJSON_PUBLIC(void) cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem);
		cJSON* subobj2 = cJSON_CreateObject();     
		cJSON_ReplaceItemInArray(ary, 3, subobj2);
		
		cout << cJSON_Print(ary) << endl;
		cJSON_Delete(ary);
	}
	
	return 0;
}