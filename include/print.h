#ifndef PRINT_H
#define PRINT_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "struct.h"
#include "util.h"

void print_dns_header(FILE *log_file, const dns_header *header) {
    fprintf(log_file, "Domain Name System (%s)\n", header->qr ? "response" : "query");
    fprintf(log_file, "  Transaction ID: 0x%x\n", header->id);
    fprintf(log_file, "  Flags: 0x%x\n", (header->qr << 15) | (header->opcode << 11) | (header->aa << 10) | (header->tc << 9) | (header->rd << 8) | (header->ra << 7) | (header->z << 4) | header->rcode);

    fprintf(log_file, "    %s, ", header->qr ? "Response" : "Query");
    switch (header->opcode) {
        case 0:
            fprintf(log_file, "Standard query");
            break;
        case 1:
            fprintf(log_file, "Inverse query");
            break;
        case 2:
            fprintf(log_file, "Server status request");
            break;
        default:
            fprintf(log_file, "Unknown operation");
            break;
    }
    fprintf(log_file, ", ");

    switch (header->rcode) {
        case 0:
            fprintf(log_file, "No error");
            break;
        case 1:
            fprintf(log_file, "Format error");
            break;
        case 2:
            fprintf(log_file, "Server failure");
            break;
        case 3:
            fprintf(log_file, "Name error");
            break;
        case 4:
            fprintf(log_file, "Not implemented");
            break;
        case 5:
            fprintf(log_file, "Refused");
            break;
        default:
            fprintf(log_file, "Unknown error");
            break;
    }
    fprintf(log_file, "\n");

    fprintf(log_file, "  Questions: %u\n", header->qdcount);
    fprintf(log_file, "  Answer RRs: %u\n", header->ancount);
    fprintf(log_file, "  Authority RRs: %u\n", header->nscount);
    fprintf(log_file, "  Additional RRs: %u\n", header->arcount);
}

void print_dns_question(FILE *log_file, const dns_question *question) {
    fprintf(log_file, "  Queries\n");
    while (question) {
        fprintf(log_file, "    %s: type A, class IN\n", question->qname);
        fprintf(log_file, "      Name: %s\n", question->qname);
        fprintf(log_file, "      [Name Length: %lu]\n", strlen((const char *)question->qname));
        fprintf(log_file, "      [Label Count: %d]\n", 3);  // 这里假设标签数为3，你可以根据具体情况计算
        fprintf(log_file, "      Type: A (1) (Host Address)\n");
        fprintf(log_file, "      Class: IN (0x0001)\n");
        question = question->next;
    }
}

void print_dns_rr(FILE *log_file, const dns_rr *rr) {
    fprintf(log_file, "  Answers\n");
    while (rr) {
        fprintf(log_file, "    %s: type %u, class IN, ", rr->name, rr->type);
        if (rr->type == 5) {  // CNAME
            fprintf(log_file, "cname %s\n", rr->rdata);
        } else if (rr->type == 1) {  // A
            fprintf(log_file, "addr %u.%u.%u.%u\n", rr->rdata[0], rr->rdata[1], rr->rdata[2], rr->rdata[3]);
        }
        fprintf(log_file, "      Name: %s\n", rr->name);
        fprintf(log_file, "      Type: %u\n", rr->type);
        fprintf(log_file, "      Class: IN (0x0001)\n");
        fprintf(log_file, "      TTL: %u\n", rr->ttl);
        fprintf(log_file, "      Data length: %u\n", rr->rdlength);
        rr = rr->next;
    }
}

void print_dns_message(FILE *log_file, const dns_message *message) {
    print_dns_header(log_file, message->header);
    print_dns_question(log_file, message->question);
    print_dns_rr(log_file, message->rr);
}

// void print_dns_string(const char * pstring, unsigned int len)
// {
//     if (!(LOG_MASK & 1))return;
//     log_debug("DNS报文字节流：")
//     for (unsigned int i = 0; i < len; i++)
//     {
//         if (i % 16 == 0)
//         {
//             if (i)fprintf(log_file, "\n");
//             fprintf(log_file, "%04x ", i);
//         }
//         fprintf(log_file, "%02hhx ", pstring[i]);
//     }
//     fprintf(log_file, "\n");
// }

// /**
//  * @brief 打印A类型RR的rdata字段
//  *
//  * @param rdata rdata字段
//  */
// static void print_rr_A(const uint8_t * rdata)
// {
//     fprintf(log_file, "%d.%d.%d.%d", rdata[0], rdata[1], rdata[2], rdata[3]);
// }

// /**
//  * @brief 打印AAAA类型RR的rdata字段
//  *
//  * @param rdata rdata字段
//  */
// static void print_rr_AAAA(const uint8_t * rdata)
// {
//     for (int i = 0; i < 16; i += 2)
//     {
//         if (i)fprintf(log_file, ":");
//         fprintf(log_file, "%x", (rdata[i] << 8) + rdata[i + 1]);
//     }
// }

// /**
//  * @brief 打印CNAME类型RR的rdata字段
//  *
//  * @param rdata rdata字段
//  */
// static void print_rr_CNAME(const uint8_t * rdata)
// {
//     fprintf(log_file, "%s", rdata);
// }

// /**
//  * @brief 打印SOA类型RR的rdata字段
//  *
//  * @param rdlength rdlength字段
//  * @param rdata rdata字段
//  */
// static void print_rr_SOA(uint16_t rdlength, const uint8_t * rdata)
// {
//     print_rr_CNAME(rdata);
//     fprintf(log_file, " ");
//     print_rr_CNAME(rdata + strlen(rdata) + 1);
//     fprintf(log_file, " ");
//     fprintf(log_file, "%" PRIu32 " ", ntohl(*(uint32_t *) (rdata + rdlength - 20)));
//     fprintf(log_file, "%" PRIu32 " ", ntohl(*(uint32_t *) (rdata + rdlength - 16)));
//     fprintf(log_file, "%" PRIu32 " ", ntohl(*(uint32_t *) (rdata + rdlength - 12)));
//     fprintf(log_file, "%" PRIu32 " ", ntohl(*(uint32_t *) (rdata + rdlength - 8)));
//     fprintf(log_file, "%" PRIu32, ntohl(*(uint32_t *) (rdata + rdlength - 4)));
// }

// /**
//  * @brief 打印MX类型RR的rdata字段
//  *
//  * @param rdlength rdlength字段
//  * @param rdata rdata字段
//  */
// static void print_rr_MX(const uint8_t * rdata)
// {
//     fprintf(log_file, "%" PRIu32 " ", ntohl(*(uint32_t *) rdata));
//     print_rr_CNAME(rdata + 2);
// }

// /**
//  * @brief 打印Header Section
//  *
//  * @param phead Header Section
//  */
// static void print_dns_header(const Dns_Header * phead)
// {
//     fprintf(log_file, "ID = 0x%04" PRIx16 "\n", phead->id);
//     fprintf(log_file, "QR = %" PRIu8 "\n", phead->qr);
//     fprintf(log_file, "OPCODE = %" PRIu8 "\n", phead->opcode);
//     fprintf(log_file, "AA = %" PRIu8 "\n", phead->aa);
//     fprintf(log_file, "TC = %" PRIu8 "\n", phead->tc);
//     fprintf(log_file, "RD = %" PRIu8 "\n", phead->rd);
//     fprintf(log_file, "RA = %" PRIu8 "\n", phead->ra);
//     fprintf(log_file, "RCODE = %" PRIu16 "\n", phead->rcode);
//     fprintf(log_file, "QDCOUNT = %" PRIu16 "\n", phead->qdcount);
//     fprintf(log_file, "ANCOUNT = %" PRIu16 "\n", phead->ancount);
//     fprintf(log_file, "NSCOUNT = %" PRIu16 "\n", phead->nscount);
//     fprintf(log_file, "ARCOUNT = %" PRIu16 "\n", phead->arcount);
// }

// /**
//  * @brief 打印Question Section
//  *
//  * @param pque Question Section
//  */
// static void print_dns_question(const Dns_Que * pque)
// {
//     fprintf(log_file, "QNAME = %s\n", pque->qname);
//     fprintf(log_file, "QTYPE = %" PRIu16 "\n", pque->qtype);
//     fprintf(log_file, "QCLASS = %" PRIu16 "\n", pque->qclass);
// }

// /**
//  * @brief 打印Resource Record
//  *
//  * @param prr Resource Record
//  */
// static void print_dns_rr(const Dns_RR * prr)
// {
//     fprintf(log_file, "NAME = %s\n", prr->name);
//     fprintf(log_file, "TYPE = %" PRIu16 "\n", prr->type);
//     fprintf(log_file, "CLASS = %" PRIu16 "\n", prr->class);
//     fprintf(log_file, "TTL = %" PRIu32 "\n", prr->ttl);
//     fprintf(log_file, "RDLENGTH = %" PRIu16 "\n", prr->rdlength);
//     fprintf(log_file, "RDATA = ");
//     if (prr->type == DNS_TYPE_A)
//         print_rr_A(prr->rdata);
//     else if (prr->type == DNS_TYPE_CNAME || prr->type == DNS_TYPE_NS)
//         print_rr_CNAME(prr->rdata);
//     else if (prr->type == DNS_TYPE_MX)
//         print_rr_MX(prr->rdata);
//     else if (prr->type == DNS_TYPE_AAAA)
//         print_rr_AAAA(prr->rdata);
//     else if (prr->type == DNS_TYPE_SOA)
//         print_rr_SOA(prr->rdlength, prr->rdata);
//     else
//         for (int i = 0; i < prr->rdlength; ++i)
//             fprintf(log_file, "%" PRIu8, *(prr->rdata + i));
//     fprintf(log_file, "\n");
// }

// void print_dns_message(const Dns_Msg * pmsg)
// {
//     if (!(LOG_MASK & 1))return;
//     log_debug("DNS报文内容：")
//     fprintf(log_file, "=======Header==========\n");
//     print_dns_header(pmsg->header);
//     fprintf(log_file, "\n");
//     fprintf(log_file, "=======Question========\n");
//     for (Dns_Que * pque = pmsg->que; pque; pque = pque->next)
//     {
//         print_dns_question(pque);
//         fprintf(log_file, "\n");
//     }
//     Dns_RR * prr = pmsg->rr;
//     fprintf(log_file, "=======Answer==========\n");
//     for (int i = 0; i < pmsg->header->ancount; ++i, prr = prr->next)
//     {
//         print_dns_rr(prr);
//         fprintf(log_file, "\n");
//     }
//     fprintf(log_file, "=======Authority=======\n");
//     for (int i = 0; i < pmsg->header->nscount; ++i, prr = prr->next)
//     {
//         print_dns_rr(prr);
//         fprintf(log_file, "\n");
//     }
//     fprintf(log_file, "=======Additional======\n");
//     for (int i = 0; i < pmsg->header->arcount; ++i, prr = prr->next)
//     {
//         print_dns_rr(prr);
//         fprintf(log_file, "\n");
//     }
// }

#endif