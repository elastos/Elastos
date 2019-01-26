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

import Foundation

/**
   A structure representing the carrier file transfer meta information.
   Two peer carrier nodes use this structure to declare which file to transfer.
 */
@objc(ELACarrierFileTransferInfo)
public class CarrierFileTransferInfo: NSObject {

    /// Carrier transfer file name max length.
    public static let MAX_FILE_NAME_LEN = 255

    /// Carrier transfer file ID max length.
    public static let MAX_FILE_ID_LEN = 45

    internal override init() {
        fileSize = 0
        super.init()
    }
    
    /**
        File name of file to transfer, without file path.
     */
    public var fileName: String?

    /**
        Unique fileid of file to transfer, which is being unique in a file
        transfer instance.
     */
    public var fileId: String?

    /**
        Total size of file to transfer.
     */
    public var fileSize: UInt64

    internal static func format(_ fileinfo: CarrierFileTransferInfo) -> String {
        return String(format: "FileTransferInfo: filename[%@]," +
                      " fileid[%@], fileSize[%@]",
                      String.toHardString(fileinfo.fileName?.description),
                      String.toHardString(fileinfo.fileId?.description),
                      String(fileinfo.fileSize))
    }

    public override var description: String {
        return CarrierFileTransferInfo.format(self)
    }
}

internal func convertCarrierFileTransferInfoToCFileTransferInfo(_ info: CarrierFileTransferInfo) -> CFileTransferInfo {
    var cInfo = CFileTransferInfo()

    info.fileName?.writeToCCharPointer(&cInfo.filename)
    info.fileId?.writeToCCharPointer(&cInfo.fileid)
    cInfo.size = info.fileSize

    return cInfo
}

internal func convertCFileTransferInfoToCarrierFileTransferInfo(_ cInfo: CFileTransferInfo) -> CarrierFileTransferInfo {
    let info = CarrierFileTransferInfo()

    var fileName = cInfo.filename
    info.fileName = String(cCharPointer: &fileName)

    var fileId = cInfo.fileid
    info.fileId = String(cCharPointer: &fileId)

    info.fileSize = cInfo.size

    return info
}
