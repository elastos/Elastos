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

#ifndef __EASYFILE_H__
#define __EASYFILE_H__

#include <stdint.h>

#include "ela_filetransfer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \~English
 * Carrier file progress callbacks.
 */
typedef struct ElaFileProgressCallbacks {
    /**
     * \~English
     * An application-defined function that handles file transfer connection
     * state changed event.
     *
     * @param
     *      state           [in] The file transfer connection state.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*state_changed)(FileTransferConnection state, void *context);

    /**
     * \~English
     * An application-defined function that handles file sent event.
     *
     * @param
     *      length          [in] The amount of data sent.
     * @param
     *      totalsz         [in] The total mount of transfering file.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*sent)(size_t length, uint64_t totalsz, void *context);

    /**
     * \~English
     * An application-defined function that handles file received event.
     *
     * @param
     *      length          [in] The amount of data sent.
     * @param
     *      totalsz         [in] The total mount of transferring file.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*received)(size_t length, uint64_t totalsz, void *context);
} ElaFileProgressCallbacks;

typedef struct EasyFile {
    ElaFileTransfer         *ft;

    ElaFileProgressCallbacks callbacks;
    void                    *callbacks_context;

    FILE                    *fp;
    char                    fileid[ELA_MAX_FILE_ID_LEN + 1];
    char                    filename[PATH_MAX];
    uint64_t                filesz;
    uint64_t                offset;

    int                     sys_errno;
    int                     carrier_errno;
} EasyFile;

int ela_file_send(ElaCarrier *w, const char *address, const char *filename,
                  ElaFileProgressCallbacks *callbacks, void *context);

int ela_file_recv(ElaCarrier *w, const char *address, const char *filename,
                  ElaFileProgressCallbacks *callbacks, void *context);

void ela_set_error(int error);

#ifdef __cplusplus
}
#endif

#endif /* __EASYFILE_H__ */
