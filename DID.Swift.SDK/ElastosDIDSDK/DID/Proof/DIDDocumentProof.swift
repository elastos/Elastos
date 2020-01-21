
import UIKit

public class DIDDocumentProof: Proof {
    public var created: Date?
    public var creator: DIDURL?
    
    init(_ type: String, _ created: Date, _ creator: DIDURL, _ signature: String) {
        super.init()
        self.type = type
        self.created = created
        self.creator = creator
        self.signature = signature
    }
    
    init(_ creator: DIDURL, _ signature: String) {
        super.init()
        self.type = DEFAULT_PUBLICKEY_TYPE
        self.created = DateFormater.currentDate()
        self.creator = creator
        self.signature = signature
    }
    
    class func fromJson(_ json: Dictionary<String, Any>, _ refSignKey: DIDURL) throws -> DIDDocumentProof {
        let type: String = try JsonHelper.getString(json, TYPE, true, ref: DEFAULT_PUBLICKEY_TYPE, "document proof type")
        
        let created: Date = try JsonHelper.getDate(json, CREATED, true, "")!
        
        let creator = try JsonHelper.getDidUrl(json, CREATOR, true, ref: refSignKey.did, "document proof creator")
        var c = creator
        if creator == nil {
            c = refSignKey
        }
        let signature: String = try JsonHelper.getString(json, SIGNATURE_VALUE, false, "document proof signature")
        
        return DIDDocumentProof(type, created, c!, signature)
    }
    
    func toJson(_ normalized: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        //type:
        if normalized || (type != DEFAULT_PUBLICKEY_TYPE) {
            dic[TYPE] = type
        }
        
        // created
        if created != nil {
            dic[CREATED] = DateFormater.format(created!)
        }
        
        // creator
        if (normalized) {
            dic[CREATOR] = creator!.toExternalForm()
        }
        // signature:
        dic[SIGNATURE_VALUE] = signature
        
        return dic
    }
}
