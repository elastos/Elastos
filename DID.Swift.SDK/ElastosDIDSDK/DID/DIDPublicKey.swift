import Foundation

public class DIDPublicKey: DIDObject {
   public var controller: DID?
   public var keyBase58: String?
    
    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        super.init(id, type)
        self.controller = controller
        self.keyBase58 = keyBase58
    }
    
    class public func fromJson(_ dic: Dictionary<String, Any>, _ ref: DID) throws -> DIDPublicKey{
        
        let controller: DID = try DID(dic["controller"] as! String)
        let id: DIDURL = try DIDURL(dic["id"] as! String)
        let type = dic["type"] as! String
        let publicKeyBase58 = dic["publicKeyBase58"] as! String
        return DIDPublicKey(id, type, controller, publicKeyBase58)
    }
    
    public func toJson(_ ref: DID, _ compact: Bool) -> Dictionary<String, Any> {
        var dic: Dictionary<String, Any> = [: ]
        var value: String
        
        // id
        if compact && id.did.isEqual(ref){
            value = "#" + id.fragment!
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
