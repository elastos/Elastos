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
 * Carrier transfer file ID max length.
 */
#define ELA_MAX_FILE_NAME_LEN   255

/**
 * \~English
 * Carrier transfer file ID max length.
 */
#define ELA_MAX_FILE_ID_LEN     ELA_MAX_ID_LEN

/**
 * \~English
 * A defined type representing the transaction of file transfer.
 */
typedef struct ElaFileTransfer ElaFileTransfer;

/**
 * \~English
 * A structure representing the file transfer meta information.
 *
 * Two peer carrier node use this structure to communicate of what file
 * or file list to transfer.
 */
typedef struct ElaFileTransferInfo {
    /**
     * \~English
     * File name of transfer file, without file path.
     */
    char filename[ELA_MAX_FILE_NAME_LEN + 1];

    /**
     * \~English
     * Unique fileid of transfer file, which is being unique to current
     * node and remote friend node.
     */
    char fileid[ELA_MAX_FILE_ID_LEN + 1];

    /**
     * \~English
     * Total file size of transfer file.
     */
    uint64_t size;
} ElaFileTransferInfo;

/**
 * \~English
 * Carrier filetransfer peers' connection numeration.
 * The filetransfer connection will be changed through whole establishment
 * process between peers.
 */
typedef enum FileTransferConnection {
    /** The filetransfer connection is initalized. */
    FileTransferConnection_initialized,

    /** The filetransfer connection is being established.*/
    FileTransferConnection_connecting,

    /** The filetransfer connection has been connected. */
    FileTransferConnection_connected,

    /** The filetransfer connection failed with some reason. */
    FileTransferConnection_failed,

    /** The filetransfer connection is closed and disconnected. */
    FileTransferConnection_closed
} FileTransferConnection;

typedef enum CancelReason {
    CancelReason_normal,
    CancelReason_timeout,
    CancelReason_other
} CancelReason;

/**
 * \~English
 * Carrier filetransfer callacks.
 */
typedef struct ElaFileTransferCallbacks {
    /**
     * \~English
     * An application-defined function that perform the state changed event.
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
     * An application-defined function that receive transfer file information
     * event.
     *
     * @param
     *      filetransfer    [in] A handle to the Carrier file transfer
     *                           instance.
     * @param
     *      fileid          [in] The file identifier.
     * @param
     *      filename        [in] The file name;
     * @param
     *      size            [in] The total file size.
     * @param
     *      context         [in] The application defined context data.
     *
     * @return
     *      True on success, or false if an error occurred.
     *      The filetransfer channel will continue to open only this callback
     *      return true, otherwise the channel will be closed.
     */
    bool (*file)(ElaFileTransfer *filetransfer, const char *fileid,
                          const char *filename, uint64_t size, void *context);
    /**
     * \~English
     * An application-defined function that receive file transfer pull request
     * event.
     *
     * @param
     *      filetransfer    [in] A handle to the Carrier file transfer
     *                           instance.
     * @param
     *      fileid          [in] The file identifier.
     * @param
     *      offset          [in] The start offset of data to transfer in file.
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
     *      fileid          [in] The unique idenfier of transfering file.
     * @param
     *      data            [in] The pointer to received data.
     * @param
     *      length          [in] The length of received data.
     * @param
     *      context         [in] The application defined context data.
     */
    bool (*data)(ElaFileTransfer *filetransfer, const char *fileid,
                 const uint8_t *data, size_t length, void *context);

    /**
     * \~English
     * An application-defined function that perform paused control packet
     * to pause file transfer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique idenfier of transfering file.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*pending)(ElaFileTransfer *filetransfer, const char *fileid,
                   void *context);

    /**
     * \~English
     * An application-defined function that perform resume control packet
     * to resume file transfer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique idenfier of transfering file.
     * @param
     *      context         [in] The application defined context data.
     */
    void (*resume)(ElaFileTransfer *filetransfer, const char *fileid,
                   void *context);

    /**
     * \~English
     * An application-defined function that perform cancel control packet
     * to cancel file transfer.
     *
     * @param
     *      filetransfer    [in] A handle to file transfer instance.
     * @param
     *      fileid          [in] The unique idenfier of transfering file.
     * @param
     *      reason          [in] Cancel transfer reason code.
     *
     * @param
     *      context         [in] The application defined context data.
     */
    void (*cancel)(ElaFileTransfer *filetransfer, const char *fileid,
                   CancelReason reason, void *context);

} ElaFileTransferCallbacks;

/**
 * \~English
 * Generate unique file identifer with random algorithm.
 *
 * Application can call this function to explicitly generate transfer fileid,
 * or let filetransfer module to generate fileid implicitly.
 *
 * @param
 *      fileid          [in] The buffer to receiver generated fileid.
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
 * An application-defined function that receive file transfer connect
 * request event.
 *
 * ElaFileTransferConnectCallback is the callback function type.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      address         [in] The target address.
 * @param
 *      fileinfo        [in] Array of file informatios.
 * @param
 *      context         [in] The application defined context data.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
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
 * As to send request to transfer file, application may or may not feed file
 * information that want to transfer. And for receiving side, application
 * should feed file information received from request callback.
 *
 * @param
 *      carrier         [in] A handle to the Carrier node instance.
 * @param
 *      address         [in] The target address.
 * @param
 *      fileinfo        [in] The information of file to transfer, could be NULL.
 * @param
 *      callbacks       [in] A pointer to ElaFileTransferCallbacks to handle
 *                           all events related to new filetransfer instance.
 * @param
 *      context         [in] The application defined context data.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
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
 * Each file has it's unique fileid used between two peers.
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
char *ela_file_transfer_get_fileid(ElaFileTransfer *filetransfer,
                                   const char *filename,
                                   char *fileid, size_t length);

/**
 * \~English
 * Get an file name according specific fileid.
 *
 * Each file has it's unique fileid used between two peers.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The target file identifer.
 * @param
 *      filename        [in] The buffer to receive file name.
 * @param
 *      length          [in] The length of buffer to receive file name.
 *
 * @return
 *      File name is returned if filetransfer instance has specific fileid,
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
 * Accept filet ransfer connection request.
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
 * Add a list of file or a file to queue of file transfer.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileinfo_list   [in] An array of file information.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_add(ElaFileTransfer *filetransfer,
                         const ElaFileTransferInfo *fileinfo_list);

/**
 * \~English
 * To send pull request to transfer file with specified file.
 *
 * @param
 *      filetransfer    [in] A handle to the Carrier file transfer instance.
 * @param
 *      fileid          [in] The file identifier.
 * @param
 *      offset          [in] The start offset of data to transfer in file.
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
 *      data            [in] The data to transfer for file.
 * @param
 *      length          [in] The length of data to transfer for file.
 *
 * @return
 *      0 on success, or -1 if an error occurred. The specific error code
 *      can be retrieved by calling ela_get_error().
 */
CARRIER_API
int ela_filetransfer_send(ElaFileTransfer *filetransfer, const char *fileid,
                          const uint8_t *data, size_t length);

/**
 * \~English
 * Cancel transfering file data with specified fileid.
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
int ela_filetransfer_cancel(ElaFileTransfer *filetransfer, const char *fileid);

/**
 * \~English
 * Pause transfering file data with specified fileid.
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
int ela_filetransfer_pause(ElaFileTransfer *filetransfer, const char *fileid);

/**
 * \~English
 * Resume transfering file data with specified fileid.
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

#ifdef __cplusplus
}
#endif

#if defined(__APPLE__)
#pragma GCC diagnostic pop
#endif

#endif /* __ELA_FILETRANSFER_H__ */
