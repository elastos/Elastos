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

public class DIDDocumentProof {
    private var _type: String
    private var _createdDate: Date
    private var _creator: DIDURL
    private var _signature: String
    
    init(_ type: String, _ createdDate: Date, _ creator: DIDURL, _ signature: String) {
        self._type = type
        self._createdDate = createdDate
        self._creator = creator
        self._signature = signature
    }
    
    convenience init(_ creator: DIDURL, _ signature: String) {
        self.init(Constants.DEFAULT_PUBLICKEY_TYPE, DateHelper.currentDate(), creator, signature)
    }

    /// The default type is ECDSAsecp256r1, which can be omitted.
    public var type: String {
        return self._type
    }

    /// The signature creation time can be omitted.
    public var createdDate: Date {
        return self._createdDate
    }

    /// Key reference to verify the signature,
    /// the value must be a reference to the key corresponding to the DID topic,
    /// can be omitted
    public var creator: DIDURL {
        return self._creator
    }

    /// The signed value, using Base64 encoding
    public var signature: String {
        return self._signature
    }

    class func fromJson(_ node: JsonNode, _ refSginKey: DIDURL) throws -> DIDDocumentProof {
        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("document proof type")
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withOptional()
                                .withHint("document proof type")
        let created = try serializer.getDate(Constants.CREATED, options)

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(refSginKey.did)
                                .withHint("document proof creator")
        var creator = try serializer.getDIDURL(Constants.CREATOR, options)
        if  creator == nil {
            creator = refSginKey
        }

        options = JsonSerializer.Options()
                                .withHint("document proof signature")
        let signature = try serializer.getString(Constants.SIGNATURE_VALUE, options)

        return DIDDocumentProof(type, created, creator!, signature)
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) {
        generator.writeStartObject()

        // type
        if normalized || self.type != Constants.DEFAULT_PUBLICKEY_TYPE {
            generator.writeFieldName(Constants.TYPE)
            generator.writeString(self._type)
        }

        // createdDate
        generator.writeFieldName(Constants.CREATED)
        generator.writeString(DateHelper.formateDate(self.createdDate))

        // creator
        if normalized {
            generator.writeFieldName(Constants.CREATOR)
            generator.writeString(self.creator.toString())
        }

        // signature
        generator.writeFieldName(Constants.SIGNATURE_VALUE)
        generator.writeString(self.signature)

        generator.writeEndObject()
    }
}
