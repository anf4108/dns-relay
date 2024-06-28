#ifndef CONVERT_H
#define CONVERT_H

#include "struct.h"

// 将字节流转换为dns_message结构体
void byte_to_dns_message(dns_message * message, const uint8_t * byte_stream);

// 将dns_message结构体转换为字节流
unsigned dns_message_to_byte(uint8_t * byte_stream, const dns_message * message);


void rdata_to_ip(char * ip, const uint8_t * rdata);



#endif //CONVERT_H