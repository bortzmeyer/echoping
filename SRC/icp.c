/*
 * Code contributed by Christian Grimm <grimm@rvs.uni-hannover.de> 
 * and patched by Stephane Bortzmeyer. 
 *
 * $Id$
 *
 */

#include "echoping.h"

#ifdef ICP


void           *
make_icp_sendline(url, shost, opcode, length)
    const char     *url;
    u_num32        *shost;
    icp_opcode      opcode;
    int            *length;

{
    icp_common_t   *headerp = NULL;
    char           *buf = NULL;
    char           *urloffset = NULL;
    u_num32         flags = ICP_FLAG_SRC_RTT;
    u_num32         pad = 0;
    u_num32         reqnum = 4711;
    unsigned short  buf_len;

    buf_len = sizeof(icp_common_t) + strlen(url) + 1;
    if (opcode == ICP_OP_QUERY)
        buf_len += sizeof(u_num32);
    buf = calloc(buf_len, 1);
    headerp = (icp_common_t *) buf;
    headerp->opcode = opcode;
    headerp->version = ICP_VERSION_CURRENT;
    headerp->length = htons(buf_len);
    headerp->reqnum = htonl(reqnum);
    headerp->flags = htonl(flags);
    headerp->pad = pad;
    if (shost)
        headerp->shostid = htonl(*shost);
    /* urloffset = (char *) ((int) buf + sizeof(icp_common_t)); */
    urloffset = (char *) (buf + sizeof(icp_common_t));
    if (opcode == ICP_OP_QUERY)
        urloffset += sizeof(u_num32);
    memcpy(urloffset, url, strlen(url));
    *length = buf_len;
    return buf;
}

int
recv_icp(sockfd, buf, retcode)
    int             sockfd;
    char           *buf;
    char           *retcode;

{                               /* 
                                 * based on draft-wessels-icp-v2-02.txt 
                                 */
    icp_common_t   *headerp = (icp_common_t *) buf;
    int             nr, length;
    unsigned char   opcode;
    static char    *icp_op_code[] = {
        /* 0 */ "ICP_OP_INVALID",
        /* 1 */ "ICP_OP_QUERY",
        /* 2 */ "ICP_OP_HIT",
        /* 3 */ "ICP_OP_MISS",
        /* 4 */ "ICP_OP_ERR",
        /* 5 */ "",
        /* 6 */ "",
        /* 7 */ "",
        /* 8 */ "",
        /* 9 */ "",
        /* 10 */ "ICP_OP_SECHO",
        /* 11 */ "ICP_OP_DECHO",
        /* 12 */ "",
        /* 13 */ "",
        /* 14 */ "",
        /* 15 */ "",
        /* 16 */ "",
        /* 17 */ "",
        /* 18 */ "",
        /* 19 */ "",
        /* 20 */ "",
        /* 21 */ "ICP_OP_MISS_NOFETCH",
        /* 22 */ "ICP_OP_DENIED",
        /* 23 */ "ICP_OP_HIT_OBJ"
    };


    nr = recvfrom(sockfd, buf, DEFLINE, 0, (struct sockaddr *) 0, (int *) 0);
    if (nr < 0) {
        if (timeout_flag)
            err_quit("Timeout while reading");
        else
            err_sys("No reply from ICP proxy server");
    }
    opcode = headerp->opcode;
    length = ntohs(headerp->length);
    sprintf(retcode, "ICP reply: \42%s\42", icp_op_code[opcode]);
    return length;
}


#endif                          /* ICP */
