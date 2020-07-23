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

public class VerifiablePresentationProof {
    private let _type: String
    private let _verificationMethod: DIDURL
    private let _realm: String
    private let _nonce: String
    private let _signature: String
    
    init(_ type: String,  _ method: DIDURL, _ realm: String,  _ nonce: String, _ signature: String) {
        self._type = type
        self._verificationMethod = method
        self._realm = realm
        self._nonce = nonce
        self._signature = signature
    }
    
    convenience init(_ method: DIDURL, _ realm: String, _ nonce: String, _ signature: String) {
        self.init(Constants.DEFAULT_PUBLICKEY_TYPE, method, realm, nonce, signature)
    }

    public var type: String {
        return _type
    }

    public var verificationMethod: DIDURL {
        return _verificationMethod
    }

    public var realm: String {
        return _realm
    }

    public var nonce: String {
        return _nonce
    }

    public var signature: String {
        return _signature
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> VerifiablePresentationProof {
        let error = { (des) -> DIDError in
            return DIDError.malformedPresentation(des)
        }

        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withError(error)
                                .withHint("presentation proof type")
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withRef(ref)
                                .withError(error)
                                .withHint("presentation proof verificationMethod")
        let method = try serializer.getDIDURL(Constants.VERIFICATION_METHOD, options)

        options = JsonSerializer.Options()
                                .withError(error)
                                .withHint("presentation proof realm")
        let realm = try serializer.getString(Constants.REALM, options)

        options = JsonSerializer.Options()
                                .withError(error)
                                .withHint("presentation proof nonce")
        let nonce = try serializer.getString(Constants.NONCE, options)

        options = JsonSerializer.Options()
                                .withError(error)
                                .withHint("presentation proof signature")
        let signature = try serializer.getString(Constants.SIGNATURE, options)

        return VerifiablePresentationProof(type, method!, realm, nonce, signature)
    }

    func toJson(_ generator: JsonGenerator) {
        generator.writeStartObject()
        generator.writeStringField(Constants.TYPE, type)
        generator.writeStringField(Constants.VERIFICATION_METHOD, verificationMethod.toString())
        generator.writeStringField(Constants.REALM, realm)
        generator.writeStringField(Constants.NONCE, nonce)
        generator.writeStringField(Constants.SIGNATURE, signature)
        generator.writeEndObject()
    }
}
