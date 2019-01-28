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

#ifndef __ELA_FILETRANSFER_H__
#define __ELA_FILETRANSFER_H__

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <ela_carrier.h>

#if defined(__APPLE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CARRIER_STATIC)
#define CARRIER_API
#elif defined(CARRIER_DYNAMIC)
  #ifdef CARRIER_BUILD
    #if defined(_WIN32) || defined(_WIN64)
      #define CARRIER_API        __declspec(dllexport)
    #else
      #define CARRIER_API        __attribute__((visibility("default")))
    #endif
  #else
    #if defined(_WIN32) || defined(_WIN64)
      #define CARRIER_API        __declspec(dllimport)
    #else
      #define CARRIER_API        __attribute__((visibility("default")))
    #endif
  #endif
#else
#define CARRIER_API
#endif

/**
 * \~English
 * Carrier transfer file name max length.
 */
#define ELA_MAX_FILE_NAME_LEN   255

/**
 * \~English
 * Carrier transfer file ID max length.
 */
#define ELA_MAX_FILE_ID_LEN     ELA_MAX_ID_LEN

/**
 * \~English
 * A defined type representing the transaction of file transfer between a friend
 * and us.
 */
typedef struct ElaFileTransfer ElaFileTransfer;

/**
 * \~English
 * A structure representing the file transfer meta information.
 *
 * Two peer carrier nodes use this structure to declare which file to transfer.
 */
typedef struct ElaFileTransferInfo {
    /**
     * \~English
     * File name of file to transfer, without file path.
     */
    char filename[ELA_MAX_FILE_NAME_LEN + 1];

    /**
     * \~English
     * Unique fileid of file to transfer, which is being unique in a file
     * transfer instance.
     */
    char fileid[ELA_MAX_FILE_ID_LEN + 1];

    /**
     * \~English
     * Total file size of file transfer.
     */
    uint64_t size;

    /**
     * \~English
     * The user defined data attached to this transfer.
     */
    void *userdata;
} ElaFileTransferInfo;

/**
 * \~English
 * Carrier file transfer connection state enumeration.
 */
typedef enum FileTransferConnection {
    /** The file transfer connection is initialized. */
    FileTransferConnection_initialized = 1,

    /** The file transfer connection is connecting.*/
    FileTransferConnection_connecting,

    /** The file transfer connection has been established. */
    FileTransferConnection_connected,

    /** The file transfer connection is closed and disconnected. */
    FileTransferConnection_closed,

    /** The file transfer connection failed with some reason. */
    FileTransferConnection_failed
} FileTransferConnection;

/**
 * \~English
 * Carrier file transfer callbacks.
 */
typedef struct ElaFileTransferCallbacks {
    /**
     * \~English
     * An application-defined function that handle the state changed event.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      state           [in] The file transfer connection state.
     * @param
     *      context         [in] The application defined context data.
     *
     */
    void (*state_changed)(ElaFileTransfer *filetransfer,
                          FileTransferConnection state, void *context);

    /**
     * \~English
     * An application-defined function that handle transfer file request event.
     *
     * @param
     *      filetransfer    [in] A handle to the Carrier file transfer
     *                           instance.
     * @param
     *      fileid          [in] The file identifier.
     * @param
     *      filename        [in] The file name.
     * @param
     *      size            [in] The total file size.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*file)(ElaFileTransfer *filetransfer, const char *fileid,
                 const char *filename, uint64_t size, void *context);
    /**
     * \~English
     * An application-defined function that handle file transfer pull request
     * event.
     *
     * @param
     *      filetransfer    [in] A handle to the Carrier file transfer
     *                           instance.
     * @param
     *      fileid          [in] The file identifier.
     * @param
     *      offset          [in] The offset of file where transfer begins.
     * @param
     *      context         [in] The application defined context data.
     */
     void (*pull)(ElaFileTransfer *filetransfer, const char *fileid,
                  uint64_t offset, void *context);

    /**
     * \~English
     * An application-defined function that perform receiving data.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      data            [in] The pointer to received data(NULL if @length
     *                           is zero).
     * @param
     *      length          [in] The length of received data.
     * @param
     *      context         [in] The application defined context data.
     *
     * @return
     *      Return false if you require no more data, otherwise return true.
     */
    bool (*data)(ElaFileTransfer *filetransfer, const char *fileid,
                 const uint8_t *data, size_t length, void *context);

    /**
     * \~English
     * An application-defined function that handles pause file transfer
     * notification from the peer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*pending)(ElaFileTransfer *filetransfer, const char *fileid,
                   void *context);

    /**
     * \~English
     * An application-defined function that handles resume file transfer
     * notification from the peer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*resume)(ElaFileTransfer *filetransfer, const char *fileid,
                   void *context);

    /**
     * \~English
     * An application-defined function that handles cancel file transfer
     * notification from the peer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique identifier of transferring file.
     * @param
     *      status          [in] Cancel transfer status code.
     * @param
     *      reason          [in] The string presents cancel reason.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*cancel)(ElaFileTransfer *filetransfer, const char *fileid,
                   int status, const char *reason, void *context);

} ElaFileTransferCallbacks;

/**
 * \~English
 * Generate unique file identifier with random algorithm.
 *
 * Application can call this function to explicitly generate transfer fileid,
 * or let file transfer module generate fileid implicitly.
 *
 * @param
 *      fileid          [in] The buffer to receive generated file identifier.
 * @param
 *      length          [in] The buffer length.
 *
 * @return
 *      The generated unique file identifier.
 */
CARRIER_API
char *ela_filetransfer_fileid(char *fileid, size_t length);

/**
 * \~English
 * An application-defined function that handles file transfer connect
 * request.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      address         [in] The requesting address.
 * @param
 *      fileinfo        [in] Information of the file to transfer.
 * @param
 *      context         [in] The application defined context data.
 */
typedef void ElaFileTransferConnectCallback(ElaCarrier *carrier,
                    const char *address, const ElaFileTransferInfo *fileinfo,
                    void *context);

/**
 * \~English
 * Initialize file transfer extension.
 *
 * The application must initialize the file transfer extension before
 * calling any file transfer API.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      callback        [in] A pointer to file transfer connect callback.
 * @param
 *      context         [in] The application defined context data.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_init(ElaCarrier *carrier,
                          ElaFileTransferConnectCallback *callback,
                          void *context);

/**
 * \~English
 * Clean up Carrier file transfer extension.
 *
 * The application should call ela_file_transfer_cleanup before quit,
 * to clean up the resources associated with the extension.
 *
 * If the extension is not initialized, this function has no effect.
 */
CARRIER_API
void ela_filetransfer_cleanup(ElaCarrier *carrier);

/**
 * \~English
 * Open a file transfer instance.
 *
 * The application must open file transfer instance before sending
 * request/reply to transfer file.
 *
 * As to send request to transfer file, application may or may not feed
 * information of the file that we want to transfer. And for receiving side,
 * application may feed file information received from connect request
 * callback.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      address         [in] The target address.
 * @param
 *      fileinfo        [in] The information of file to transfer, could be NULL.
 * @param
 *      callbacks       [in] A pointer to ElaFileTransferCallbacks to handle
 *                           all events related to new file transfer instance.
 * @param
 *      context         [in] The application defined context data.
 *
 * @return
 *      Return an ElaFileTransfer instance on success, NULL otherwise(The
 *      specific error code can be retrieved by calling ela_get_error()).
 */
CARRIER_API
ElaFileTransfer *ela_filetransfer_new(ElaCarrier *carrier,
                                      const char *address,
                                      const ElaFileTransferInfo *fileinfo,
                                      ElaFileTransferCallbacks *callbacks,
                                      void *context);

/**
 * \~English
 * Close file transfer instance.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 */
CARRIER_API
void ela_filetransfer_close(ElaFileTransfer *filetransfer);

/**
 * \~English
 * Get an unique file identifier of specified file.
 *
 * Each file has its unique fileid used between two peers.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      filename        [in] The target file name.
 * @param
 *      fileid          [in] A buffer to receive file identifier.
 * @param
 *      length          [in] The length of buffer to receive file identifier.
 *
 * @return
 *      Fileid is returned if filetransfer instance has file info of filename,
 *      otherwise, NULL value is returned. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
char *ela_filetransfer_get_fileid(ElaFileTransfer *filetransfer,
                                  const char *filename,
                                  char *fileid, size_t length);

/**
 * \~English
 * Get file name by fileid.
 *
 * Each file has its unique fileid used between two peers.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The target file identifier.
 * @param
 *      filename        [in] The buffer to receive file name.
 * @param
 *      length          [in] The length of buffer to receive file name.
 *
 * @return
 *      File name is returned if filetransfer instance has fileid specified,
 *      otherwise, NULL value is returned. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
char *ela_filetransfer_get_filename(ElaFileTransfer *filetransfer,
                                    const char *fileid,
                                    char *filename, size_t length);

/**
 * \~English
 * Send a file transfer connect request to target peer.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_connect(ElaFileTransfer *filetransfer);

/**
 * \~English
 * Accept file transfer connection request.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_accept_connect(ElaFileTransfer *filetransfer);

/**
 * \~English
 * Add a file to queue of file transfer.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileinfo        [in] Information of the file to be added.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_add(ElaFileTransfer *filetransfer,
                         const ElaFileTransferInfo *fileinfo);

/**
 * \~English
 * To send pull request to transfer file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      offset          [in] The offset of file where transfer begins.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_pull(ElaFileTransfer *filetransfer, const char *fileid,
                          uint64_t offset);

/**
 * \~English
 * To transfer file data with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      data            [in] The data to transfer for file(MUST be NULL if
 *                           @length is zero).
 * @param
 *      length          [in] The length of data to transfer for file
 *                           (COULD be zero. In that case, the receiver will
 *                            get ElaFileTransferCallbacks::data callback with
 *                            argument @length being zero).
 *
 * @return
 *      Sent bytes on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
ssize_t ela_filetransfer_send(ElaFileTransfer *filetransfer, const char *fileid,
                          const uint8_t *data, size_t length);

/**
 * \~English
 * Cancel transferring file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      status          [in] Cancel transfer status code.
 * @param
 *      reason          [in] Cancel transfer reason.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_cancel(ElaFileTransfer *filetransfer, const char *fileid,
                            int status, const char *reason);

/**
 * \~English
 * Pause transferring file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_pend(ElaFileTransfer *filetransfer, const char *fileid);

/**
 * \~English
 * Resume transferring file with specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_resume(ElaFileTransfer *filetransfer, const char *fileid);


/**
 * \~English
 * Bind userdata to specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      userdata        [in] The pointer to userdata.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_set_userdata(ElaFileTransfer *filetransfer,
                                  const char *fileid, void *userdata);

/**
 * \~English
 * Get userdata bound to specified fileid.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 *
 * @return
 *      userdata on success, or NULL if an error occurred. The specific error
 *      code can be retrieved by calling ela_get_error().
 */
CARRIER_API
void *ela_filetransfer_get_userdata(ElaFileTransfer *ft, const char *fileid);

#ifdef __cplusplus
}
#endif

#if defined(__APPLE__)
#pragma GCC diagnostic pop
#endif

#endif /* __ELA_FILETRANSFER_H__ */
