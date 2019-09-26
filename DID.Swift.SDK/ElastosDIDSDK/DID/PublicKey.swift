import Foundation

public class PublicKey: DIDObject {
    var controller: DID?
    var keyBase58: String?
    
    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        super.init(id, type)
    }
    
    class public func fromJson(_ dic: Dictionary<String, Any>, _ ref: DID) throws -> PublicKey{
        let id: DIDURL = try JsonHelper.getDidUrl(dic, Constants.id, ref, "publicKey' id")
        let type = try JsonHelper.getString(dic, Constants.type, true, Constants.defaultPublicKeyType, "publicKey' type")
        let controller: DID = try JsonHelper.getDid(dic, Constants.controller, true, ref, "publicKey' controller")
        let keyBase58: String = try JsonHelper.getString(dic, Constants.publicKeyBase58, false, nil, "publicKeyBase58")
        return PublicKey(id, type, controller, keyBase58)
    }
    
    public func toJson(_ ref: DID, _ compact: Bool) -> Dictionary<String, Any> {
        
        var json: [String: Any] = [:]
        let isCompact = (ref != nil && compact)
        if isCompact && (self.id.did.isEqual(ref)) {
            json["id"] = "#" + id.fragment
        } else {
            json["id"] = id.toExternalForm()
        }
        
        if !isCompact || type != Constants.defaultPublicKeyType {
            json[Constants.type] = type
        }
        
        if !isCompact || !controller!.isEqual(ref) {
            json[Constants.controller] = controller?.toExternalForm()
        }
        json[Constants.publicKeyBase58] = keyBase58
        
        return json
    }
    
}
