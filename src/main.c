#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "convert.h"
#include "struct.h"
#include "utils.h"
#include "config.h"
#include "query.h"
#include "print.h"

// 初始化服务器套接字
int init_server_socket(struct sockaddr_in *srv) {
    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0) {
        log_error("socket error");
        exit(1);
    }

    bzero(srv, sizeof(*srv));
    srv->sin_family = AF_INET;
    srv->sin_port = htons(PORT);
    srv->sin_addr.s_addr = INADDR_ANY;

    int r = bind(socketFd, (struct sockaddr *)srv, sizeof(*srv));
    if (r < 0) {
        log_error("bind error");
        exit(1);
    }

    return socketFd;
}

void init_buffer_pool() {
    FILE *fp;
    char ipAddr[BUFFER_SIZE];
    char domainName[BUFFER_SIZE];

    if ((fp = fopen("../dnsrelay.txt", "r")) == NULL) {
        log_error("file open error\n");
        exit(1);
    }

    while (fgets(ipAddr, sizeof(ipAddr), fp) != NULL) {
        // 去掉行末的换行符
        ipAddr[strcspn(ipAddr, "\n")] = '\0';

        // 找到IP地址和域名之间的空格，并将其替换为字符串结束符
        char *space = strchr(ipAddr, ' ');
        if (space != NULL) {
            *space = '\0';
            strcpy(domainName, space + 1);

            // 将字符串形式的ip地址转换为二进制形式
            char ip[16];
            int ip_parts[4];
            sscanf(ipAddr, "%d.%d.%d.%d", &ip_parts[0], &ip_parts[1], &ip_parts[2], &ip_parts[3]);
            for (int i = 0; i < 4; i++) {
                ip[i] = (char)ip_parts[i];
            }
            log_debug("域名：%s，IP地址：%u.%u.%u.%u", domainName, (uint8_t)ip[0], (uint8_t)ip[1], (uint8_t)ip[2], (uint8_t)ip[3]);

            BufferPool_add(bp, domainName, 1, 360000, ip);
        }
    }

    fclose(fp);
}


// 接收客户端数据
int receive_client_data(int socketFd, char *buf, struct sockaddr_in *clt) {
    unsigned int len = sizeof(*clt);
    int r = recvfrom(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr *)clt, &len);
    if (r < 0) {
        log_error("recvfrom error");
        exit(1);
    }
    return r;
}

int dbg_flag;

void send_to_local(int socketFd, char *buf, struct sockaddr_in *clt, char *ip, dns_message *message, bool isIPv4) {
    log_info("本地查询到IP地址：%u.%u.%u.%u", (uint8_t)ip[0], (uint8_t)ip[1], (uint8_t)ip[2], (uint8_t)ip[3]);
    dns_message response;
    response.header = (dns_header *)malloc(sizeof(dns_header));
    if (response.header == NULL) {
        log_fatal("内存分配错误");
    }
    *response.header = *message->header;
    response.header->qr = 1;  // 响应
    response.header->opcode = 0;  // 标准查询
    response.header->aa = 0;  // 非权威答案
    response.header->tc = 0;  // 非截断
    response.header->rd = 1;  // 期望递归
    response.header->ra = 1;  // 支持递归
    response.header->z = 0;  // 保留字段
    response.header->rcode = DNS_RCODE_OK;  // 没有错误
    response.header->qdcount = 1;  // 问题数
    response.header->ancount = 1;  // 回答数
    response.header->nscount = 0;  // 授权资源记录数
    response.header->arcount = 0;  // 附加资源记录数

    response.question = message->question;

	dns_rr *answer = (dns_rr *)malloc(sizeof(dns_rr));
	if (answer == NULL) {
		log_fatal("内存分配错误");
	}

	if (isIPv4)
	{
		log_info("本地查询到IPv4地址：%u.%u.%u.%u", (uint8_t)ip[0], (uint8_t)ip[1], (uint8_t)ip[2], (uint8_t)ip[3]);
		if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
			response.header->rcode = 3;  // NXDOMAIN
		}

		answer->name = (uint8_t *)strdup((char *)message->question->qname);
		answer->type = DNS_TYPE_A;		// 类型
		answer->class = 1;				// IN类
		answer->ttl = 3600;				// 3600秒
		answer->rdlength = IP4SIZE;		// 4字节
		answer->rdata = (uint8_t *)malloc(IP4SIZE);
		if (answer->rdata == NULL) {
			log_fatal("内存分配错误");
		}
		memcpy(answer->rdata, ip, IP4SIZE);

		response.rr = answer;

	}
	else
	{
		log_info("本地查询到IPv6地址：%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
			ip[0], ip[1], ip[2], ip[3],
			ip[4], ip[5], ip[6], ip[7],
			ip[8], ip[9], ip[10], ip[11],
			ip[12], ip[13], ip[14], ip[15]);

		if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0
			&& ip[4] == 0 && ip[5] == 0 && ip[6] == 0 && ip[7] == 0
			&& ip[8] == 0 && ip[9] == 0 && ip[10] == 0 && ip[11] == 0
			&& ip[12] == 0 && ip[13] == 0 && ip[14] == 0 && ip[15] == 0)
		{
			response.header->rcode = 3;  // NXDOMAIN
		}

		answer->name = (uint8_t *)strdup((char *)message->question->qname);
		answer->type = DNS_TYPE_AAAA;	// 类型
		answer->class = 1;				// IN类
		answer->ttl = 3600;				// 3600秒
		answer->rdlength = IP6SIZE;		// 16字节
		answer->rdata = (uint8_t *)malloc(IP6SIZE);
		if (answer->rdata == NULL) {
			log_fatal("内存分配错误");
		}
		memcpy(answer->rdata, ip, IP6SIZE);

		response.rr = answer;

	}
    uint32_t response_len = dns_message_to_byte((uint8_t *)buf, &response);

    // sendto(socketFd, buf, response_len, 0, (struct sockaddr *)clt, sizeof(*clt));

    destroy_dns_rr(answer);
}

void send_to_remote(int socketFd, char *buf, struct sockaddr_in *clt, char *ip, dns_message *message, bool isIPv4) {
    struct sockaddr_in DnsSrvAddr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        log_error("socket error");
        exit(1);
    }

    // 设置超时时间为5秒
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        log_error("setsockopt error");
        close(fd);
        return;
    }

    // /* * * * * * * * * * * * * * * * * * * add pm * * * * * * * * * * * * * * * * * * */
    // uint16_t local_id = message->header->id;
    // uint16_t global_id;

    // char source_ip[4];
    // source_ip[0] = (clt->sin_addr.s_addr >> 24) & 0xFF;
    // source_ip[1] = (clt->sin_addr.s_addr >> 16) & 0xFF;
    // source_ip[2] = (clt->sin_addr.s_addr >> 8) & 0xFF; 
    // source_ip[3] =  clt->sin_addr.s_addr & 0xFF;        

    // PortMap_allocSeq(pm, source_ip, local_id, &global_id);

    // // 把buf（此时的buf是将发送给dns服务器的报文）的前16位改为global_id
    // buf[0] = (global_id >> 8) & 0xFF;
    // buf[1] = global_id & 0xFF;

    // /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    bzero(&DnsSrvAddr, sizeof(DnsSrvAddr));
    DnsSrvAddr.sin_family = AF_INET;
    inet_aton(SERVER_IPADDR, &DnsSrvAddr.sin_addr);
    DnsSrvAddr.sin_port = htons(PORT);

    sendto(fd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&DnsSrvAddr, sizeof(DnsSrvAddr));
    unsigned int len = sizeof(DnsSrvAddr);
    int r = recvfrom(fd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&DnsSrvAddr, &len);
    if (r < 0) {
        log_error("recvfrom error or timeout");
        close(fd);
        return;
    }
    dns_message *response = (dns_message *)malloc(sizeof(dns_message));
    if (response == NULL) {
        log_fatal("内存分配错误");
    }
    byte_to_dns_message(response, buf);

    // /* * * * * * * * * * * * * * * * * * * add pm * * * * * * * * * * * * * * * * * * */
    // global_id = response->header->id;
    
    // char dest_ip[4];
    // dest_ip[0] = (DnsSrvAddr.sin_addr.s_addr >> 24) & 0xFF;
    // dest_ip[1] = (DnsSrvAddr.sin_addr.s_addr >> 16) & 0xFF;
    // dest_ip[2] = (DnsSrvAddr.sin_addr.s_addr >> 8) & 0xFF;
    // dest_ip[3] = DnsSrvAddr.sin_addr.s_addr & 0xFF;

    // // 用global_id和dest_ip在pm中查到local_id
    // PortMap_querySeq(pm, global_id, &local_id, dest_ip);
    
    // // 把buf（此时的buf是将发送给客户端的报文）的前16位改为local_id
    // buf[0] = (local_id >> 8) & 0xFF;
    // buf[1] = local_id & 0xFF; 

    // // 因已回送报文，故在pm中删去该项
    // // PortMap_remove(pm, dest_ip, global_id);
    // /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    dns_rr *current_rr = response->rr;
    while (current_rr != NULL) {
        if (current_rr->type == DNS_TYPE_A && current_rr->rdlength == 4) {  // A记录
            rdata_to_ip(ip, current_rr->rdata);
            log_info("外部服务器查询到IP地址：%u.%u.%u.%u", (uint8_t)ip[0], (uint8_t)ip[1], (uint8_t)ip[2], (uint8_t)ip[3]);
            BufferPool_add(bp, message->question->qname, 1, 100, ip);
        } else if (current_rr->type == DNS_TYPE_AAAA && current_rr->rdlength == 16) { //AAAA记录
            rdata_to_ipv6(ip, current_rr->rdata);
            log_info("外部服务器查询到IPv6地址：%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                ip[0], ip[1], ip[2], ip[3],
                ip[4], ip[5], ip[6], ip[7],
                ip[8], ip[9], ip[10], ip[11],
                ip[12], ip[13], ip[14], ip[15]);
            BufferPool_add(bp, message->question->qname, 0, 100, ip);
        }
        current_rr = current_rr->next;
    }
    destroy_dns_message(response);
    close(fd);
}



// 处理DNS查询
void handle_dns_query(int socketFd, char *buf, struct sockaddr_in *clt) {
    dns_message *message = (dns_message *)malloc(sizeof(dns_message));
    if (message == NULL) {
        log_fatal("内存分配错误");
    }
    byte_to_dns_message(message, buf);

    dns_question *current_question = message->question;
    while (current_question != NULL) {
        char *domain_name = (char *)malloc(BUFFER_SIZE);

		// add ipv6
        char *ip = (char *)malloc(IP6SIZE);

        strcpy(domain_name, current_question->qname);
        log_info("收到查询请求：%s 并且QTYPE = %d", domain_name, message->question->qtype);

		// add ipv6
		bool isIPv4;
		int find_dn_ip;
		if (message->question->qtype == DNS_TYPE_A)
		{
			isIPv4 = true;
			find_dn_ip = lookup_in_cache(domain_name, ip);
		}
		else if (message->question->qtype == DNS_TYPE_AAAA)
		{
			isIPv4 = false;
			find_dn_ip = lookup_in_cache_ipv6(domain_name, ip);
		}

        free(domain_name);

        if (find_dn_ip) {
            send_to_local(socketFd, buf, clt, ip, message, isIPv4); //加参数
        } else {
            send_to_remote(socketFd, buf, clt, ip, message, isIPv4); //加参数
        }

        free(ip);
        current_question = current_question->next;
    }

    destroy_dns_message(message);
}

// 发送响应
void send_response(int socketFd, char *buf, struct sockaddr_in *clt) {
    int r = sendto(socketFd, buf, BUFFER_SIZE, 0, (struct sockaddr *)clt, sizeof(*clt));
    if (r < 0) {
        log_error("sendto error");
        exit(1);
    }
}

typedef struct {
    int socketFd;
    struct sockaddr_in clt;
    char buf[1024];
} client_request;

void *handle_client(void *arg) {
    client_request *request = (client_request *)arg;
    struct sockaddr_in clt = request->clt;
    char buf[BUFFER_SIZE];
    memcpy(buf, request->buf, BUFFER_SIZE);

    handle_dns_query(request->socketFd, buf, &clt);
    send_response(request->socketFd, buf, &clt);

    free(request);
    return NULL;
}

// 主函数
void dns_run() {
    char buf[BUFFER_SIZE];
    struct sockaddr_in srv, clt;

    int socketFd = init_server_socket(&srv);

    bp = BufferPool_create(1024, 100);
    init_buffer_pool();
    pm = PortMap_create(1024);

    while (1) {
        // receive_client_data(socketFd, buf, &clt);
        // handle_dns_query(socketFd, buf, &clt);
        // send_response(socketFd, buf, &clt);
        client_request *request = (client_request *)malloc(sizeof(client_request));
        if (request == NULL) {
            log_fatal("内存分配错误");
            continue;
        }
        request->socketFd = socketFd;
        receive_client_data(socketFd, request->buf, &request->clt);
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, request) != 0) {
            log_error("Failed to create thread");
            free(request);
        } else {
            pthread_detach(tid);
        }
    }
    PortMap_destroy(pm);

    close(socketFd);
}

int main() {
    printf("tested by hz\n");
    log_file = stderr;
    dns_run();
    return 0;
}