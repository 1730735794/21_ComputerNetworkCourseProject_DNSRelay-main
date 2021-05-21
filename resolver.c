#include "resolver.h"

// u_char get_name_len(u_char *buf, u_char bias)
// {
//     u_char query_len = 0;
//     u_char domain_seg_num = buf[bias];
//     while (domain_seg_num != 0)
//     {
//         query_len = query_len + domain_seg_num + 1;
//         domain_seg_num = buf[bias + query_len];
//     }
//     return ++query_len;
// }

u_char get_name(u_char * destination, u_char * buf, u_short bias)
{
    memset(destination, 0, 64);
    u_char len = 0;
    u_int flag = 0; //标志是否出现过指针，0为未出现过，1为出现过
    u_char have_ptr_bias = 0;
    while(buf[bias] != 0)
    {
        if(buf[bias] >= 0xc0)
        {
            if(!flag)
                have_ptr_bias = bias + 2; //计算与初始偏移量相比的偏移量
            flag = 1;
            bias = ((buf[bias] & 0x3f) << 8) + buf[bias + 1];
        }
        else
        {
            memcpy(destination + len, buf + bias, buf[bias] + 1);
            len += buf[bias] + 1;
            bias += buf[bias] + 1;
        }
    }
    destination[len] = 0;

    printf("query_name:");
    for (int i = 0; i <= len; i++)
    {
        if (destination[i] > 9)
            printf("%c", destination[i]);
        else
            printf("%d", destination[i]);
    }
    printf("\n");
    if(flag) 
    return have_ptr_bias; 
    else
    return bias + 1;
}
void header_process(u_char *buf, struct header *h)
{
    //header 从buf[0]至buf[11] 共12字节
    memcpy((u_char *)&h->ID, buf, 2);
    h->ID = ntohs(h->ID);
    h->QR = buf[2] >> 7;
    h->opcode = (buf[2] >> 3) & 0b01111;
    h->AA = (buf[2] >> 2) & 0b000001;
    h->TC = (buf[2] & 0b10) >> 1;
    h->RD = buf[2] & 0b1;
    h->RA = buf[3] >> 7;
    h->nil = (buf[3] >> 4) & 0b111;
    h->rcode = buf[3] & 0b1111;
    memcpy((u_char *)&h->query_num, buf + 4, 2);
    h->query_num = ntohs(h->query_num);
    memcpy((u_char *)&h->answer_num, buf + 6, 2);
    h->answer_num = ntohs(h->answer_num);
    memcpy((u_char *)&h->authority_num, buf + 8, 2);
    h->authority_num = ntohs(h->authority_num);
    memcpy((u_char *)&h->addition_num, buf + 10, 2);
    h->addition_num = ntohs(h->addition_num);
    // printf("ID: %x\n",h->ID);
    // printf("QR: %d\n",h->QR);
    // printf("opcode: %d\n",h->opcode);
    // printf("AA: %d\n",h->AA);
    // printf("TC: %d\n",h->TC);
    // printf("RD: %d\n",h->RD);
    // printf("RA: %d\n",h->RA);
    // printf("nil: %d\n",h->nil);
    // printf("rcode: %d\n",h->rcode);
    // printf("query_num: %x\n",h->query_num);
    // printf("answer_num: %d\n",h->answer_num);
    // printf("authority_num: %d\n",h->authority_num);
    // printf("addition_num: %d\n",h->addition_num);
}

u_short query_process(u_char *buf, u_short bias, struct query *q)
{
    bias = get_name((u_char *)&q->name, buf, bias);
    memcpy((u_char *)&q->type, buf +bias, 2);
    q->type = ntohs(q->type);
    memcpy((u_char *)&q->class, buf +bias + 2, 2);
    q->class = ntohs(q->class);
    bias += 4;

    printf("q_type:%x\n", q->type);
    printf("q_class:%x\n", q->class);
    return bias;
}

u_short RR_process(u_char *buf, u_short bias, struct RR *rr)
{
    bias = query_process(buf, bias, &rr->RR_query);
    memcpy((u_char *)&rr->TTL, buf + bias, 4);
    rr->TTL = ntohl(rr->TTL);
    memcpy((u_char *)&rr->RD_len, buf + bias + 4, 2);
    rr->RD_len = ntohs(rr->RD_len);
    memcpy((u_char *)&rr->RD_data, buf + bias + 6, rr->RD_len);
    bias += 4 + 2 + rr->RD_len;

    
    printf("RR_TTL:%d\n",rr->TTL);
    printf("RD_len:%d\n",rr->RD_len);
    printf("RR_data:");
    for(int i = 0; i < rr->RD_len; i++)
        printf("%x ",rr->RD_data[i]);
    printf("\n");
    return bias;
}

void recv_process(u_char *buf, struct message* m)
{
    u_short bias = 0;
    //header
    header_process(buf, &m->header);
    bias += 12;
    //query
    m->query = malloc(sizeof(struct query) * m->header.query_num);
    for (int i = 0; i < m->header.query_num; i++)
        bias = query_process(buf, bias, &m->query[i]);
    //RR
    m->answer = malloc(sizeof(struct query) * m->header.answer_num);
    for (int i = 0; i < m->header.answer_num; i++)
        bias = RR_process(buf, bias, &m->answer[i]);

    m->authority = malloc(sizeof(struct query) * m->header.authority_num);
    for (int i = 0; i < m->header.authority_num; i++)
        bias = RR_process(buf, bias, &m->authority[i]);

    m->addition = malloc(sizeof(struct query) * m->header.addition_num);
    for (int i = 0; i < m->header.addition_num; i++)
        bias = RR_process(buf, bias, &m->addition[i]);
}

void resolve(char *in_buffer, char *out_buffer, struct node *internal_cache, struct node *external_cache)
{
    struct message recv_message;
    recv_process(in_buffer, &recv_message);
    struct message send_message;
    struct RR answer[MAX_DB_RETURN];
    int answer_num = 0;
    if (answer_num = query_in_database(internal_cache, &recv_message.query, answer))
    {
        //内部cache命中,发送回应
        send_message.header = recv_message.header;
        send_message.header.QR = 1;
        send_message.header.RA = 1;
        send_message.header.answer_num = answer_num;
        send_message.query
        send_process(out_buffer, &send_message);
    }
    // else if (query_in_database(external_cache, &question, answer))
    // {
    //     //外部cache命中，发送回应
    // }
    // else
    // {
    //     //向外部dns服务器转发请求
    //     //接收外部dns服务器回应
    //     //将回应数据添加到外部cache
    //     //将外部dns回应转发给客户端
    // }
}
