#include "struct.h"
#include <stdlib.h>


void destroy_dns_header(dns_header * header) {
    free(header);
}

void destroy_dns_question(dns_question * question) {
    dns_question *p = question;
    while (p) {
        dns_question *np = p;
        p = p->next;
        free(np->qname);
        free(np);
    }
}

void destroy_dns_rr(dns_rr * rr) {
    dns_rr *p = rr;
    while (p) {
        dns_rr *np = p;
        p = p->next;
        free(np->name);
        free(np->rdata);
        free(np);
    }
}

void destroy_dns_message(dns_message * message) {
    destroy_dns_header(message->header);
    destroy_dns_question(message->question);
    destroy_dns_rr(message->rr);
    free(message);
}