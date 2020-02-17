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
    
    class func fromJson(_ node: Dictionary<String, Any>, _ ref: DID?) throws
                        -> VerifiableCredentialProof {
        let serializer = JsonSerializer(node)
        let proofType = try serializer.getString(Constants.TYPE,
                                JsonSerializer.Options<String>()
                                    .withOptional()
                                    .withDefValue(Constants.DEFAULT_PUBLICKEY_TYPE)
                                    .withHint("credential proof type"))
        let method    = try serializer.getDIDURL(Constants.VERIFICATION_METHOD,
                                JsonSerializer.Options<DIDURL>()
                                    .withHint("credential proof verificationMethod"))
        let signature = try serializer.getString(Constants.SIGNATURE,
                                JsonSerializer.Options<String>()
                                    .withHint("credential proof signature"))

        return VerifiableCredentialProof(proofType!, method!, signature!)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) throws {
        try generator.writeStartObject()
        if normalized || self.type != Constants.DEFAULT_PUBLICKEY_TYPE {
            try generator.writeStringField(Constants.TYPE, self.type)
        }

        try generator.writeStringField(Constants.VERIFICATION_METHOD, IDGetter(verificationMethod, ref).value(normalized))
        try generator.writeStringField(Constants.SIGNATURE, self.signature)
        try generator.writeEndObject()
    }
}
