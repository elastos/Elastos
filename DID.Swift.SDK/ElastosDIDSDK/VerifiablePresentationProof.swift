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

    class func fromJson(_ node: Dictionary<String, Any>, _ ref: DID?) throws -> VerifiablePresentationProof {
        let serializer = JsonSerializer(node)

        let type = try serializer.getString(Constants.TYPE,
                            JsonSerializer.Options<String>()
                                .withOptional()
                                .withDefValue(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("presentation proof type"))
        let method = try serializer.getDIDURL(Constants.VERIFICATION_METHOD,
                            JsonSerializer.Options<DIDURL>()
                                .withHint("presentation proof verificationMethod"))
        let realm = try serializer.getString(Constants.REALM,
                            JsonSerializer.Options<String>()
                                .withHint("presentation proof realm"))
        let nonce = try serializer.getString(Constants.NONCE,
                            JsonSerializer.Options<String>()
                                .withHint("presentation proof nonce"))
        let signature = try serializer.getString(Constants.SIGNATURE,
                            JsonSerializer.Options<String>()
                                .withHint("presentation proof signature"))

        return VerifiablePresentationProof(type!, method!, realm!, nonce!, signature!)
    }

    func toJson(_ generator: JsonGenerator) throws {
        try generator.writeStartObject()
        try generator.writeStringField(Constants.TYPE, self.type)
        try generator.writeStringField(Constants.VERIFICATION_METHOD, self.verificationMethod.toString())
        try generator.writeStringField(Constants.REALM, self.realm)
        try generator.writeStringField(Constants.NONCE, self.nonce)
        try generator.writeStringField(Constants.SIGNATURE, self.signature)
        try generator.writeEndObject()
    }
}
