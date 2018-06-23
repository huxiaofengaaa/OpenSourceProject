#include <iostream>
#include <string.h>
#include "base64.h"
using namespace std;


int main(int argc, char** argv)
{
	string input = "abcdefghijk";
	char* input_encode =  base64_encode_v2(input.c_str(), input.size());
	cout <<  input_encode << endl;
	free(input_encode);
	
	char* input_decode = base64_decode_v2(input.c_str(), input.size());
	cout << strlen(input_decode) << endl;
	for(int i = 0; i < strlen(input_decode) ; i++)
	{
		printf("0x%x ", input_decode[i]);
	}
	cout << endl;
	cout << input_decode << endl;
	free(input_decode);
	return 0;
}
