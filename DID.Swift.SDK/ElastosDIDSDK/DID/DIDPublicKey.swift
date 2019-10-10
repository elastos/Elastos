import Foundation

public class DIDPublicKey: DIDObject {
    var controller: DID?
    var keyBase58: String?
    
    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        super.init(id, type)
    }
    
    class public func fromJson(_ dic: Dictionary<String, Any>, _ ref: DID) throws -> DIDPublicKey{
        let id: DIDURL = try JsonHelper.getDidUrl(dic, Constants.id, ref, "publicKey' id")
        let type = try JsonHelper.getString(dic, Constants.type, true, Constants.defaultPublicKeyType, "publicKey' type")
        let controller: DID = try JsonHelper.getDid(dic, Constants.controller, true, ref, "publicKey' controller")
        let keyBase58: String = try JsonHelper.getString(dic, Constants.publicKeyBase58, false, nil, "publicKeyBase58")
        return DIDPublicKey(id, type, controller, keyBase58)
    }
    
    public func toJson(_ ref: DID, _ compact: Bool) -> Dictionary<String, Any> {
        var dic: Dictionary<String, Any> = [: ]
        var value: String
        
        // id
        if compact && id.did.isEqual(ref){
            value = "#" + id.fragment
        }
        else {
            value = id.toExternalForm()
        }
        dic[Constants.id] = value
        
        // type
        if !compact && (type != Constants.defaultPublicKeyType) {
            dic[Constants.type] = type
        }
        
        // controller
        if !compact && !((controller?.isEqual(ref))!) {
            dic[Constants.controller] = controller?.toExternalForm()
        }
        
        // publicKeyBase58
        dic[Constants.publicKeyBase58] = keyBase58
        return dic
    }
    
    public func getPublicKeyBytes() -> [UInt8]{
        return Base58.bytesFromBase58(keyBase58!)
    }
    
}
