/****************************************************************************************************
 *  base64编码、解码实现 C语言源代码
 *
 *  注意：请使用gcc编译
 *
 *  使用说明：
 *      命令行参数说明：若有“-d”参数，则为base64解码，否则为base64编码。
 *                      若有“-o”参数，后接文件名，则输出到标准输出文件。
 *      输入来自标准输入stdin，输出为标准输出stdout。可重定向输入输出流。
 *
 *      base64编码：输入任意二进制流，读取到文件读完了为止（键盘输入则遇到文件结尾符为止）。
 *                  输出纯文本的base64编码。
 *
 *      base64解码：输入纯文本的base64编码，读取到文件读完了为止（键盘输入则遇到文件结尾符为止）。
 *                  输出原来的二进制流。
 ****************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include "base64.h"

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef MAX_STR
#define MAX_STR 1024
#endif

void encode(FILE * fp_in, FILE * fp_out)
{
    unsigned char bindata[2050];
    char base64[4096];
    size_t bytes;
    while ( !feof( fp_in ) )
    {
        bytes = fread( bindata, 1, 2049, fp_in );
        base64_encode( bindata, base64, bytes );
        fprintf( fp_out, "%s", base64 );
    }
}

void decode(FILE * fp_in, FILE * fp_out)
{
    int i;
    unsigned char bindata[2050];
    char base64[4096];
    size_t bytes;
    while ( !feof( fp_in ) )
    {
        for ( i = 0 ; i < 2048 ; i ++ )
        {
            base64[i] = fgetc(fp_in);
            if ( base64[i] == EOF )
                break;
            else if ( base64[i] == '\n' || base64[i] == '\r' )
                i --;
        }
        bytes = base64_decode( base64, bindata );
        fwrite( bindata, bytes, 1, fp_out );
    }
}

void help(const char * filepath)
{
    fprintf( stderr, "Usage: %s [-d] [input_filename] [-o output_filepath]\n", filepath );
    fprintf( stderr, "\t-d\tdecode data\n" );
    fprintf( stderr, "\t-o\toutput filepath\n" );
    fprintf( stderr, "\t-i\tinput filepath\n");
    fprintf( stderr, "\t-s\tinput string\n\n");
}

int main(int argc, char * argv[])
{
    FILE * fp_input = NULL;
    FILE * fp_output = NULL;
    bool isencode = true;
    bool needHelp = false;
    int opt = 0;
    char input_filename[MAX_PATH] = "";
    char output_filename[MAX_PATH] = "";
    char input_string[MAX_STR] = "";
    int mode = 0;

    opterr = 0;
    while ( (opt = getopt(argc, argv, "hds:i:o:")) != -1 )
    {
        switch(opt)
        {
        case 'd':
            isencode = false;
            break;
	case 's':
            strncpy(input_string, optarg, sizeof(input_string));
            input_string[sizeof(input_string)-1] = '\0';
            break;
	case 'i':
	    strncpy(input_filename, optarg, sizeof(input_filename));
	    input_filename[sizeof(input_filename)-1] = '\0';
	    break;
        case 'o':
            strncpy(output_filename, optarg, sizeof(output_filename));
            output_filename[sizeof(output_filename)-1] = '\0';
            break;
        case 'h':
            needHelp = true;
            break;
        default:
            fprintf(stderr, "%s: invalid option -- %c\n", argv[0], optopt);
            needHelp = true;
            break;
        }
    }

    if(strcmp(input_filename, "") == 0 && strcmp(input_string, "") == 0)
    {
        needHelp = true;
    }

    if (needHelp)
    {
        help(argv[0]);
        return EXIT_FAILURE;
    }

    // set output
    if ( !strcmp(output_filename, "") )
    {
        fp_output = stdout;
    }
    else
    {
        if (isencode)
            fp_output = fopen(output_filename, "w");
        else
            fp_output = fopen(output_filename, "wb");
    }
    if ( fp_output == NULL )
    {
        fclose(fp_input);
        fp_input = NULL;
        fprintf(stderr, "Output file open error\n");
        return EXIT_FAILURE;
    }

    // encode or decode file
    if ( strcmp(input_filename, "") )
    {
        if (isencode)
            fp_input = fopen(input_filename, "rb");
        else
            fp_input = fopen(input_filename, "r");
        if ( fp_input == NULL )
        {
            fprintf(stderr, "Input file open error\n");
            return EXIT_FAILURE;
        }
        if (isencode)
            encode(fp_input, fp_output);
        else
            decode(fp_input, fp_output);
        fclose(fp_input);
        fp_input = fp_output = NULL;
    }
    else if(strcmp(input_string, ""))
    {
       char base64[4096] = { 0 };
       if(isencode)
           base64_encode(input_string, base64, strlen(input_string));
       else
           base64_decode(input_string, base64);
       fwrite( base64, sizeof(base64), 1, fp_output );
       fwrite( "\n\n", 2, 1, fp_output);
    }

    fclose(fp_output);
    return EXIT_SUCCESS;
}
