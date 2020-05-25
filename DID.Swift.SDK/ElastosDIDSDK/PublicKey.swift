import Foundation

public class PublicKey: DIDObject {
    private var _controller: DID
    private var _keyBase58: String

    private var authenticationKey: Bool
    private var authorizationKey: Bool

    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        self._controller = controller
        self._keyBase58 = keyBase58

        self.authenticationKey = false
        self.authorizationKey = false

        super.init(id, type)
    }

    convenience init(_ id: DIDURL, _ controller: DID, _ keyBase58: String) {
        self.init(id, Constants.DEFAULT_PUBLICKEY_TYPE, controller, keyBase58)
    }

    public var controller: DID {
        return _controller
    }

    public var publicKeyBase58: String {
        return _keyBase58
    }

    public var publicKeyBytes: [UInt8] {
        return Base58.bytesFromBase58(_keyBase58)
    }

//    public var publicKeyData: Data {
//        return _keyBase58.data(using: .utf8)!
//    }

    public var isAuthenticationKey: Bool {
        return authenticationKey
    }

    func setAuthenticationKey(_ newValue: Bool) {
        self.authenticationKey = newValue
    }

    public var isAthorizationKey: Bool {
        return authorizationKey
    }

    func setAthorizationKey(_ newValue: Bool) {
        self.authorizationKey = newValue
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> PublicKey {
        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withRef(ref)
                                .withHint("publicKey id")
        let id = try serializer.getDIDURL(Constants.ID, options)

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("publicKey type")
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(ref)
                                .withHint("publicKey controller")
        let controller = try serializer.getDID(Constants.CONTROLLER, options)

        options = JsonSerializer.Options()
                                .withHint("publicKeyBase58")
        let keybase58 = try serializer.getString(Constants.PUBLICKEY_BASE58, options)

        return PublicKey(id!, type, controller, keybase58)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) {
        generator.writeStartObject()
        generator.writeFieldName(Constants.ID)
        generator.writeString(IDGetter(getId(), ref).value(normalized))

        // type
        if normalized || !isDefType() {
            generator.writeStringField(Constants.TYPE, getType())
        }

        // controller
        if normalized || ref == nil || ref != controller {
            generator.writeFieldName(Constants.CONTROLLER);
            generator.writeString(controller.toString())
        }

        // publicKeyBase58
        generator.writeFieldName(Constants.PUBLICKEY_BASE58)
        generator.writeString(publicKeyBase58)
        generator.writeEndObject()
    }

    override func equalsTo(_ other: DIDObject) -> Bool {
        guard other is PublicKey else {
            return false
        }

        let publicKey = other as! PublicKey
        return super.equalsTo(other) &&
               controller == publicKey.controller &&
               publicKeyBase58 == publicKey.publicKeyBase58
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
