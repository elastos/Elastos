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

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> PublicKey {
        let id: DIDURL?
        let type: String?
        let controller: DID?
        let keyBase58: String?
        let errorGenerator = { (desc: String) -> DIDError in
            return DIDError.malformedDocument(desc)
        }

        id   = try JsonHelper.getDidUrl(node, Constants.ID, ref, "publicKey id",
                                        errorGenerator)

        type = try JsonHelper.getString(node, Constants.TYPE, true,
                                        Constants.DEFAULT_PUBLICKEY_TYPE, "publicKey type",
                                        errorGenerator)

        controller = try JsonHelper.getDid(node, Constants.CONTROLLER, true,
                                        ref, "publicKey controller",
                                        errorGenerator)

        keyBase58  = try JsonHelper.getString(node, Constants.PUBLICKEY_BASE58,
                                        false, nil, "publicKeyBase58",
                                        errorGenerator)

        return PublicKey(id!, type!, controller!, keyBase58!) // TODO:
    }

    private func computeIdValue(_ ref: DID?, _ normalized: Bool) -> String {
        let value: String

        if normalized || ref == nil || ref != getId().did {
            value = getId().toString()
        } else {
            // DIDObject always keeps not empty fragment.
            value = "#" + getId().fragment!
        }
        return value
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) throws {
        try generator.writeStartObject()
        try generator.writeFieldName(Constants.ID)
        try generator.writeString(computeIdValue(ref, normalized))

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
        try generator.writeString(self._keyBase58)
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
