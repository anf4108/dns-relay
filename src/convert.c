#include "convert.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define LOG_MASK 0x0F

// 从字节流中读取一个16位数
static uint16_t byte_to_uint16(const uint8_t * byte_stream, uint32_t * offset) {
    uint16_t res = ntohs(*(uint16_t *)(byte_stream + *offset));
    *offset += 2;
    return res;
}

// 从字节流中读取一个32位数
static uint32_t byte_to_uint32(const uint8_t * byte_stream, uint32_t * offset) {
    uint32_t res = ntohl(*(uint32_t *)(byte_stream + *offset));
    *offset += 4;
    return res;
}

// 从字节流中读取一个域名, 考虑压缩指针
static void byte_to_domain_name(char * domain_name, const uint8_t * byte_stream, uint32_t * offset) {
    int domain_name_len = 0;
    while (true) {
        int len = byte_stream[*offset];
        if (len == 0) {
            *offset += 1;
            break;
        }
        if (domain_name_len != 0) {
            domain_name[domain_name_len++] = '.';
        }
        if ((len & 0xc0) == 0xc0) {
            uint32_t pointer = byte_to_uint16(byte_stream, offset) & 0x3fff;
            uint32_t old_offset = *offset;
            byte_to_domain_name(domain_name + domain_name_len, byte_stream, &pointer);
            *offset = old_offset;
            return;
        }
        for (int i = 0; i < len; i++) {
            domain_name[domain_name_len++] = byte_stream[*offset + i + 1];
        }
        *offset += len + 1;
    }
    domain_name[domain_name_len] = '\0';
}

//将字节流转换成dnx_header结构体
static void byte_to_dns_header(dns_header * header, const uint8_t * byte_stream, uint32_t * offset) {
    header->id = byte_to_uint16(byte_stream, offset);
    uint16_t flag = byte_to_uint16(byte_stream, offset);
    header->qr = (flag >> 15) & 0x1;
    header->opcode = (flag >> 11) & 0xf;
    header->aa = (flag >> 10) & 0x1;
    header->tc = (flag >> 9) & 0x1;
    header->rd = (flag >> 8) & 0x1;
    header->ra = (flag >> 7) & 0x1;
    header->z = (flag >> 4) & 0x7;
    header->rcode = flag & 0xf;
    header->qdcount = byte_to_uint16(byte_stream, offset);
    header->ancount = byte_to_uint16(byte_stream, offset);
    header->nscount = byte_to_uint16(byte_stream, offset);
    header->arcount = byte_to_uint16(byte_stream, offset);
}

// 将字节流转换成dns_question结构体
static void byte_to_dns_question(dns_question * question, const uint8_t * byte_stream, uint32_t * offset) {
    question->qname = (uint8_t *)malloc(DNS_NAME_MAX_SIZE * sizeof(uint8_t));
    if (question->qname == NULL) {
        log_fatal("内存分配错误");
    }
    byte_to_domain_name(question->qname, byte_stream, offset);
    question->qtype = byte_to_uint16(byte_stream, offset);
    question->qclass = byte_to_uint16(byte_stream, offset);
}

// 将字节流转换成dns_rr结构体
static void byte_to_dns_rr(dns_rr * rr, const uint8_t * byte_stream, uint32_t * offset) {
    rr->name = (uint8_t *)malloc(DNS_NAME_MAX_SIZE * sizeof(uint8_t));
    if (rr->name == NULL) {
        log_fatal("内存分配错误");
    }
    byte_to_domain_name(rr->name, byte_stream, offset);
    rr->type = byte_to_uint16(byte_stream, offset);
    rr->class = byte_to_uint16(byte_stream, offset);
    rr->ttl = byte_to_uint32(byte_stream, offset);
    rr->rdlength = byte_to_uint16(byte_stream, offset);
    rr->rdata = (uint8_t *)malloc(rr->rdlength * sizeof(uint8_t));
    // TODO: 不同类型?
    if (rr->rdata == NULL) {
        log_fatal("内存分配错误");
    }
    memcpy(rr->rdata, byte_stream + *offset, rr->rdlength);
    *offset += rr->rdlength;
}

// 将字节流转换为 dns_message 结构体
void byte_to_dns_message(dns_message * message, const uint8_t * byte_stream) {
    uint32_t offset = 0;
    message->header = (dns_header *)malloc(sizeof(dns_header));
    if (message->header == NULL) {
        log_fatal("内存分配错误");
    }
    byte_to_dns_header(message->header, byte_stream, &offset);
    int q_num = message->header->qdcount;
    dns_question * q_tail = NULL;
    for (int i = 0; i < q_num; i++) {
        dns_question * tmp = (dns_question *)malloc(sizeof(dns_question));
        if (tmp == NULL) {
            log_fatal("内存分配错误");
        }
        if (!q_tail) {
            message->question = q_tail = tmp;
        } else {
            q_tail->next = tmp;
            q_tail = tmp;
        }
        byte_to_dns_question(tmp, byte_stream, &offset);
    }
    int rr_num = message->header->ancount + message->header->nscount + message->header->arcount;
    dns_rr *rr_tail = NULL;
    for (int i = 0; i < rr_num; i++) {
        dns_rr * tmp = (dns_rr *)malloc(sizeof(dns_rr));
        if (tmp == NULL) {
            log_fatal("内存分配错误");
        }
        if (!rr_tail) {
            message->rr = rr_tail = tmp;
        } else {
            rr_tail->next = tmp;
            rr_tail = tmp;
        }
        byte_to_dns_rr(tmp, byte_stream, &offset);
    }
}

// 将16位数转换为字节流
static void uint16_to_byte(uint8_t * byte_stream, uint16_t num, uint32_t * offset) {
    *(uint16_t *)(byte_stream + *offset) = htons(num);
    *offset += 2;
}

// 将32位数转换为字节流
static void uint32_to_byte(uint8_t * byte_stream, uint32_t num, uint32_t * offset) {
    *(uint32_t *)(byte_stream + *offset) = htonl(num);
    *offset += 4;
}

// 将域名转换为字节流
static void domain_name_to_byte(uint8_t * byte_stream, const char * domain_name, uint32_t * offset) {
    while (true) {
        char *location = strchr(domain_name, '.');
        if (location == NULL) break; 
        long len = location - domain_name;
        byte_stream[*offset] = len;
        memcpy(byte_stream + *offset + 1, domain_name, len);
        domain_name += len + 1;
        *offset += len + 1;
    }
    byte_stream[(*offset)++] = 0;
}

// 将dns_header结构体转换为字节流
static void dns_header_to_byte(uint8_t * byte_stream, const dns_header * header, uint32_t * offset) {
    uint16_to_byte(byte_stream, header->id, offset);
    uint16_t flag = 0;
    flag |= header->qr << 15;
    flag |= header->opcode << 11;
    flag |= header->aa << 10;
    flag |= header->tc << 9;
    flag |= header->rd << 8;
    flag |= header->ra << 7;
    flag |= header->z << 4;
    flag |= header->rcode;
    uint16_to_byte(byte_stream, flag, offset);
    uint16_to_byte(byte_stream, header->qdcount, offset);
    uint16_to_byte(byte_stream, header->ancount, offset);
    uint16_to_byte(byte_stream, header->nscount, offset);
    uint16_to_byte(byte_stream, header->arcount, offset);
}

// 将dns_question结构体转换为字节流
static void dns_question_to_byte(uint8_t * byte_stream, const dns_question * question, uint32_t * offset) {
    domain_name_to_byte(byte_stream, question->qname, offset);
    uint16_to_byte(byte_stream, question->qtype, offset);
    uint16_to_byte(byte_stream, question->qclass, offset);
}

// 将dns_rr结构体转换为字节流
static void dns_rr_to_byte(uint8_t * byte_stream, const dns_rr * rr, uint32_t * offset) {
    domain_name_to_byte(byte_stream, rr->name, offset);
    uint16_to_byte(byte_stream, rr->type, offset);
    uint16_to_byte(byte_stream, rr->class, offset);
    uint32_to_byte(byte_stream, rr->ttl, offset);
    uint16_to_byte(byte_stream, rr->rdlength, offset);
    memcpy(byte_stream + *offset, rr->rdata, rr->rdlength);
    *offset += rr->rdlength;
}

// 将dns_message结构体转换为字节流
unsigned dns_message_to_byte(uint8_t * byte_stream, const dns_message * message) {
    uint32_t offset = 0;
    dns_header_to_byte(byte_stream, message->header, &offset);
    dns_question * q = message->question;
    for (int i = 0; i < message->header->qdcount; i++) {
        dns_question_to_byte(byte_stream, q, &offset);
        q = q->next;
    }
    dns_rr * rr = message->rr;
    int rr_num = message->header->ancount + message->header->nscount + message->header->arcount;
    for (int i = 0; i < rr_num; i++) {
        dns_rr_to_byte(byte_stream, rr, &offset);
        rr = rr->next;
    }
    return offset;
}
