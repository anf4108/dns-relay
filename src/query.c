#include "query.h"
#include "config.h"

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