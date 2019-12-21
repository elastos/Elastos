import Foundation

public class DIDPublicKey: DIDObject {
    public var controller: DID!
    public var keyBase58: String!

    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        super.init(id, type)
        self.controller = controller
        self.keyBase58 = keyBase58
    }

    init(_ id: DIDURL, _ controller: DID, _ keyBase58: String) {
        super.init(id, Constants.defaultPublicKeyType)
        self.controller = controller
        self.keyBase58 = keyBase58
    }

    public override func isEqual(_ object: Any?) -> Bool {
        guard object is DIDPublicKey else {
            return false
        }

        let ref = object as! DIDPublicKey
        return self.id == ref.id
            && self.type == ref.type
            && self.controller == ref.controller
            && self.keyBase58 == ref.keyBase58
    }

    class public func fromJson(_ dict: OrderedDictionary<String, Any>, _ ref: DID) throws -> DIDPublicKey {
        var value = dict[Constants.controller]
        var controller: DID
        
        if let _ = value {
            controller = try DID(value as! String)
        } else {
            controller = ref
        }
        
        var frag = dict[Constants.id] as! String
        var id: DIDURL
        let hashStr = frag.prefix(1)

        if hashStr == "#" {
            frag = String(frag.suffix(frag.count - 1))
            id = try DIDURL(ref, frag)
        } else {
            id = try DIDURL(frag)
        }
        
        value = dict[Constants.type]
        var type: String

        if let _ = value {
            type = value as! String
        } else {
            type = Constants.defaultPublicKeyType
        }

        let publicKeyBase58 = dict[Constants.publicKeyBase58] as! String
        return DIDPublicKey(id, type, controller, publicKeyBase58)
    }
    
    class public func fromJson_dc(_ dic: OrderedDictionary<String, Any>, _ ref: DID) throws -> DIDPublicKey {
        let id = try JsonHelper.getDidUrl(dic, Constants.id,
        ref, "publicKey' id")
        let type = try JsonHelper.getString(dic, Constants.type, true,
        Constants.defaultPublicKeyType, "publicKey' type")
        let controller = try JsonHelper.getDid(dic, Constants.controller,
        true, ref, "publicKey' controller")
        let keyBase58 = try JsonHelper.getString(dic, Constants.publicKeyBase58,
        false, nil, "publicKeyBase58")
            
        return DIDPublicKey(id!, type, controller!, keyBase58)
    }
    
    public func toJson_dc(_ ref: DID, _ normalized: Bool) -> OrderedDictionary<String, Any> {
        var dict: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        
        // id
        if normalized || id.did != ref {
            value = id.toExternalForm()
        } else {
            value = "#" + id.fragment!
        }
        dict[Constants.id] = value
        
        // type
        if normalized || type != Constants.defaultPublicKeyType {
            dict[Constants.type] = type
        }
        
        // controller
        if normalized || controller != ref {
            dict[Constants.controller] = controller.toExternalForm()
        }
        
        // publicKeyBase58
        dict[Constants.publicKeyBase58] = keyBase58
        
        return dict
    }
    
    public func getPublicKeyBytes() -> [UInt8]{
        return Base58.bytesFromBase58(keyBase58)
    }
}
