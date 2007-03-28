#ifndef ICP_HEADER

#define ICP_HEADER

/* Version */
#define ICP_VERSION_1           1   
#define ICP_VERSION_2           2
#define ICP_VERSION_3           3
#define ICP_VERSION_CURRENT     ICP_VERSION_2

#define ICP_FLAG_HIT_OBJ	0x80000000ul
#define ICP_FLAG_SRC_RTT	0x40000000ul

#if SIZEOF_INT == 4
typedef unsigned int u_num32;
#elif SIZEOF_LONG == 4
typedef unsigned long u_num32;
#else
#error "No suitable type for representing a 32-bits value"
#endif

struct icp_common_s {
    unsigned char opcode;       /* opcode */
    unsigned char version;      /* version number */
    unsigned short length;      /* total length (bytes) */
    u_num32 reqnum;             /* req number (req'd for UDP) */
    u_num32 flags;
    u_num32 pad;
    u_num32 shostid;            /* sender host id */
};
typedef struct icp_common_s icp_common_t;

typedef enum {
    ICP_OP_INVALID,		/* 00 to insure 0 doesn't get accidently interpreted. */
    ICP_OP_QUERY,		/* 01 query opcode (cl->sv) */
    ICP_OP_HIT,			/* 02 hit (cl<-sv) */
    ICP_OP_MISS,		/* 03 miss (cl<-sv) */
    ICP_OP_ERR,			/* 04 error (cl<-sv) */
    ICP_OP_SEND,		/* 05 send object non-auth (cl->sv) */
    ICP_OP_SENDA,		/* 06 send object authoritative (cl->sv) */
    ICP_OP_DATABEG,		/* 07 first data, but not last (sv<-cl) */
    ICP_OP_DATA,		/* 08 data middle of stream (sv<-cl) */
    ICP_OP_DATAEND,		/* 09 last data (sv<-cl) */
    ICP_OP_SECHO,		/* 10 echo from source (sv<-os) */
    ICP_OP_DECHO,		/* 11 echo from dumb cache (sv<-dc) */
    ICP_OP_UNUSED0,		/* 12 */
    ICP_OP_UNUSED1,		/* 13 */
    ICP_OP_UNUSED2,		/* 14 */
    ICP_OP_UNUSED3,		/* 15 */
    ICP_OP_UNUSED4,		/* 16 */
    ICP_OP_UNUSED5,		/* 17 */
    ICP_OP_UNUSED6,		/* 18 */
    ICP_OP_UNUSED7,		/* 19 */
    ICP_OP_UNUSED8,		/* 20 */
    ICP_OP_MISS_NOFETCH,	/* 21 access denied while reloading */
    ICP_OP_DENIED,		/* 22 access denied (cl<-sv) */
    ICP_OP_HIT_OBJ,		/* 23 hit with object data (cl<-sv) */
    ICP_OP_END			/* 24 marks end of opcodes */
} icp_opcode;


#endif  /* ICP_HEADER */
