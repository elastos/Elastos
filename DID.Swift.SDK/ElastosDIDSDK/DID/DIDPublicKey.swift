import Foundation

public class DIDPublicKey: DIDObject {
    private var _controller: DID
    private var _keyBase58: String

    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        self._controller = controller
        self._keyBase58 = keyBase58
        super.init(id, type)
    }

    init(_ id: DIDURL, _ controller: DID, _ keyBase58: String) {
        self._controller = controller
        self._keyBase58 = keyBase58
        super.init(id, DEFAULT_PUBLICKEY_TYPE)
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

    public override func isEqual(_ object: Any?) -> Bool {
        guard object is DIDPublicKey else {
            return false
        }

        let ref = object as! DIDPublicKey
        return self.id == ref.id
            && self.type == ref.type
            && self.controller == ref.controller
            && self.publicKeyBase58 == ref.publicKeyBase58
    }

    public override var description: String {
        let dic = toJson(_controller, false)
        let json = JsonHelper.creatJsonString(dic: dic)
        return json
    }

    class public func fromJson(_ dic: OrderedDictionary<String, Any>, _ ref: DID) throws -> DIDPublicKey {
        // id
        let id = try JsonHelper.getDidUrl(dic, "id", ref, "publicKey' id")
        
        // type
        let type = try JsonHelper.getString(dic, TYPE, true, DEFAULT_PUBLICKEY_TYPE, "publicKey' type")
        
        // controller
        let controller = try JsonHelper.getDid(dic, CONTROLLER, true, ref, "publicKey' controller")
        
        // publicKeyBase58
        let keyBase58 = try JsonHelper.getString(dic, PUBLICKEY_BASE58, false, nil, "publicKeyBase58")
            
        return DIDPublicKey(id!, type, controller!, keyBase58)
    }

    public func toJson(_ ref: DID, _ normalized: Bool) -> OrderedDictionary<String, Any> {
        var dict: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        
        // id
        if normalized || id.did != ref {
            value = id.toExternalForm()
        } else {
            value = "#" + id.fragment!
        }
        dict[ID] = value
        
        // type
        if normalized || type != DEFAULT_PUBLICKEY_TYPE {
            dict[TYPE] = type
        }
        
        // controller
        if normalized || controller != ref {
            dict[CONTROLLER] = controller.description
        }
        
        // publicKeyBase58
        dict[PUBLICKEY_BASE58] = _keyBase58
        
        return dict
    }
}
