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
        return self._type
    }

    public var verificationMethod: DIDURL {
        return self._verificationMethod
    }

    public var realm: String {
        return self._realm
    }

    public var nonce: String {
        return self._nonce
    }

    public var signature: String {
        return self._signature
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> VerifiablePresentationProof {
        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("presentation proof type")
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withRef(ref)
                                .withHint("presentation proof verificationMethod")
        let method = try serializer.getDIDURL(Constants.VERIFICATION_METHOD, options)

        options = JsonSerializer.Options()
                                .withHint("presentation proof realm")
        let realm = try serializer.getString(Constants.REALM, options)

        options = JsonSerializer.Options()
                                .withHint("presentation proof nonce")
        let nonce = try serializer.getString(Constants.NONCE, options)

        options = JsonSerializer.Options()
                                .withHint("presentation proof signature")
        let signature = try serializer.getString(Constants.SIGNATURE, options)

        return VerifiablePresentationProof(type, method!, realm, nonce, signature)
    }

    func toJson(_ generator: JsonGenerator) {
        generator.writeStartObject()
        generator.writeStringField(Constants.TYPE, self.type)
        generator.writeStringField(Constants.VERIFICATION_METHOD, self.verificationMethod.toString())
        generator.writeStringField(Constants.REALM, self.realm)
        generator.writeStringField(Constants.NONCE, self.nonce)
        generator.writeStringField(Constants.SIGNATURE, self.signature)
        generator.writeEndObject()
    }
}
