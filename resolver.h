#ifndef RESOLVER_H
#define RESOLVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include "protocol.h"
// #include "trietree.h"

struct header
{
    u_short ID;       //会话标识
    u_int QR : 1;     //查询/响应标志
    u_int opcode : 4; //查询响应类型
    u_int AA : 1;     //授权回答 authoritative answer
    u_int TC : 1;     //截断标志 truncated
    u_int RD : 1;     //期望递归标志 recursion desired
    u_int RA : 1;     //可用递归标志 recursion available
    u_int nil : 3;    //必须为0
    u_int rcode : 4;  //返回码
    u_short query_num;
    u_short answer_num;
    u_short authority_num;
    u_short addition_num;
};

struct message
{
    struct header header;
    struct query *query;
    struct RR *answer;
    struct RR *authority;
    struct RR *addition;
};

// u_char get_name_len(u_char *buf, u_char * bias);
void header_process(u_char *buffer, struct header *h);
u_short query_process(u_char *buf, u_short bias, struct query *q);
u_short RR_process(u_char *buf, u_short bias, struct RR *rr);
void recv_process(u_char *buf, struct message* m);
void send_process(u_char *buf, struct message* m);
// void resolve(char *in_buffer, char *out_buffer, struct node *internal_cache, struct node *external_cache);
#endif