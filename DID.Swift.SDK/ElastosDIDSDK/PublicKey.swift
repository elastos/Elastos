import Foundation

public class PublicKey: DIDObject {
    private var _controller: DID
    private var _keyBase58: String

    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        self._controller = controller
        self._keyBase58 = keyBase58
        super.init(id, type)
    }

    convenience init(_ id: DIDURL, _ controller: DID, _ keyBase58: String) {
        self.init(id, Constants.DEFAULT_PUBLICKEY_TYPE, controller, keyBase58)
    }

    public var controller: DID {
        return self._controller
    }

    public var publicKeyBase58: String {
        return self._keyBase58
    }

    public var publicKeyBytes: [UInt8] {
        return Base58.bytesFromBase58(self._keyBase58)
    }

    public var publicKeyInData: Data {
        return self._keyBase58.data(using: .utf8)!
    }

    class func fromJson(_ node: Dictionary<String, Any>, _ ref: DID?) throws -> PublicKey {
        let serializer = JsonSerializer(node)
        let id = try serializer.getDIDURL(Constants.ID,
                            JsonSerializer.Options<DIDURL>()
                                .withOptional()
                                .withHint("publicKey id"))

        let type = try serializer.getString(Constants.TYPE,
                            JsonSerializer.Options<String>()
                                .withOptional()
                                .withDefValue(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("publicKey type"))

        let controller = try serializer.getDID(Constants.CONTROLLER,
                            JsonSerializer.Options<DID>()
                                .withOptional()
                                .withDefValue(ref)
                                .withHint("publicKey controller"))

        let keybase58 = try serializer.getString(Constants.PUBLICKEY_BASE58,
                            JsonSerializer.Options<String>()
                                .withHint("pulbicKeyBase58"))

        return PublicKey(id!, type!, controller!, keybase58!)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) throws {
        try generator.writeStartObject()
        try generator.writeFieldName(Constants.ID)
        try generator.writeString(IDGetter(getId(), ref).value(normalized))

        // type
        if normalized || self.getType() != Constants.DEFAULT_PUBLICKEY_TYPE {
            try generator.writeStringField(Constants.TYPE, self.getType())
        }

        // controller
        if normalized || ref == nil || ref != self.controller {
            try generator.writeFieldName(Constants.CONTROLLER);
            try generator.writeString(self.controller.toString())
        }

        // publicKeyBase58
        try generator.writeFieldName(Constants.PUBLICKEY_BASE58)
        try generator.writeString(self.publicKeyBase58)
        try generator.writeEndObject()
    }

    override func equalsTo(_ other: DIDObject) -> Bool {
        guard other is PublicKey else {
            return false
        }

        let publicKey = other as! PublicKey
        return super.equalsTo(other) &&
               self.controller == publicKey.controller &&
               self.publicKeyBase58 == publicKey.publicKeyBase58
    }
}

extension PublicKey {
    public static func == (lhs: PublicKey, rhs: PublicKey) -> Bool {
        return lhs.equalsTo(rhs)
    }

    public static func != (lhs: PublicKey, rhs: PublicKey) -> Bool {
        return !lhs.equalsTo(rhs)
    }
}
