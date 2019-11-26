import Foundation

public class DIDPublicKey: DIDObject {
   public var controller: DID?
   public var keyBase58: String?
    
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
//    public override func isEqual(_ object: Any?) -> Bool {
//    }
    /*
     @Override
     public boolean equals(Object obj) {
         if (obj instanceof PublicKey) {
             PublicKey ref = (PublicKey)obj;

             if (getId().equals(ref.getId()) &&
                     getType().equals(ref.getType()) &&
                     getController().equals(ref.getController()) &&
                     getPublicKeyBase58().equals(ref.getPublicKeyBase58()))
                 return true;
         }

         return false;
     }
     */
    
    class public func fromJson(_ dic: OrderedDictionary<String, Any>, _ ref: DID) throws -> DIDPublicKey{
        
        var value = dic[Constants.controller]
        var controller: DID
        if (value != nil) {
            controller = try DID(dic[Constants.controller] as! String)
        }
        else {
            controller = ref
        }
        var  fra = dic[Constants.id] as! String
        var id: DIDURL
        let preString = fra.prefix(1)
        if preString == "#" {
            fra = String(fra.suffix(fra.count - 1))
            id = try DIDURL(ref, fra)
        }
        else {
            id = try DIDURL(fra)
        }
        
        value = dic[Constants.type]
        var type: String
        if value == nil {
            type = "ECDSAsecp256r1"
        }
        else {
           type = value as! String
        }
        let publicKeyBase58 = dic["publicKeyBase58"] as! String
        return DIDPublicKey(id, type, controller, publicKeyBase58)
    }
    
    public func toJson(_ ref: DID, _ compact: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
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
        if (!compact || (type != Constants.defaultPublicKeyType)) {
            dic[Constants.type] = type
        }
        
        // controller
        if (!compact || !((controller?.isEqual(ref))!)) {
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
