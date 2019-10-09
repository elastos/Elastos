import Foundation

public class VerifiableCredential: DIDObject {
    public var types: Array<String>!
    public var issuer: DID!
    public var issuanceDate: Date!
    public var expirationDate: Date!
    public var subject: CredentialSubject!
    public var proof: Proof!
    
    override init() {
        super.init()
    }
    
    init(_ vc: VerifiableCredential) {
        super.init(vc.id, vc.type)
        self.id = vc.id
        self.types = vc.types
        self.issuer = vc.issuer
        self.issuanceDate = vc.issuanceDate
        self.expirationDate = vc.expirationDate
        self.subject = vc.subject
        self.proof = vc.proof
    }
    
    public func toJson(_ ref: DID, _ compact: Bool, _ forSign: Bool) -> Dictionary<String, Any> {
        var dic: Dictionary<String, Any> = [: ]
        var value: String
        
        // id
        if compact && id.isEqual(ref) {
            value = "#" + id.fragment
        }
        else {
            value = id.toExternalForm()
        }
        dic[Constants.id] = value
        
        // type
        var strs: Array<String> = []
        types.forEach{ str in
            strs.append(str)
        }
        dic[Constants.type] = strs
        
        // issuer
        if !compact && !(issuer.isEqual(subject.id)) {
            dic[Constants.issuer] = issuer.toExternalForm()
        }
        
        // issuanceDate
        if (expirationDate != nil) {
            dic[Constants.expirationDate] = "TODO: change to time string"
        }
        
        // credentialSubject
        dic[Constants.credentialSubject] = subject.toJson(ref, compact)
        
        // proof
        if !forSign {
            dic[Constants.proof] = proof.toJson(ref, compact)
        }
        return dic
    }
    
    class func fromJson(_ json: String) -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        
        return vc
    }

    class func fromJson(_ md: Any, _ ref: DID) -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        return vc
    }

    class func fromJson(_ md: Any) -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        return vc
    }
    
    func parse(_ md: Dictionary<String, Any>, _ ref: DID?) throws {
        // id
        let id: DIDURL = try JsonHelper.getDidUrl(md, Constants.id, ref!, "crendential id")
        self.id = id
        
        // type
        var value = md[Constants.type]
        guard !(value is Array<Any>) else {
            // throws error
            return
        }
        
        let arr = value as! Array<Any>
        guard arr.count != 0 else {
            // throws error
            return
        }
        
        arr.forEach { obj in
            let t: String = obj as! String
            if !t.isEmpty { types.append(t) }
        }
        
        // issuer
        issuer = try JsonHelper.getDid(md, Constants.issuer, true, ref, "crendential issuer")
        
        // issuanceDate
        issuanceDate = JsonHelper.getDate(md, Constants.issuanceDate, false, nil, "credential issuanceDate")
        
        // expirationDate
        expirationDate = JsonHelper.getDate(md, Constants.expirationDate, true, nil, "credential expirationDate")
        
        // credentialSubject
        value = md[Constants.credentialSubject]
        subject = try CredentialSubject.fromJson(md, ref)
        
        // IMPORTANT: help resolve full method in proof
        var re: DID
        if ref == nil {
            re = issuer
        }
        else {
            re = ref!
        }
        
        // proof
        value = md[Constants.proof]
        proof = try Proof.fromJson(md, re)
    }
}
