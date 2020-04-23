/*
 * Copyright (c) 2020 Elastos Foundation
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

#include "extension.h"
#include "ela_carrier_impl.h"

#if defined(_WIN32)
#include <malloc.h>
#define alloca _alloca
#else
#include <alloca.h>
#endif

static
void extension_destroy(void *obj)
{
    CarrierExtension *ext = (CarrierExtension *)obj;

    ext->carrier->carrier_extesion = NULL;
}

static
void on_friend_invite(ElaCarrier *carrier, const char *from, const char *bundle,
                      const void *data, size_t len, void *context)
{
    CarrierExtension *ext = (CarrierExtension *)context;
    ExtensionInviteCallback *callback = (ExtensionInviteCallback *)ext->user_callback;

    (void)bundle;

    vlogD("CarrierExtension: Extension request from %s with data: %s", from, (char *)data);

    callback(carrier, from, data, len, ext->user_context);
}

int extension_init(ElaCarrier *carrier, ExtensionInviteCallback *callback, void *context)
{
    CarrierExtension *ext;

    if (!carrier) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    pthread_mutex_lock(&carrier->ext_mutex);
    if (carrier->carrier_extesion) {
        ref(carrier->carrier_extesion);
        pthread_mutex_unlock(&carrier->ext_mutex);
        vlogD("CarrierExtension: Extension initialized already, ref counter(%d).",
              nrefs(carrier->carrier_extesion));
        return 0;
    }

    ext = (CarrierExtension *)rc_zalloc(sizeof(CarrierExtension),
                                        extension_destroy);
    if (!ext) {
        pthread_mutex_unlock(&carrier->ext_mutex);
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }

    ext->carrier = carrier;
    ext->friend_invite_cb = on_friend_invite;
    ext->friend_invite_context = ext;
    ext->user_callback = callback;
    ext->user_context = context;

    carrier->carrier_extesion = ext;
    pthread_mutex_unlock(&carrier->ext_mutex);

    vlogD("CarrierExtension: Initializing carrier extension finished.");

    return 0;
}

void extension_cleanup(ElaCarrier *carrier)
{
    if (!carrier)
        return;

    pthread_mutex_lock(&carrier->ext_mutex);
    if (!carrier->carrier_extesion) {
        pthread_mutex_unlock(&carrier->ext_mutex);
        return;
    }

    if (nrefs(carrier->carrier_extesion) > 1) {
        vlogD("CarrierExtension: Extension cleanup, ref counter(%d).",
              nrefs(carrier->carrier_extesion));
    }

    deref(carrier->carrier_extesion);
    pthread_mutex_unlock(&carrier->ext_mutex);
}

static
void on_friend_invite_reply(ElaCarrier *carrier, const char *from,
                            const char *bundle, int status, const char *reason,
                            const void *data, size_t len, void *context)
{
    void **callback_ctx = (void *)context;
    ExtensionInviteReplyCallback *callback = (ExtensionInviteReplyCallback *)callback_ctx[0];
    void *user_ctx = (void *)callback_ctx[1];

    (void)bundle;

    deref(callback_ctx);

    vlogD("CarrierExtension: Extension response from %s with data: %s",
          from, (const char *)data);

    callback(carrier, from, status, reason, data, len, user_ctx);
}

int extension_invite_friend(ElaCarrier *carrier, const char *to,
                            const void *data, size_t len,
                            ExtensionInviteReplyCallback *callback,
                            void *context)
{
    int rc;
    char *ext_to;
    void **callback_ctx;

    if (!carrier || !carrier->carrier_extesion || !to || !*to ||
        !ela_id_is_valid(to) || !data || !len || len > ELA_MAX_INVITE_DATA_LEN ||
        !callback) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    callback_ctx = rc_zalloc(sizeof(void *) << 1, NULL);
    if (!callback_ctx) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_OUT_OF_MEMORY));
        return -1;
    }
    callback_ctx[0] = callback;
    callback_ctx[1] = context;

    ext_to = (char *)alloca(ELA_MAX_ID_LEN + strlen(carrier_extension_name) + 2);
    strcpy(ext_to, to);
    strcat(ext_to, ":");
    strcat(ext_to, carrier_extension_name);

    rc = ela_invite_friend(carrier, ext_to, NULL, data, len,
                           on_friend_invite_reply, callback_ctx);

    vlogD("CarrierExtension: Extension invitation to %s %s.", to,
          rc == 0 ? "success" : "failed");

    if (rc < 0)
        deref(callback_ctx);

    return rc;
}

int extension_reply_friend_invite(ElaCarrier *carrier, const char *to,
                                  int status, const char *reason,
                                  const void *data, size_t len)
{
    char *ext_to;
    int rc;

    if (!carrier || !carrier->carrier_extesion) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (status && (!reason || strlen(reason) > ELA_MAX_INVITE_REPLY_REASON_LEN
                   || data || len > 0)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!status && (reason || !data || !len || len > ELA_MAX_INVITE_DATA_LEN)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    if (!to || !*to || !ela_id_is_valid(to)) {
        ela_set_error(ELA_GENERAL_ERROR(ELAERR_INVALID_ARGS));
        return -1;
    }

    ext_to = (char *)alloca(ELA_MAX_ID_LEN + strlen(carrier_extension_name) + 2);
    strcpy(ext_to, to);
    strcat(ext_to, ":");
    strcat(ext_to, carrier_extension_name);

    rc = ela_reply_friend_invite(carrier, ext_to, NULL, status, reason,
                                 data, len);

    vlogD("CarrierExtension: Extension reply to %s %s.", to,
          rc == 0 ? "success" : "failed");

    return rc;
}
