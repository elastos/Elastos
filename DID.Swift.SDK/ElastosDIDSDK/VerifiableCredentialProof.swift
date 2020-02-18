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
        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("credential proof type")
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withRef(ref)
                                .withHint("credential proof verificationMethod")
        let method = try serializer.getDIDURL(Constants.VERIFICATION_METHOD, options)

        options = JsonSerializer.Options()
                                .withHint("credential proof signature")
        let signature = try serializer.getString(Constants.SIGNATURE, options)

        return VerifiableCredentialProof(type, method!, signature)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) {
        generator.writeStartObject()
        if normalized || self.type != Constants.DEFAULT_PUBLICKEY_TYPE {
            generator.writeStringField(Constants.TYPE, self.type)
        }

        generator.writeStringField(Constants.VERIFICATION_METHOD, IDGetter(verificationMethod, ref).value(normalized))
        generator.writeStringField(Constants.SIGNATURE, self.signature)
        generator.writeEndObject()
    }
}
