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

    public var type: String {
        return self._type
    }

    public var verificationMethod: DIDURL {
        return self._verificationMethod
    }

    public var signature: String {
        return self._signature
    }
    
    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> VerifiableCredentialProof {
        let errorGenerator = { (desc: String) -> DIDError in
            return DIDError.malformedDocument(desc)
        }
        let type = try JsonHelper.getString(node, Constants.TYPE, true,
                                            Constants.DEFAULT_PUBLICKEY_TYPE, "credential proof type",
                                            errorGenerator)
        let method = try JsonHelper.getDidUrl(node, Constants.VERIFICATION_METHOD, ref,
                                            "credential proof verficationMethod",
                                            errorGenerator)
        let signature = try JsonHelper.getString(node, Constants.SIGNATURE, false, nil,
                                            "credential proof signature", errorGenerator)

        return VerifiableCredentialProof(type!, method!, signature!)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID? = nil, _ normalized: Bool) throws {
        try generator.writeStartObject()
        // type
        if normalized || self.type != Constants.DEFAULT_PUBLICKEY_TYPE {
            try generator.writeStringField(Constants.TYPE, self.type)
        }

        let value: String
        if normalized || ref != nil || ref != self.verificationMethod.did {
            value = verificationMethod.toString()
        } else {
            value = "#" + verificationMethod.fragment
        }

        try generator.writeStringField(Constants.VERIFICATION_METHOD, value)
        try generator.writeStringField(Constants.SIGNATURE, self.signature)
        try generator.writeEndObject()
    }
}
