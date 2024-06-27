#include "query.h"
#include "config.h"

int lookup_int_text(char *dn, char *ip) {
    int flag = 0;  // 标志，表示是否找到匹配的域名
    FILE *fp;
    char ipAddr[BUFFER_SIZE];
    char domainName[BUFFER_SIZE];

    // 先查找缓存
    if (BufferPool_find(bp, dn, 1, ip)) {
        return 1; // 缓存命中
    }

    // 如果缓存未命中，查找本地文件
    if ((fp = fopen("../dnsrelay.txt", "r")) == NULL) {
        printf("file open error\n");
        exit(1);
    }

    while (fgets(ipAddr, sizeof(ipAddr), fp) != NULL) {
        // 找到IP地址和域名之间的空格，并将其替换为字符串结束符
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (ipAddr[i] == ' ') {
                ipAddr[i] = '\0';
                break;
            }
        }
        strcpy(domainName, ipAddr + strlen(ipAddr) + 1);
        if (domainName[strlen(domainName) - 1] == '\n')
            domainName[strlen(domainName) - 1] = '\0';

        // 找到匹配的域名，提取IP地址
        if (strcmp(dn, domainName) == 0) {
            char *h = ipAddr;
            char *p = ipAddr;
            int i = 0;
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

            // 将结果添加到缓存
            BufferPool_add(bp, dn, 1, 3600, ip); // TTL为3600秒

            fclose(fp);
            return flag;
        }
    }
    fclose(fp);
    return flag;
}