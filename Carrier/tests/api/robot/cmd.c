#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "ela_carrier.h"
#include "ela_session.h"

#include "cmd.h"
#include "tests.h"
#include "test_helper.h"

#define CHK_ARGS(exp) if (!(exp)) { \
        robot_log_error("Invalid command syntax\n"); \
        return; \
    }

struct SessionContextExtra {
    int init_flag;

    char remote_sdp[2048];
    size_t sdp_len;
    char test_peer_id[(ELA_MAX_ID_LEN + 1) * 2];
};

static SessionContextExtra session_extra = {
    .init_flag = 0,

    .remote_sdp = {0},
    .sdp_len = 0,
    .test_peer_id = {0}
};

static void session_request_callback(ElaCarrier *w, const char *from,
                                     const char *sdp, size_t len, void *context)
{
    SessionContextExtra *extra = ((SessionContext *)context)->extra;

    extra->sdp_len = len;
    strncpy(extra->remote_sdp, sdp, len);
    extra->remote_sdp[len] = 0;

    strcpy(extra->test_peer_id, from);

    robot_ack("srequest received\n");
}

static void session_request_complete_callback(ElaSession *ws, int status,
                const char *reason, const char *sdp, size_t len, void *context)
{
    SessionContext *sctxt = ((TestContext *)context)->session;
    StreamContext *stream_ctxt = ((TestContext *)context)->stream;
    int rc;

    robot_log_debug("Session complete, status: %d, reason: %s\n", status,
                    reason ? reason : "null");

    if (status != 0) {
        assert(0 && "test client should confirm session request.\n");
        goto cleanup;
    }

    cond_wait(stream_ctxt->cond);
    if (!(stream_ctxt->state_bits & (1 << ElaStreamState_transport_ready))) {
        robot_log_error("Stream state not 'transport ready' state\n");
        goto cleanup;
    }

    rc = ela_session_start(ws, sdp, len);
    if (rc < 0) {
        robot_log_error("Start session for robot failed (0x%x)\n",
                        ela_get_error());
        goto cleanup;
    } else
        robot_log_debug("Start session for robot success");

    cond_wait(stream_ctxt->cond);
    if (!(stream_ctxt->state_bits & (1 << ElaStreamState_connecting))) {
        robot_log_error("Stream state not 'connnecting' state\n");
        goto cleanup;
    }

    cond_wait(stream_ctxt->cond);
    if (!(stream_ctxt->state_bits & (1 << ElaStreamState_connected))) {
        robot_log_error("Stream state not 'connected' state\n");
        goto cleanup;
    }

    robot_ack("sconnect success\n");
    return ;

cleanup:
    if (stream_ctxt->stream_id > 0) {
        ela_session_remove_stream(sctxt->session, stream_ctxt->stream_id);
        stream_ctxt->stream_id = -1;
    }
    if (sctxt->session) {
        ela_session_close(sctxt->session);
        sctxt->session = NULL;
    }
    robot_ack("sconnect failed\n");
}

static SessionContext session_context = {
    .request_cb = session_request_callback,
    .request_received = 0,
    .request_cond = NULL,

    .request_complete_cb = session_request_complete_callback,
    .request_complete_status = -1,
    .request_complete_cond = NULL,
    
    .session = NULL,
    .extra = &session_extra
};

static void stream_on_data(ElaSession *ws, int stream, const void *data,
                           size_t len, void *context)
{
    robot_log_debug("Stream [%d] received data [%.*s]\n", stream, (int)len,
                    (char*)data);
}

static void stream_state_changed(ElaSession *ws, int stream,
                                 ElaStreamState state, void *context)
{
    StreamContext *stream_ctxt = (StreamContext *)context;

    stream_ctxt->state = state;
    stream_ctxt->state_bits |= 1 << state;

    robot_log_debug("Stream [%d] state changed to: %s\n", stream,
                    stream_state_name(state));

    cond_signal(stream_ctxt->cond);
}

struct StreamContextExtra {
    struct {
        int will_open_confirm;
        int channel_id;
        int channel_error_state;
    } channels[MAX_CHANNEL_COUNT];

    int portforwarding_id;
};

static StreamContextExtra stream_extra = {
    .channels = { {0, 0, 0 } },
    .portforwarding_id = -1
};

static bool channel_open(ElaSession *ws, int stream, int channel,
                         const char *cookie, void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    robot_log_debug("Request open new channel %d on stream %d with cookie: %s.\n",
                     channel, stream, cookie);

    return extra->channels[channel-1].will_open_confirm;
}

static
void channel_opened(ElaSession *ws, int stream, int channel, void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    robot_log_debug("Channel %d opened.\n", channel);

    extra->channels[channel-1].channel_id = channel;
}

static void channel_close(ElaSession *ws, int stream, int channel,
                          CloseReason reason, void *context)
{
    StreamContextExtra *extra = ((StreamContext *)context)->extra;

    const char *state_name[] = {
        "Normal",
        "Timeout",
        "Error"
    };

    robot_log_debug("Channel %d closeing with %s.\n", channel, state_name[reason]);

    if (reason == CloseReason_Error || reason == CloseReason_Timeout)
        extra->channels[channel-1].channel_error_state = 1;
}

static  bool channel_data(ElaSession *ws, int stream, int channel,
                          const void *data, size_t len, void *context)
{
    robot_log_debug("stream [%d] channel [%d] received data [%.*s]\n",
                    stream, channel, (int)len, (char*)data);
    return true;
}

static
void channel_pending(ElaSession *ws, int stream, int channel, void *context)
{
    robot_log_debug("stream [%d] channel [%d] pend data.\n", stream, channel);
}

static
void channel_resume(ElaSession *ws, int stream, int channel, void *context)
{
    robot_log_debug("stream [%d] channel [%d] resume data.\n", stream, channel);
}

static ElaStreamCallbacks stream_callbacks = {
    .stream_data = stream_on_data,
    .state_changed = stream_state_changed,
    .channel_open = channel_open,
    .channel_opened = channel_opened,
    .channel_close = channel_close,
    .channel_data = channel_data,
    .channel_pending = channel_pending,
    .channel_resume = channel_resume
};

static Condition DEFINE_COND(stream_cond);

static StreamContext stream_context = {
    .cbs = &stream_callbacks,
    .stream_id = -1,
    .state = 0,
    .state_bits = 0,
    .cond = &stream_cond,
    .extra = &stream_extra
};

TestContext test_context = {
    .carrier = &carrier_context,
    .session = &session_context,
    .stream  = &stream_context
};

/*
 *  command format: ready"
 */
static void wmready(TestContext *context, int argc, char *argv[])
{
    CHK_ARGS(argc == 1);

    //ready_to_echo = 1;
}

/*
 * command format: fadd userid useraddr hello
 */
static void fadd(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    int rc;

    CHK_ARGS(argc == 4);

    if (ela_is_friend(w, argv[1]))
        ela_remove_friend(w, argv[1]);

    rc = ela_add_friend(w, argv[2], argv[3]);
    if (rc < 0) {
        robot_log_error("Add user %s to be friend error (0x%x)\n",
                         argv[2], ela_get_error());
    } else
        robot_log_debug("Add user %s to be friend success\n", argv[2]);
}

/*
 * command format: faccept userid entrusted expire
 */
static void faccept(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    int rc;

    CHK_ARGS(argc == 2);
    rc = ela_accept_friend(w, argv[1]);
    if (rc < 0) {
        if (ela_get_error() == ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST))
            robot_log_debug("User %s already is friend.\n", argv[1]);
        else
            robot_log_error("Accept friend request from user %s error (0x%x)\n",
                            argv[1], ela_get_error());
    } else
        robot_log_debug("Accept friend request from user %s success\n", argv[1]);
}

/*
 * command format: fmsg userid message
 */
static void fmsg(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    int rc;

    CHK_ARGS(argc == 3);

    rc = ela_send_friend_message(w, argv[1], argv[2], strlen(argv[2]) + 1);
    if (rc < 0)
        robot_log_error("Send message to friend %s error (0x%x)\n",
                        argv[1], ela_get_error());
    else
        robot_log_debug("Send message to friend %s success\n", argv[1]);
}

/*
 * command format: fremove userid
 */
static void fremove(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    int rc;

    CHK_ARGS(argc == 2);

    rc = ela_remove_friend(w, argv[1]);
    if (rc < 0)
        robot_log_error("Remove friend %s error (0x%x)\n", argv[1],
                        ela_get_error());
    else
        robot_log_debug("Remove friend %s success\n", argv[1]);
}

static void invite_response_callback(ElaCarrier *w, const char *friendid,
                                     int status, const char *reason,
                                     const char *data, size_t len, void *context)
{
    robot_log_debug("Received invite response from friend %s\n", friendid);

    if (status == 0) {
        robot_log_debug("Message within response: %.*s\n", (int)len, data);
    } else {
        robot_log_debug("Refused: %s\n", reason);
    }
}

/*
 * command format: finvite userid data
 */
static void finvite(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    int rc;

    CHK_ARGS(argc == 3);

    rc = ela_invite_friend(w, argv[1], argv[2], strlen(argv[2] + 1),
                               invite_response_callback, NULL);
    if (rc < 0)
        robot_log_error("Send invite request to friend %s error (0x%x)\n",
                        argv[1], ela_get_error());
    else
        robot_log_debug("Send invite request to friend %s success\n", argv[1]);
}

/*
 * command format: freplyinvite userid [ confirm data ] | [ refuse reason ]
 */
static void freplyinvite(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    int rc;
    int status = 0;
    const char *reason = NULL;
    const char *msg = NULL;
    size_t msg_len = 0;

    CHK_ARGS(argc == 4);

    if (strcmp(argv[2], "confirm") == 0) {
        msg = argv[3];
        msg_len = strlen(argv[3]) + 1;
    } else if (strcmp(argv[2], "refuse") == 0) {
        status = -1; // TODO: fix to correct status code.
        reason = argv[3];
    } else {
        robot_log_error("Unknown sub command: %s\n", argv[2]);
        return;
    }

    rc = ela_reply_friend_invite(w, argv[1], status, reason, msg, msg_len);
    if (rc < 0)
        robot_log_error("Reply invite request from friend %s error (0x%x)\n",
                        argv[1], ela_get_error());
    else
        robot_log_debug("Reply invite request from friend %s success\n", argv[1]);
}

/*
 * command format: kill
 */
static void wmkill(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;

    robot_log_info("wmkill: kill\n");
    ela_kill(w);
}

static void robot_context_reset(TestContext *context)
{
    SessionContext *sctxt = context->session;
    StreamContext *stream_ctxt = context->stream;

    sctxt->request_received = 0;
    sctxt->request_complete_status = -1;
    sctxt->session = NULL;

    sctxt->extra->init_flag = 0;
    sctxt->extra->remote_sdp[0] = 0;
    sctxt->extra->sdp_len = 0;
    sctxt->extra->test_peer_id[0] = 0;

    cond_reset(stream_ctxt->cond);
    stream_ctxt->stream_id = -1;
    stream_ctxt->state = 0;
    stream_ctxt->state_bits = 0;
    stream_ctxt->extra->portforwarding_id = -1;
}

/*
 * command format: sinit
 */
static void sinit(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    SessionContext *sctxt = context->session;
    int rc;

    CHK_ARGS(argc == 1);

    robot_context_reset(context);

    rc = ela_session_init(w, sctxt->request_cb, sctxt);
    if (rc < 0) {
        robot_log_error("session init failed: 0x%x\n", ela_get_error());
        robot_ack("sinit failed\n");
        return;
    } else {
        robot_log_debug("session init success.\n");
        sctxt->extra->init_flag = 1;
        robot_ack("sinit success\n");
    }
}

/*
 * command format: srequest peerid stream_options
 */
static void srequest(TestContext *context, int argc, char *argv[])
{
    SessionContext *sctxt = context->session;
    StreamContext *stream_ctxt = context->stream;
    int rc;

    CHK_ARGS(argc == 3);

    sctxt->session = ela_session_new(context->carrier->carrier, argv[1]);
    if (!sctxt->session) {
        robot_log_error("New session to %s failed: 0x%x\n", argv[1],
                        ela_get_error());
        goto cleanup;
    }

    stream_ctxt->stream_id = ela_session_add_stream(sctxt->session,
                                        ElaStreamType_text, atoi(argv[2]),
                                        stream_ctxt->cbs, stream_ctxt);
    if (stream_ctxt->stream_id < 0) {
        robot_log_error("Add text stream failed: 0x%x\n", ela_get_error());
        goto cleanup;
    }

    cond_wait(stream_ctxt->cond);
    if (!(stream_ctxt->state_bits & (1 << ElaStreamState_initialized))) {
        robot_log_error("Stream state not 'initialized' state\n");
        goto cleanup;
    }

    rc = ela_session_request(sctxt->session, sctxt->request_complete_cb, context);
    if (rc < 0) {
        robot_log_error("Session request failed: 0x%x\n", ela_get_error());
        goto cleanup;
    }

    robot_log_debug("sesion request succeed\n");
    robot_ack("srequest success\n");

    // the request complete callback should be invoked soon later, and do
    // the rest work.
    return;

cleanup:
    if (stream_ctxt->stream_id > 0) {
        ela_session_remove_stream(sctxt->session, stream_ctxt->stream_id);
        stream_ctxt->stream_id = -1;
    }

    if (sctxt->session) {
        ela_session_close(sctxt->session);
        sctxt->session = NULL;
    }

    robot_ack("srequest failed\n");
}

/*
 * command format: sreply comfirm stream_type stream_options
 *                 sreply refuse
 */
static void sreply(TestContext *context, int argc, char *argv[])
{
    SessionContext *sctxt = context->session;
    StreamContext *stream_ctxt = context->stream;
    int need_sreply_ack = 1;
    int rc;

    CHK_ARGS(argc == 4 || argc == 2);

    sctxt->session = ela_session_new(context->carrier->carrier, sctxt->extra->test_peer_id);
    if (!sctxt->session) {
        robot_log_error("New session to %s failed: 0x%x\n",
                        sctxt->extra->test_peer_id, ela_get_error());
        robot_ack("sreply failed\n");
        return;
    }

    if (strcmp(argv[1], "confirm") == 0) {
        int stream_type    = atoi(argv[2]);
        int stream_options = atoi(argv[3]);

        stream_ctxt->stream_id = ela_session_add_stream(sctxt->session,
                    stream_type, stream_options, stream_ctxt->cbs, stream_ctxt);
        if (stream_ctxt->stream_id < 0) {
            robot_log_error("Add stream failed: 0x%x\n", ela_get_error());
            goto cleanup;
        }

        cond_wait(stream_ctxt->cond);
        if (!(stream_ctxt->state_bits & (1 << ElaStreamState_initialized))) {
            robot_log_error("Stream is in %d state, not 'initialized'\n",
                            stream_ctxt->state);
            goto cleanup;
        }

        rc = ela_session_reply_request(sctxt->session, 0, NULL);
        if (rc < 0) {
            robot_log_error("Confirm session reqeust failed: 0x%x\n",
                            ela_get_error());
            goto cleanup;
        }

        cond_wait(stream_ctxt->cond);
        if (!(stream_ctxt->state_bits & (1 << ElaStreamState_transport_ready))) {
            robot_log_error("Stream is in %d state, not 'transport ready'\n",
                            stream_ctxt->state);
            goto cleanup;
        }

        robot_log_debug("Confirm session request success.\n");
        robot_ack("sreply success\n");

        need_sreply_ack = 0;

        rc = ela_session_start(sctxt->session, sctxt->extra->remote_sdp,
                                   sctxt->extra->sdp_len);
        if (rc < 0) {
            robot_log_error("Start session failed: 0x%x\n", ela_get_error());
            goto cleanup;
        }

        cond_wait(stream_ctxt->cond);
        if (!(stream_ctxt->state_bits & (1 << ElaStreamState_connecting))) {
            robot_log_error("Stream is in %d state, not 'connecting'\n",
                            stream_ctxt->state);
            goto cleanup;
        }

        cond_wait(stream_ctxt->cond);
        if (!(stream_ctxt->state_bits & (1 << ElaStreamState_connected))) {
            robot_log_error("Stream is in %d state, not 'connected'\n",
                            stream_ctxt->state);
            goto cleanup;
        }

        robot_ack("sconnect success\n");
        return;
    }
    else if (strcmp(argv[1], "refuse") == 0) {
        rc = ela_session_reply_request(sctxt->session, 1, "testing");
        if (rc < 0) {
            robot_log_error("Refuse session request failed: 0x%x\n",
                            ela_get_error());
            goto cleanup;
        }

        robot_log_debug("Refused session request success\n");
        robot_ack("sreply success\n");
        return;
    }
    else {
        printf("Unknown sub command.\n");
        robot_ack("sreply failed\n");
        return;
    }
    return;

cleanup:
    if (stream_ctxt->stream_id > 0) {
        ela_session_remove_stream(sctxt->session, stream_ctxt->stream_id);
        stream_ctxt->stream_id = -1;
    }

    if (need_sreply_ack && sctxt->session)
        ela_session_reply_request(sctxt->session, 2, "Error");

    if (sctxt->session) {
        ela_session_close(sctxt->session);
        sctxt->session = NULL;
    }

    if (need_sreply_ack)
        robot_ack("sreply failed\n");
    else
        robot_ack("sconnect failed\n");
}

static void sfree(TestContext *context, int argc, char *argv[])
{
    ElaCarrier *w = context->carrier->carrier;
    SessionContext *sctxt = context->session;
    StreamContext *stream_ctxt = context->stream;

    CHK_ARGS(argc == 1);

    if (stream_ctxt->stream_id > 0) {
        ela_session_remove_stream(sctxt->session, stream_ctxt->stream_id);

        cond_wait(stream_ctxt->cond);
        if (stream_ctxt->state != ElaStreamState_closed)
            robot_log_error("Stream should be closed, but (%d)\n",
                            stream_ctxt->state);
        stream_ctxt->stream_id = -1;
    }

    if (sctxt->session) {
        ela_session_close(sctxt->session);
        sctxt->session = NULL;
    }

    if (sctxt->extra->init_flag) {
        ela_session_cleanup(w);
        sctxt->extra->init_flag = 0;
    }

    robot_log_debug("Robot session cleanuped\n");
}

static void spfsvcadd(TestContext *context, int argc, char *argv[])
{
    ElaSession *session = context->session->session;
    int rc;
    PortForwardingProtocol protocol;

    CHK_ARGS(argc == 5);

    if (strcmp(argv[2], "tcp") == 0)
        protocol = PortForwardingProtocol_TCP;
    else {
        robot_log_error("Invalid portforwarding protocol: %s\n", argv[2]);
        robot_ack("spfsvcadd failed\n");
        return;
    }

    rc = ela_session_add_service(session, argv[1], protocol, argv[3], argv[4]);
    if (rc < 0) {
        robot_log_error("Add service %s failed (0x%x)\n", argv[1],
                        ela_get_error());
        robot_ack("spfsvcadd failed\n");
        return;
    }

    robot_log_debug("Added service %s to current session\n", argv[1]);
    robot_ack("spfsvcadd success\n");
}

static void spfsvcremove(TestContext *context, int argc, char *argv[])
{
    ElaSession *session = context->session->session;

    CHK_ARGS(argc == 2);

    ela_session_remove_service(session, argv[1]);

    robot_log_debug("Service %s removed\n", argv[1]);
}

static void spf_open(TestContext *context, int argc, char *argv[])
{
    ElaSession *session = context->session->session;
    StreamContext *stream_ctxt = context->stream;
    PortForwardingProtocol protocol;
    int pfid;

    CHK_ARGS(argc == 5);

    if (strcmp(argv[2], "tcp") == 0)
        protocol = PortForwardingProtocol_TCP;
    else {
        robot_log_error("Invalid portforwarding protocol %s\n", argv[2]);
        robot_ack("spfopen failed\n");
        return;
    }

    // Double check.
    if (stream_ctxt->extra->portforwarding_id > 0) {
        ela_stream_close_port_forwarding(session, stream_ctxt->stream_id,
                                        stream_ctxt->extra->portforwarding_id);
        stream_ctxt->extra->portforwarding_id = -1;
    }

    pfid = ela_stream_open_port_forwarding(session, stream_ctxt->stream_id,
                                        argv[1], protocol, argv[3], argv[4]);
    if (pfid <= 0) {
        robot_log_error("Open portforwarding for service %s failed: 0x%x\n",
                        argv[1], ela_get_error());
        robot_ack("spfopen failed\n");
        return;
    }

    stream_ctxt->extra->portforwarding_id = pfid;

    robot_log_debug("Open portforwarding for service %s on %s:%s success",
                    argv[1], argv[3], argv[4]);
    robot_ack("spfopen success\n");
}

static void spf_close(TestContext *context, int argc, char *argv[])
{
    ElaSession *session = context->session->session;
    StreamContext *stream_ctxt = context->stream;
    int rc;

    CHK_ARGS(argc == 1);

    if (stream_ctxt->extra->portforwarding_id > 0) {
        rc = ela_stream_close_port_forwarding(session, stream_ctxt->stream_id,
                                        stream_ctxt->extra->portforwarding_id);
        stream_ctxt->extra->portforwarding_id = -1;

        if (rc < 0) {
            robot_log_error("Close portforwarding failed: 0x%x\n",
                            ela_get_error());
            robot_ack("spfclose failed\n");
            return;
        }
    }
    robot_ack("spfclose success\n");
}

static void cready2open(TestContext *context, int argc, char *argv[])
{
    StreamContextExtra *extra = context->stream->extra;
    int will_open_confirm = 0;
    int i;

    CHK_ARGS(argc == 2);

    if (strcmp(argv[1], "confirm") == 0)
        will_open_confirm = 1;
    else if (strcmp(argv[1], "refuse") == 0)
        will_open_confirm = 0;
    else {
        robot_log_error("Unknown command option: %s\n", argv[2]);
        robot_ack("cready2open failed\n");
        return;
    }

    for (i = 0; i < MAX_CHANNEL_COUNT; i++)
        extra->channels[i].will_open_confirm = will_open_confirm;

    robot_ack("cready2open success\n");
}

static void cpend(TestContext *context, int argc, char *argv[])
{
    ElaSession *session = context->session->session;
    StreamContext *stream_ctxt = context->stream;
    int rc;

    CHK_ARGS(argc == 1);

    rc = ela_stream_pend_channel(session, stream_ctxt->stream_id,
                                 stream_ctxt->extra->channels[0].channel_id);
    if (rc < 0) {
        robot_log_error("Pending stream %d channel %d failed: 0x%x\n",
                        stream_ctxt->stream_id,
                        stream_ctxt->extra->channels[0].channel_id,
                        ela_get_error());

        robot_ack("cpend failed\n");
    } else {
        robot_ack("cpend success\n");
    }
}

static void cresume(TestContext *context, int argc, char *argv[])
{
    ElaSession *session = context->session->session;
    StreamContext *stream_ctxt = context->stream;
    int rc;

    CHK_ARGS(argc == 1);

    rc = ela_stream_resume_channel(session, stream_ctxt->stream_id,
                                   stream_ctxt->extra->channels[0].channel_id);
    if (rc < 0) {
        robot_log_error("Resume stream %d channel %d failed: 0x%x\n",
                        stream_ctxt->stream_id,
                        stream_ctxt->extra->channels[0].channel_id,
                        ela_get_error());

        robot_ack("cresume failed\n");
    } else {
        robot_ack("cresume success\n");
    }
}

static struct command {
    const char* name;
    void (*cmd_cb) (TestContext *context, int argc, char *argv[]);
} commands[] = {
    { "ready",        wmready      },
    { "fadd",         fadd         },
    { "faccept",      faccept      },
    { "fmsg",         fmsg         },
    { "fremove",      fremove      },
    { "finvite",      finvite      },
    { "freplyinvite", freplyinvite },
    { "kill",         wmkill       },
    { "sinit",        sinit        },
    { "srequest",     srequest     },
    { "sreply",       sreply       },
    { "sfree",        sfree        },
    { "spfsvcadd",    spfsvcadd    },
    { "spfsvcremove", spfsvcremove },
    { "spfopen",      spf_open     },
    { "spfclose",     spf_close    },
    { "cready2open",  cready2open  },
    { "cpend",        cpend        },
    { "cresume",      cresume      },
    { NULL, NULL},
};

char* read_cmd(void)
{
    int ch = 0;
    char *p;

    static int  cmd_len = 0;
    static char cmd_line[1024];

    ch = robot_ctrl_getchar();
    if (ch == EOF)
        return NULL;

    if (isprint(ch)) {
        cmd_line[cmd_len++] = ch;
    }
    else if (ch == 10 || ch == 13) {

        cmd_line[cmd_len] = 0;
        // Trim trailing spaces;
        for (p = cmd_line + cmd_len -1; p > cmd_line && isspace(*p); p--);
        *(++p) = 0;

        // Trim leading spaces;
        for (p = cmd_line; *p && isspace(*p); p++);

        cmd_len = 0;
        if (strlen(p) > 0)
            return p;
    } else {
        // ignored;
    }

    return NULL;
}

void do_cmd(TestContext *context, char *line)
{
    char *args[64];
    int count = 0;
    char *p;
    int word = 0;

    for (p = line; *p != 0; p++) {
        if (isspace(*p)) {
            *p = 0;
            word = 0;
        } else {
            if (word == 0) {
                args[count] = p;
                count++;
            }

            word = 1;
        }
    }

    if (count > 0) {
        struct command *p;

        for (p = commands; p->name; p++) {
            if (strcmp(args[0], p->name) == 0) {
                robot_log_debug("execute command %s\n", args[0]);
                p->cmd_cb(context, count, args);
                return;
            }
        }

        robot_log_error("Unknown command: %s\n", args[0]);
    }
}
