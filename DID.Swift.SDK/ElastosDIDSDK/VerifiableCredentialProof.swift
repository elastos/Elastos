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

public class VerifiableCredentialProof {
    private var _type: String
    private var _verificationMethod: DIDURL
    private var _signature: String
    
    init(_ type: String, _ method: DIDURL, _ signature: String) {
        self._type = type
        self._verificationMethod = method
        self._signature = signature
    }

    /// The cryptographic signature suite that was used to generate the signature
    public var type: String {
        return _type
    }

    /// The public key identifier that created the signature
    public var verificationMethod: DIDURL {
        return _verificationMethod
    }

    /// The signed value, using Base64 encoding
    public var signature: String {
        return _signature
    }
    
    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> VerifiableCredentialProof {
        let error = { (des) -> DIDError in
            return DIDError.malformedCredential(des)
        }

        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("credential proof type")
                                .withError(error)
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withRef(ref)
                                .withHint("credential proof verificationMethod")
                                .withError(error)
        let method = try serializer.getDIDURL(Constants.VERIFICATION_METHOD, options)

        options = JsonSerializer.Options()
                                .withHint("credential proof signature")
                                .withError(error)
        let signature = try serializer.getString(Constants.SIGNATURE, options)

        return VerifiableCredentialProof(type, method!, signature)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) {
        generator.writeStartObject()
        if normalized || type != Constants.DEFAULT_PUBLICKEY_TYPE {
            generator.writeStringField(Constants.TYPE, type)
        }

        generator.writeFieldName(Constants.VERIFICATION_METHOD)
        generator.writeString(IDGetter(verificationMethod, ref).value(normalized))

        generator.writeStringField(Constants.SIGNATURE, signature)
        generator.writeEndObject()
    }
}
