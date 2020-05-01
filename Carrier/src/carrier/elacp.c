/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include <crystal.h>

#include "elacp.h"
#include "elacp_generated.h"
#include "flatcc/support/hexdump.h"
#include "ela_carrier.h"

#pragma pack(push, 1)

struct ElaCP {
    uint8_t type;
    const char *ext;
};

struct ElaCPUserInfo {
    ElaCP header;
    bool has_avatar;
    const char *name;
    const char *descr;
    const char *phone;
    const char *gender;
    const char *email;
    const char *region;
};

struct ElaCPFriendReq {
    ElaCP header;
    const char *name;
    const char *descr;
    const char *hello;
};

struct ElaCPFriendMsg {
    ElaCP headr;
    size_t len;
    const uint8_t *msg;
};

struct ElaCPInviteReq {
    ElaCP header;
    int64_t tid;
    const char *bundle;
    size_t totalsz;
    size_t len;
    const uint8_t *data;
};

struct ElaCPInviteRsp {
    ElaCP header;
    int64_t tid;
    const char *bundle;
    size_t totalsz;
    int status;
    const char *reason;
    size_t len;
    const uint8_t *data;
};

struct ElaCPBulkMsg {
    ElaCP headr;
    int64_t tid;
    size_t totalsz;
    size_t len;
    const uint8_t *data;
};

#pragma pack(pop)

#define pktinfo pkt.u.pkt_info
#define pktfreq pkt.u.pkt_freq
#define pktfmsg pkt.u.pkt_fmsg
#define pktireq pkt.u.pkt_ireq
#define pktirsp pkt.u.pkt_irsp
#define pktbmsg pkt.u.pkt_bmsg

#define tblinfo tbl.u.tbl_info
#define tblfreq tbl.u.tbl_freq
#define tblfmsg tbl.u.tbl_fmsg
#define tblireq tbl.u.tbl_ireq
#define tblirsp tbl.u.tbl_irsp
#define tblbmsg tbl.u.tbl_bmsg

struct elacp_packet_t {
    union {
        struct ElaCP          *cp;
        struct ElaCPUserInfo  *pkt_info;
        struct ElaCPFriendReq *pkt_freq;
        struct ElaCPFriendMsg *pkt_fmsg;
        struct ElaCPInviteReq *pkt_ireq;
        struct ElaCPInviteRsp *pkt_irsp;
        struct ElaCPBulkMsg   *pkt_bmsg;
    } u;
};

struct elacp_table_t {
    union {
        elacp_userinfo_table_t  tbl_info;
        elacp_friendreq_table_t tbl_freq;
        elacp_friendmsg_table_t tbl_fmsg;
        elacp_invitereq_table_t tbl_ireq;
        elacp_invitersp_table_t tbl_irsp;
        elacp_bulkmsg_table_t   tbl_bmsg;
    } u;
};

ElaCP *elacp_create(uint8_t type, const char *ext_name)
{
    ElaCP *cp;
    size_t len;

    switch(type) {
    case ELACP_TYPE_USERINFO:
        len = sizeof(struct ElaCPUserInfo);
        break;
    case ELACP_TYPE_FRIEND_REQUEST:
        len = sizeof(struct ElaCPFriendReq);
        break;
    case ELACP_TYPE_MESSAGE:
        len = sizeof(struct ElaCPFriendMsg);
        break;
    case ELACP_TYPE_INVITE_REQUEST:
        len = sizeof(struct ElaCPInviteReq);
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        len = sizeof(struct ElaCPInviteRsp);
        break;
    case ELACP_TYPE_BULKMSG:
        len = sizeof(struct ElaCPBulkMsg);
        break;
    default:
        assert(0);
        return NULL;
    }

    cp = (ElaCP *)calloc(1, len);
    if (!cp)
        return NULL;

    cp->type = type;
    cp->ext  = ext_name;

    return cp;
}

void elacp_free(ElaCP *cp)
{
    if (cp)
        free(cp);
}

int elacp_get_type(ElaCP *cp)
{
    assert(cp);

    return cp->type;
}

const char *elacp_get_extension(ElaCP *cp)
{
    assert(cp);

    return cp->ext;
}

const char *elacp_get_name(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *name = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        name = pktinfo->name;
        break;
    case ELACP_TYPE_FRIEND_REQUEST:
        name = pktfreq->name;
        break;
    default:
        assert(0);
        break;
    }

    return name;
}

const char *elacp_get_descr(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *descr = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        descr = pktinfo->descr;
        break;
    case ELACP_TYPE_FRIEND_REQUEST:
        descr = pktfreq->descr;
        break;
    default:
        assert(0);
        break;
    }

    return descr;
}

const char *elacp_get_gender(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *gender = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        gender = pktinfo->gender;
        break;
    default:
        assert(0);
        break;
    }

    return gender;
}

const char *elacp_get_phone(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *phone = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        phone = pktinfo->phone;
        break;
    default:
        assert(0);
        break;
    }

    return phone;
}

const char *elacp_get_email(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *email = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        email = pktinfo->email;
        break;
    default:
        assert(0);
        break;
    }

    return email;
}

const char *elacp_get_region(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *region = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        region = pktinfo->region;
        break;
    default:
        assert(0);
        break;
    }

    return region;
}

bool elacp_get_has_avatar(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    bool has_avatar = 0;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        has_avatar = pktinfo->has_avatar;
        break;
    default:
        assert(0);
        break;
    }

    return has_avatar;
}

const char *elacp_get_hello(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *hello = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_FRIEND_REQUEST:
        hello = pktfreq->hello;
        break;
    default:
        assert(0);
        break;
    }

    return hello;
}

int64_t elacp_get_tid(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    int64_t tid = 0;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_REQUEST:
        tid = pktireq->tid;
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        tid = pktirsp->tid;
        break;
    case ELACP_TYPE_BULKMSG:
        tid = pktbmsg->tid;
        break;
    default:
        assert(0);
        break;
    }

    return tid;
}

size_t elacp_get_totalsz(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    size_t totalsz = 0;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_REQUEST:
        totalsz = pktireq->totalsz;
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        totalsz = pktirsp->totalsz;
        break;
    case ELACP_TYPE_BULKMSG:
        totalsz = pktbmsg->totalsz;
        break;
    default:
        assert(0);
        break;
    }

    return totalsz;
}

int elacp_get_status(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    int status = 0;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_RESPONSE:
        status = pktirsp->status;
        break;
    default:
        assert(0);
        break;
    }

    return status;
}

const void *elacp_get_raw_data(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const void *data = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_MESSAGE:
        data = pktfmsg->msg;
        break;
    case ELACP_TYPE_INVITE_REQUEST:
        data = pktireq->data;
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        data = pktirsp->data;
        break;
    case ELACP_TYPE_BULKMSG:
        data = pktbmsg->data;
        break;
    default:
        assert(0);
        break;
    }

    return data;
}

size_t elacp_get_raw_data_length(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    size_t len = 0;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_MESSAGE:
        len = pktfmsg->len;
        break;
    case ELACP_TYPE_INVITE_REQUEST:
        len = pktireq->len;
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        len = pktirsp->len;
        break;
    case ELACP_TYPE_BULKMSG:
        len = pktbmsg->len;
        break;
    default:
        assert(0);
        break;
    }

    return len;
}

const char *elacp_get_bundle(ElaCP *cp)
{
     struct elacp_packet_t pkt;
     const char *bundle = NULL;

     assert(cp);
     pkt.u.cp = cp;

     switch(cp->type) {
     case ELACP_TYPE_INVITE_REQUEST:
         bundle = pktireq->bundle;
         break;
     case ELACP_TYPE_INVITE_RESPONSE:
         bundle = pktirsp->bundle;
         break;
     default:
         assert(0);
         break;
     }

     return bundle;
 }

const char *elacp_get_reason(ElaCP *cp)
{
    struct elacp_packet_t pkt;
    const char *reason = NULL;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_RESPONSE:
        reason = pktirsp->reason;
        break;
    default:
        assert(0);
        break;
    }

    return reason;
}

void elacp_set_name(ElaCP *cp, const char *name)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(name);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        pktinfo->name = name;
        break;
    case ELACP_TYPE_FRIEND_REQUEST:
        pktfreq->name = name;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_descr(ElaCP *cp, const char *descr)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(descr);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        pktinfo->descr = descr;
        break;
    case ELACP_TYPE_FRIEND_REQUEST:
        pktfreq->descr = descr;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_gender(ElaCP *cp, const char *gender)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(gender);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        pktinfo->gender = gender;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_phone(ElaCP *cp, const char *phone)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(phone);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        pktinfo->phone = phone;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_email(ElaCP *cp, const char *email)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(email);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        pktinfo->email = email;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_region(ElaCP *cp, const char *region)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(region);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        pktinfo->region = region;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_has_avatar(ElaCP *cp, int has_avatar)
{
    struct elacp_packet_t pkt;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        pktinfo->has_avatar = !!has_avatar;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_hello(ElaCP *cp, const char *hello)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(hello);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_FRIEND_REQUEST:
        pktfreq->hello = hello;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_tid(ElaCP *cp, int64_t *tid)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(tid);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_REQUEST:
        pktireq->tid = *tid;
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        pktirsp->tid = *tid;
        break;
    case ELACP_TYPE_BULKMSG:
        pktbmsg->tid = *tid;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_totalsz(ElaCP *cp, size_t totalsz)
{
    struct elacp_packet_t pkt;

    assert(cp);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_REQUEST:
        pktireq->totalsz = totalsz;
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        pktirsp->totalsz = totalsz;
        break;
    case ELACP_TYPE_BULKMSG:
        pktbmsg->totalsz = totalsz;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_status(ElaCP *cp, int status)
{
    struct elacp_packet_t pkt;

    assert(cp);
    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_RESPONSE:
        pktirsp->status = status;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_raw_data(ElaCP *cp, const void *data, size_t len)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(data);
    assert(len > 0);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_MESSAGE:
        pktfmsg->msg = data;
        pktfmsg->len = len;
        break;
    case ELACP_TYPE_INVITE_REQUEST:
        pktireq->data = data;
        pktireq->len = len;
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        pktirsp->data = data;
        pktirsp->len = len;
        break;
    case ELACP_TYPE_BULKMSG:
        pktbmsg->data = data;
        pktbmsg->len = len;
        break;
    default:
        assert(0);
        break;
    }
}

void elacp_set_bundle(ElaCP *cp, const char *bundle)
 {
     struct elacp_packet_t pkt;
     assert(cp);

     pkt.u.cp = cp;

     switch(cp->type) {
     case ELACP_TYPE_INVITE_REQUEST:
         pktireq->bundle = bundle;
         break;
     case ELACP_TYPE_INVITE_RESPONSE:
         pktirsp->bundle = bundle;
         break;
     default:
         assert(0);
         break;
     }
}

void elacp_set_reason(ElaCP *cp, const char *reason)
{
    struct elacp_packet_t pkt;

    assert(cp);
    assert(reason);

    pkt.u.cp = cp;

    switch(cp->type) {
    case ELACP_TYPE_INVITE_RESPONSE:
        pktirsp->reason = reason;
        break;
    default:
        assert(0);
        break;
    }
}

uint8_t *elacp_encode(ElaCP *cp, size_t *encoded_len)
{
    struct elacp_packet_t pkt;
    flatcc_builder_t builder;
    flatcc_builder_ref_t str;
    flatbuffers_uint8_vec_ref_t vec;
    flatbuffers_ref_t ref;
    elacp_anybody_union_ref_t body;
    uint8_t *encoded_data;

    assert(cp);
    assert(encoded_len);

    pkt.u.cp = cp;

    flatcc_builder_init(&builder);

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        elacp_userinfo_start(&builder);
        if (pktinfo->name) {
            str = flatcc_builder_create_string_str(&builder, pktinfo->name);
            elacp_userinfo_name_add(&builder, str);
        }
        str = flatcc_builder_create_string_str(&builder, pktinfo->descr);
        elacp_userinfo_descr_add(&builder, str);
        str = flatcc_builder_create_string_str(&builder, pktinfo->gender);
        elacp_userinfo_gender_add(&builder, str);
        str = flatcc_builder_create_string_str(&builder, pktinfo->phone);
        elacp_userinfo_phone_add(&builder, str);
        str = flatcc_builder_create_string_str(&builder, pktinfo->email);
        elacp_userinfo_email_add(&builder, str);
        str = flatcc_builder_create_string_str(&builder, pktinfo->region);
        elacp_userinfo_region_add(&builder, str);
        elacp_userinfo_avatar_add(&builder, pktinfo->has_avatar);
        ref = elacp_userinfo_end(&builder);
        break;

    case ELACP_TYPE_FRIEND_REQUEST:
        elacp_friendreq_start(&builder);
        str = flatcc_builder_create_string_str(&builder, pktfreq->name);
        elacp_friendreq_name_add(&builder, str);
        str = flatcc_builder_create_string_str(&builder, pktfreq->descr);
        elacp_friendreq_descr_add(&builder, str);
        str = flatcc_builder_create_string_str(&builder, pktfreq->hello);
        elacp_friendreq_hello_add(&builder, str);
        ref = elacp_friendreq_end(&builder);
        break;

    case ELACP_TYPE_MESSAGE:
        elacp_friendmsg_start(&builder);
        if (cp->ext) {
            str = flatcc_builder_create_string_str(&builder, cp->ext);
            elacp_friendmsg_ext_add(&builder, str);
        }

        vec = flatbuffers_uint8_vec_create(&builder, pktfmsg->msg, pktfmsg->len);
        elacp_friendmsg_msg_add(&builder, vec);
        ref = elacp_friendmsg_end(&builder);
        break;

    case ELACP_TYPE_INVITE_REQUEST:
        elacp_invitereq_start(&builder);
        if (cp->ext) {
            str = flatcc_builder_create_string_str(&builder, cp->ext);
            elacp_invitereq_ext_add(&builder, str);
        }
        elacp_invitereq_tid_add(&builder, pktireq->tid);
        elacp_invitereq_totalsz_add(&builder, pktireq->totalsz);
        if (pktireq->bundle) {
             str = flatcc_builder_create_string_str(&builder, pktireq->bundle);
             elacp_invitereq_bundle_add(&builder, str);
        }
        vec = flatbuffers_uint8_vec_create(&builder, pktireq->data, pktireq->len);
        elacp_invitereq_data_add(&builder, vec);
        ref = elacp_invitereq_end(&builder);
        break;

    case ELACP_TYPE_INVITE_RESPONSE:
        elacp_invitersp_start(&builder);
        if (cp->ext) {
            str = flatcc_builder_create_string_str(&builder, cp->ext);
            elacp_invitersp_ext_add(&builder, str);
        }
        elacp_invitersp_tid_add(&builder, pktirsp->tid);
        elacp_invitersp_totalsz_add(&builder, pktirsp->totalsz);
        if (pktirsp->bundle) {
             str = flatcc_builder_create_string_str(&builder, pktirsp->bundle);
             elacp_invitersp_bundle_add(&builder, str);
        }
        elacp_invitersp_status_add(&builder, pktirsp->status);
        if (pktirsp->status && pktirsp->reason) {
            str = flatcc_builder_create_string_str(&builder, pktirsp->reason);
            elacp_invitersp_reason_add(&builder, str);
        } else {
            vec = flatbuffers_uint8_vec_create(&builder, pktirsp->data, pktirsp->len);
            elacp_invitersp_data_add(&builder, vec);
        }
        ref = elacp_invitersp_end(&builder);
        break;

    case ELACP_TYPE_BULKMSG:
        elacp_bulkmsg_start(&builder);
        if (cp->ext) {
            str = flatcc_builder_create_string_str(&builder, cp->ext);
            elacp_bulkmsg_ext_add(&builder, str);
        }
        elacp_bulkmsg_tid_add(&builder, pktbmsg->tid);
        elacp_bulkmsg_totalsz_add(&builder, pktbmsg->totalsz);
        vec = flatbuffers_uint8_vec_create(&builder, pktbmsg->data, pktbmsg->len);
        elacp_bulkmsg_data_add(&builder, vec);
        ref = elacp_bulkmsg_end(&builder);
        break;

    default:
        assert(0);
        ref = 0; // to clean builder.
        break;
    }

    if (!ref) {
        flatcc_builder_clear(&builder);
        return NULL;
    }

    switch(cp->type) {
    case ELACP_TYPE_USERINFO:
        body = elacp_anybody_as_userinfo(ref);
        break;
    case ELACP_TYPE_FRIEND_REQUEST:
        body = elacp_anybody_as_friendreq(ref);
        break;
    case ELACP_TYPE_MESSAGE:
        body = elacp_anybody_as_friendmsg(ref);
        break;
    case ELACP_TYPE_INVITE_REQUEST:
        body = elacp_anybody_as_invitereq(ref);
        break;
    case ELACP_TYPE_INVITE_RESPONSE:
        body = elacp_anybody_as_invitersp(ref);
        break;
    case ELACP_TYPE_BULKMSG:
        body = elacp_anybody_as_bulkmsg(ref);
        break;
    default:
        assert(0);
        return NULL;
    }

    elacp_packet_start_as_root(&builder);
    elacp_packet_type_add(&builder, cp->type);
    elacp_packet_body_add(&builder, body);
    if (!elacp_packet_end_as_root(&builder)) {
        flatcc_builder_clear(&builder);
        return NULL;
    }

    encoded_data = flatcc_builder_finalize_buffer(&builder, encoded_len);
    flatcc_builder_clear(&builder);

    return encoded_data;
}

ElaCP *elacp_decode(const uint8_t *data, size_t len)
{
    ElaCP *cp;
    struct elacp_packet_t pkt;
    struct elacp_table_t  tbl;
    elacp_packet_table_t packet;
    flatbuffers_uint8_vec_t vec;
    uint8_t type;

    packet = elacp_packet_as_root(data);
    if (!packet)
        return NULL;

    type = elacp_packet_type(packet);
    switch(type) {
    case ELACP_TYPE_USERINFO:
    case ELACP_TYPE_FRIEND_REQUEST:
    case ELACP_TYPE_MESSAGE:
    case ELACP_TYPE_INVITE_REQUEST:
    case ELACP_TYPE_INVITE_RESPONSE:
    case ELACP_TYPE_BULKMSG:
        break;
    default:
        //TODO: clean resource for 'packet'; (how ?)
        return NULL;
    }

    cp = elacp_create(type, NULL);
    if (!cp) {
        //TODO: clean resource for 'packet'; (how ?)
        return NULL;
    }
    pkt.u.cp = cp;

    if (!elacp_packet_body_is_present(packet)) {
        elacp_free(cp);
        return NULL;
    }

    switch(type) {
    case ELACP_TYPE_USERINFO:
        tblinfo = elacp_packet_body(packet);
        if (elacp_userinfo_name_is_present(tblinfo))
            pktinfo->name = elacp_userinfo_name(tblinfo);
        pktinfo->descr  = elacp_userinfo_descr(tblinfo);
        pktinfo->gender = elacp_userinfo_gender(tblinfo);
        pktinfo->phone  = elacp_userinfo_phone(tblinfo);
        pktinfo->email  = elacp_userinfo_email(tblinfo);
        pktinfo->region = elacp_userinfo_region(tblinfo);
        pktinfo->has_avatar = elacp_userinfo_avatar(tblinfo);
        break;

    case ELACP_TYPE_FRIEND_REQUEST:
        tblfreq = elacp_packet_body(packet);
        pktfreq->name  = elacp_friendreq_name(tblfreq);
        pktfreq->descr = elacp_friendreq_descr(tblfreq);
        pktfreq->hello = elacp_friendreq_hello(tblfreq);
        break;

    case ELACP_TYPE_MESSAGE:
        tblfmsg = elacp_packet_body(packet);
        pktfmsg->msg = vec = elacp_friendmsg_msg(tblfmsg);
        pktfmsg->len = flatbuffers_uint8_vec_len(vec);
        if (elacp_friendmsg_ext_is_present(tblfmsg))
            cp->ext = elacp_friendmsg_ext(tblfmsg);
        break;

    case ELACP_TYPE_INVITE_REQUEST:
        tblireq = elacp_packet_body(packet);
        pktireq->tid = elacp_invitereq_tid(tblireq);
        pktireq->totalsz = elacp_invitereq_totalsz(tblireq);
        if (elacp_invitereq_bundle_is_present(tblireq))
             pktireq->bundle = elacp_invitereq_bundle(tblireq);
        pktireq->data = vec = elacp_invitereq_data(tblireq);
        pktireq->len = flatbuffers_uint8_vec_len(vec);
        if (elacp_invitereq_ext_is_present(tblireq))
            cp->ext = elacp_invitereq_ext(tblireq);
        break;

    case ELACP_TYPE_INVITE_RESPONSE:
        tblirsp = elacp_packet_body(packet);
        pktirsp->tid = elacp_invitersp_tid(tblirsp);
        pktireq->totalsz = elacp_invitersp_totalsz(tblirsp);
        if (elacp_invitersp_bundle_is_present(tblirsp))
             pktirsp->bundle = elacp_invitersp_bundle(tblirsp);
        pktirsp->status = elacp_invitersp_status(tblirsp);
        if (pktirsp->status)
            pktirsp->reason = elacp_invitersp_reason(tblirsp);
        else {
            pktirsp->data = vec = elacp_invitersp_data(tblirsp);
            pktirsp->len = flatbuffers_uint8_vec_len(vec);
        }
        if (elacp_invitersp_ext_is_present(tblirsp))
            cp->ext = elacp_invitersp_ext(tblirsp);
        break;

    case ELACP_TYPE_BULKMSG:
        tblbmsg = elacp_packet_body(packet);
        pktbmsg->tid = elacp_bulkmsg_tid(tblbmsg);
        pktbmsg->data = vec = elacp_bulkmsg_data(tblbmsg);
        pktbmsg->len = flatbuffers_uint8_vec_len(vec);
        pktbmsg->totalsz = elacp_bulkmsg_totalsz(tblbmsg);
        break;

    default:
        assert(0);
        break;
    }

    return cp;
}

int elacp_decode_pullmsg(const uint8_t *data, ElaCPPullMsg *pullmsg)
{
    elacp_pullmsg_table_t pmsg_tbl;
    flatbuffers_uint8_vec_t vec;

    assert(data && pullmsg);
    memset(pullmsg, 0, sizeof(*pullmsg));

    pmsg_tbl = elacp_pullmsg_as_root(data);
    if (!pmsg_tbl)
        return -1;

    pullmsg->id = elacp_pullmsg_id(pmsg_tbl);
    pullmsg->from = elacp_pullmsg_from(pmsg_tbl);
    pullmsg->type = elacp_pullmsg_type(pmsg_tbl);
    pullmsg->timestamp = elacp_pullmsg_timestamp(pmsg_tbl);
    pullmsg->address = elacp_pullmsg_address(pmsg_tbl);
    pullmsg->payload = vec = elacp_pullmsg_payload(pmsg_tbl);
    pullmsg->payload_sz = flatbuffers_uint8_vec_len(vec);

    return 0;
}