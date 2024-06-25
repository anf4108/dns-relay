#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "convert.h"
#include "struct.h"
#include "utils.h"

#define LOG_MASK 15
#define BUFFER_SIZE 1024
#define IPSIZE 4
#define PORT 53
const char *SERVER_IPADDR = "10.3.9.4";
const char *LOCAL_IPADDR = "127.0.0.1";
FILE *log_file;

int lookup_int_text(char *dn, char *ip) {
    int flag = 0;
    FILE *fp;
    char ipAddr[BUFFER_SIZE];
    char domainName[BUFFER_SIZE];
    if ((fp = fopen("../dnsrelay.txt", "r")) == NULL) {
        printf("file open error\n");
        exit(1);
    }
    while (!feof(fp)) {
        fgets(ipAddr, 1024, fp);
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (ipAddr[i] == ' ') {
                ipAddr[i] = '\0';
                break;
            }
        }
        strcpy(domainName, ipAddr + strlen(ipAddr) + 1);
        if (domainName[strlen(domainName) - 1] == '\n')
            domainName[strlen(domainName) - 1] = '\0';
        else
            domainName[strlen(domainName)] = '\0';

        // printf("111:%s!\n", domainName);
        // printf("222:%s!\n", dn);
        // printf("333:%d\n", strcmp(dn, domainName));
        //  找到了域名，得到ip地址

        if (strcmp(dn, domainName) == 0) {
            // 得到ip地址
            char *h = ipAddr;
            char *p = ipAddr;
            int i = 0;
            // printf("%s\n", ipAddr);
            while (*p != '\0') {
                if (*p == '.') {
                    *p = '\0';
                    ip[i] = (char)atoi(h);
                    i++;
                    h = p + 1;
                }
                p++;
            }
            ip[i] = atoi(h);
            flag = 1;
            return flag;
        }
    }
    return flag;
}

void dns_run() {
    char buf[BUFFER_SIZE];
    int socketFd, r;
    struct sockaddr_in srv;
    struct sockaddr_in clt;

    bzero(&srv, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    inet_aton(LOCAL_IPADDR, &srv.sin_addr);

    clt.sin_family = AF_INET;
    clt.sin_port = htons(PORT);
    inet_aton(LOCAL_IPADDR, &clt.sin_addr);

    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0) {
        perror("socket error\n");
        exit(1);
    }

    r = bind(socketFd, (struct sockaddr *)&srv, sizeof(srv));
    if (r < 0) {
        log_error("bind error");
        exit(1);
    }

    unsigned int len = sizeof(clt);
    while (1) {
        // 从客户端接收数据
        memset(buf, 0, BUFFER_SIZE);
        r = recvfrom(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&clt, &len);
        if (r < 0) {
            log_error("recvfrom error");
            exit(1);
        }

        // 解析DNS报文
        dns_message *message = (dns_message *)malloc(sizeof(dns_message));
        if (message == NULL) {
            log_fatal("内存分配错误");
        }
        byte_to_dns_message(message, buf);
        char *domain_name = (char *)malloc(BUFFER_SIZE);
        char *ip = (char *)malloc(IPSIZE);
        strcpy(domain_name, message->question->qname);
        log_info("收到查询请求：%s", domain_name);
        int find_dn_ip = lookup_int_text(domain_name, ip);
        free(domain_name);
        if (find_dn_ip) {
            log_info("查询到IP地址：%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            // 构造DNS响应报文
            dns_message response;
            response.header = (dns_header *)malloc(sizeof(dns_header));
            if (response.header == NULL) {
                log_fatal("内存分配错误");
            }
            *response.header = *message->header;
            response.header->qr = 1;              // 响应
            response.header->opcode = 0;          // 标准查询
            response.header->aa = 0;              // 非权威答案
            response.header->tc = 0;              // 非截断
            response.header->rd = 1;              // 期望递归
            response.header->ra = 1;              // 支持递归
            response.header->z = 0;               // 保留字段
            response.header->rcode = 0;           // 没有错误
            response.header->qdcount = htons(1);  // 问题数
            response.header->ancount = htons(1);  // 回答数
            response.header->nscount = htons(0);  // 授权资源记录数
            response.header->arcount = htons(0);  // 附加资源记录数
            if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
                response.header->rcode = 3;  // NXDOMAIN
            }

            // 设置问题部分
            response.question = message->question;

            // 设置回答部分
            dns_rr *answer = (dns_rr *)malloc(sizeof(dns_rr));
            if (answer == NULL) {
                log_fatal("内存分配错误");
            }
            answer->name = (uint8_t *)strdup((char *)message->question->qname);
            answer->type = htons(1);      // A记录
            answer->class = htons(1);     // IN类
            answer->ttl = htonl(3600);    // 3600秒
            answer->rdlength = htons(4);  // 4字节
            answer->rdata = (uint8_t *)malloc(4);
            if (answer->rdata == NULL) {
                log_fatal("内存分配错误");
            }
            memcpy(answer->rdata, ip, 4);

            response.rr = answer;

            // 将DNS消息转换为字节流
            uint32_t response_len = dns_message_to_byte((uint8_t *)buf, &response);

            // 发送响应报文给客户端
            sendto(socketFd, buf, response_len, 0, (struct sockaddr *)&clt, len);

            // 释放内存
            free(answer->rdata);
            free(answer);
            free(response.header);
        } else {
            struct sockaddr_in DnsSrvAddr;
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd < 0) {
                log_error("socket error");
                exit(1);
            }

            // 配置查询用DNS服务器地址
            bzero(&DnsSrvAddr, sizeof(DnsSrvAddr));
            DnsSrvAddr.sin_family = AF_INET;
            inet_aton(SERVER_IPADDR, &DnsSrvAddr.sin_addr);
            DnsSrvAddr.sin_port = htons(PORT);

            // 发送DNS查询请求
            unsigned int i = sizeof(DnsSrvAddr);
            len = sendto(fd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&DnsSrvAddr, sizeof(DnsSrvAddr));
            len = recvfrom(fd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&DnsSrvAddr, &i);
            if (len < 0) {
                log_error("recvfrom error");
                exit(1);
            }
            // TODO: 从DNS响应报文中解析出查询到的ip地址
        }

        r = sendto(socketFd, buf, sizeof(buf), 0, (struct sockaddr *)&clt, sizeof(clt));
        if (r < 0) {
            log_error("sendto error");
            exit(1);
        }
    }
}

int main() {
    log_file = stderr;
    dns_run();
    return 0;
}