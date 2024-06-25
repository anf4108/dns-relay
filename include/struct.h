#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

// #define DNS_STRING_MAX_SIZE 8192
// #define DNS_RR_NAME_MAX_SIZE 512
#define DNS_NAME_MAX_SIZE 512

// #define DNS_QR_QUERY 0
// #define DNS_QR_ANSWER 1

// #define DNS_OPCODE_QUERY 0
// #define DNS_OPCODE_IQUERY 1
// #define DNS_OPCODE_STATUS 2

// #define DNS_TYPE_A 1
// #define DNS_TYPE_NS 2
// #define DNS_TYPE_CNAME 5
// #define DNS_TYPE_SOA 6
// #define DNS_TYPE_PTR 12
// #define DNS_TYPE_HINFO 13
// #define DNS_TYPE_MINFO 15
// #define DNS_TYPE_MX 15
// #define DNS_TYPE_TXT 16
// #define DNS_TYPE_AAAA 28

// #define DNS_CLASS_IN 1

// #define DNS_RCODE_OK 0
// #define DNS_RCODE_NXDOMAIN 3

/// 报文Header Section结构体
typedef struct HEADER
{
    uint16_t id;
    uint8_t qr: 1;
    uint8_t opcode: 4;
    uint8_t aa: 1;
    uint8_t tc: 1;
    uint8_t rd: 1;
    uint8_t ra: 1;
    uint8_t z: 3;
    uint8_t rcode: 4;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header;

/// 报文Question Section结构体，以链表表示
typedef struct QUESTION
{
    uint8_t * qname;
    uint16_t qtype;
    uint16_t qclass;
    struct QUESTION * next;
} dns_question;

/// 报文Resource Record结构体，以链表表示
typedef struct RESOURCE_RECORD
{
    uint8_t * name;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t * rdata;
    struct RESOURCE_RECORD * next;
} dns_rr;

/// DNS报文结构体
typedef struct MESSAGE
{
    dns_header * header; 
    dns_question * question; 
    dns_rr * rr; 
} dns_message;

#endif // STRUCT_H
