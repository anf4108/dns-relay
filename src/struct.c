#include "struct.h"
#include <stdlib.h>


void destroy_dns_header(dns_header * header) {
    if (header != NULL) {
        free(header);
    }
}

void destroy_dns_question(dns_question * question) {
    dns_question *p = question;
    while (p) {
        dns_question *np = p->next;
        if (p->qname != NULL) {
            free(p->qname);
        }
        free(p);
        p = np;
    }
}

void destroy_dns_rr(dns_rr * rr) {
    dns_rr *p = rr;
    while (p) {
        dns_rr *np = p->next;
        if (p->name != NULL) {
            free(p->name);
        }
        if (p->rdata != NULL) {
            free(p->rdata);
        }
        free(p);
        p = np;
    }
}

void destroy_dns_message(dns_message * message) {
    if (message != NULL) {
        destroy_dns_header(message->header);
        destroy_dns_question(message->question);
        destroy_dns_rr(message->rr);
        free(message);
    }
}
