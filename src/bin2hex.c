#include "bin2hex.h"
#include <stdio.h>

/************************************************************
 * input:
 *   dest: 为转换后的结果
 *   p->addr[0]: 高地址
 *   p->addr[1]: 低地址
 *   p->type: 记录类型
 *   p->len: 为bin格式流有效数据长度
 * output:
 *   返回有效数据的长度
 ************************************************************/
uint16_t BinFormatEncode(uint8_t *dest, HexFormat *p)
{
    uint16_t offset = 0;
    uint8_t check = 0, num = 0; //:(1) + 长度(2) + 地址(4) + 类型(2)
    sprintf((char *)&dest[offset], ":%02X%02X%02X%02X", p->len, p->addr[0], p->addr[1], p->type);
    offset += 9; // hex格式流数据指针偏移9
    check = p->len + p->addr[0] + p->addr[1] + p->type; // 计算校验和
    while (num < p->len)
    {
        sprintf((char *)&dest[offset], "%02X", p->data[num]);
        check += p->data[num]; // 计算校验和
        offset += 2; // hex格式数据流数据指针偏移2
        num++; // 下一个字符
    }
    check = ~check + 1; // 反码+1
    sprintf((char *)&dest[offset], "%02X", check);
    offset += 2;
    return offset; // 返回hex格式数据流的长度
}

/************************************************************
 * input:
 *   src: 源文件路径
 *   dest: 目标文件路径
 * output:
 *   返回转换结果
 ************************************************************/
RESULT_STATUS BinFile2HexFile(char *src, char *dest)
{
    FILE *src_file, *dest_file;
    uint16_t tmp;
    HexFormat hex_frmt;
    uint32_t low_addr = 0, high_addr = 0;
    uint8_t buffer_bin[NUMBER_OF_ONE_LINE], buffer_hex[MAX_BUFFER_OF_ONE_LINE];
    uint32_t src_file_length;
    uint16_t src_file_quotient, cur_file_page = 0;
    uint8_t src_file_remainder;

    src_file = fopen(src, "rb"); // 源文件为bin文件，以二进制的形式打开
    if (!src_file)
    {
        return RES_BIN_FILE_NOT_EXIST;
    }

    dest_file = fopen(dest, "w"); // 目的文件为hex文件，以文本的形式打开
    if (!dest_file)
    {
        return RES_HEX_FILE_PATH_ERROR;
    }

    fseek(src_file, 0, SEEK_END); // 定位到文末
    src_file_length = ftell(src_file);
    fseek(src_file, 0, SEEK_SET); // 重新定位到开头，准备开始读取数据
    src_file_quotient = (uint16_t)(src_file_length / NUMBER_OF_ONE_LINE); // 商，需要读取多少次
    src_file_remainder = (uint8_t)(src_file_length % NUMBER_OF_ONE_LINE); // 余数，最后一次需要多少个字符
    hex_frmt.data = buffer_bin; // 指向需要转化的bin数据流
    
    while (cur_file_page < src_file_quotient)
    {
        fread(buffer_bin, 1, NUMBER_OF_ONE_LINE, src_file);
        hex_frmt.len = NUMBER_OF_ONE_LINE;
        if ((low_addr & 0xffff0000) != high_addr && high_addr != 0) // 只有大于64K以后才写入扩展线性地址，第一次一般是没有
        {
            high_addr = low_addr & 0xffff0000;
            hex_frmt.addr[0] = (uint8_t)((high_addr & 0xff000000) >> 24);
            hex_frmt.addr[1] = (uint8_t)((high_addr & 0xff0000) >> 16);
            hex_frmt.type = 4;
            hex_frmt.len = 0;
            tmp = BinFormatEncode(buffer_hex, &hex_frmt);
            fwrite(buffer_hex, 1, tmp, dest_file);
            fprintf(dest_file, "\n");
        }
        hex_frmt.addr[0] = (uint8_t)((low_addr & 0xff00) >> 8);
        hex_frmt.addr[1] = (uint8_t)(low_addr & 0xff);
        hex_frmt.type = 0;
        tmp = BinFormatEncode(buffer_hex, &hex_frmt);
        fwrite(buffer_hex, 1, tmp, dest_file);
        fprintf(dest_file, "\n");
        cur_file_page++;
        low_addr += NUMBER_OF_ONE_LINE;
    }

    if (src_file_remainder != 0) // 最后一次读取的个数不为0，继续读取
    {
        fread(buffer_bin, 1, src_file_remainder, src_file);
        hex_frmt.addr[0] = (uint8_t)((low_addr & 0xff00) >> 8);
        hex_frmt.addr[1] = (uint8_t)(low_addr & 0xff);
        hex_frmt.len = src_file_remainder;
        hex_frmt.type = 0;
        tmp = BinFormatEncode(buffer_hex, &hex_frmt);
        fwrite(buffer_hex, 1, tmp, dest_file);
        fprintf(dest_file, "\n");
    }
    hex_frmt.addr[0] = 0;
    hex_frmt.addr[1] = 0;
    hex_frmt.type = 1; // 结束符
    hex_frmt.len = 0;
    tmp = BinFormatEncode(buffer_hex, &hex_frmt);
    fwrite(buffer_hex, 1, tmp, dest_file);
    fprintf(dest_file, "\n");
    fclose(src_file);
    fclose(dest_file);
    
    return RES_OK;
}