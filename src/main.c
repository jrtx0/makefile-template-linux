#include <stdio.h>
#include "bin2hex.h"

int main(int argc, char *argv[])
{
    RESULT_STATUS res;

    if (argc != 3)
    {
        printf("input para doesn't match\r\n");
        return -1;
    }

    res = BinFile2HexFile(argv[1], argv[2]);

    switch (res)
    {
    case RES_OK:
        printf("hex file to bin file success!\r\n");
        return -1;
    case RES_BIN_FILE_NOT_EXIST:
        printf("bin file doesn't exist!\r\n");
        return -1;
    case RES_HEX_FILE_PATH_ERROR:
        printf("hex file path is error, please check it!\r\n");
        return -1;
    
    default:
        break;
    }
    return 0;
}