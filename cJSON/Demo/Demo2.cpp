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
	
	cout << "cJSON_IsInvalid : " << cJSON_IsInvalid(obj) << endl;
	cout << "cJSON_IsObject  : " << cJSON_IsObject(obj) << endl;

	// JSON 对象转字符串，带JSON格式输出
	cout << cJSON_Print(obj) << endl;
	
	// JSON 对象转字符串，不带JSON格式输出，纯字符串
	cout << cJSON_PrintUnformatted(obj) << endl;
	
	// JSON 对象转字符串，带缓冲输出
	// CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt);
	// 参数item是JSON对象，
	// 参数prebuffer是预期内存大小，设置过大容易造成内存浪费，设置过小容易多次malloc重分配
	// 参数fmt=0表示无JSON格式输出，fmt=1表示带JSON格式输出
	cout << cJSON_PrintBuffered(obj, 1024, 0) << endl;
	
	// JSON 对象转字符串，字符串存储到指定缓冲区
	// CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);
	// 参数item是JSON对象，
	// 参数buffer缓冲区
	// 参数length是缓冲区buffer大小
	// 参数format=0表示不带JSON格式字符串输出，format=1代表带JSON格式字符串输出
	char* buffer = (char*)malloc(1024);
	memset(buffer, 0, 1024);
	cJSON_PrintPreallocated(obj, buffer, 1024, 0);
	cout << buffer << endl;
	free(buffer);
	
	cJSON_Delete(obj);
	cout << "=============================================" << endl;	
	
	return 0;
}