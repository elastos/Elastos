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

import Foundation

class IDTransactionInfo: DIDTransaction {
    private var _transactionId: String
    private var _timestamp: Date
    private var _request: IDChainRequest
    
    init(_ transactionId: String, _ timestamp: Date, _ request: IDChainRequest) {
        self._transactionId = transactionId;
        self._timestamp = timestamp;
        self._request = request;
    }

    /// Get did.
    /// - Returns: The handle of did.
    public func getDid() -> DID {
        return request.did!
    }

    /// Get transaction id.
    /// - Returns: The handle of transaction id.
    public func getTransactionId() -> String {
        return self.transactionId
    }

    /// Get time stamp.
    /// - Returns: The handle time stamp.
    public func getTimestamp() -> Date {
        return self.timestamp
    }

    /// Get IDChain request operation.
    /// - Returns: The handle of operation.
    public func getOperation() -> IDChainRequestOperation {
        return request.operation
    }

    /// Get DID Document.
    /// - Returns: The handle of DID Document.
    public func getDocument() -> DIDDocument {
        return request.document!
    }

    var transactionId: String {
        return self._transactionId
    }

    var timestamp: Date {
        return self._timestamp
    }

    var did: DID? {
        return self._request.did
    }
    
    var operation: IDChainRequestOperation {
        return self._request.operation
    }

    var payload: String {
        return self._request.toJson(false)
    }

    var request: IDChainRequest {
        return self._request
    }

    class func fromJson(_ node: JsonNode) throws -> IDTransactionInfo {
        let error = { (des: String) -> DIDError in
            return DIDError.didResolveError(des)
        }

        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withHint("transaction id")
                                .withError(error)
        let transactionId = try serializer.getString(Constants.TXID, options)

        options = JsonSerializer.Options()
                                .withHint("transaction timestamp")
                                .withError(error)
        let timestamp = try serializer.getDate(Constants.TIMESTAMP, options)

        let subNode = node.get(forKey: Constants.OPERATION)
        guard let _ = subNode else {
            throw DIDError.didResolveError("missing ID operation")
        }

        let request = try IDChainRequest.fromJson(subNode!)
        return IDTransactionInfo(transactionId, timestamp, request)
    }

    func toJson(_ generator: JsonGenerator) {
        generator.writeStartObject()
        generator.writeStringField(Constants.TXID, self.transactionId)
        generator.writeStringField(Constants.TIMESTAMP, self.timestamp.description)

        generator.writeFieldName(Constants.OPERATION)
        self._request.toJson(generator, false)
        generator.writeEndObject()
    }
}
