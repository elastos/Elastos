import Foundation

public class VerifiableCredential: DIDObject {
    public var types: Array<String> = [String]()
    public var issuer: DID!
    public var issuanceDate: Date!
    public var expirationDate: Date!
    public var subject: CredentialSubject!
    public var proof: Proof!
    private var propf: Proof?

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
    
    public func toJson(_ ref: DID, _ compact: Bool, _ forSign: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        
        // id
        if compact && id.isEqual(ref) {
            value = "#" + id.fragment!
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
    
    class func fromJsonInPath(_ path: String) throws -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        let readHandle = FileHandle(forReadingAtPath: path)
        let data = readHandle?.readDataToEndOfFile()
        
        let json = try JSONSerialization.jsonObject(with: data!, options: [])
        let dic = json as! Dictionary<String, Any>
        try vc.parse(dic, nil)
        return vc
    }
    
    class func fromJson(_ json: Dictionary<String, Any>, _ ref: DID) throws -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        try vc.parse(json, ref)
        return vc
    }

    class func fromJson(_ json: Dictionary<String, Any>) throws -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        try vc.parse(json, nil)
        return vc
    }
    
    func parse(_ json: Dictionary<String, Any>, _ ref: DID?) throws {
        // id
        let id: DIDURL = try JsonHelper.getDidUrl(json, Constants.id, ref!, "crendential id")
        self.id = id
        
        // type
        var value = json[Constants.type]
        guard (value is Array<Any>) else {
            throw DIDError.failue("Invalid type, should be an array.")
        }
        
        let arr = value as! Array<Any>
        guard arr.count != 0 else {
            throw DIDError.failue("Invalid type, should be an array.")
        }
        
        arr.forEach { obj in
            let t: String = obj as! String
            if !t.isEmpty { types.append(t) }
        }
        
        // issuer
        issuer = try JsonHelper.getDid(json, Constants.issuer, true, ref, "crendential issuer")
        
        // issuanceDate
        issuanceDate = try DateFormater.getDate(json, Constants.issuanceDate, false, nil, "credential issuanceDate")
        
        // expirationDate
        expirationDate = try DateFormater.getDate(json, Constants.expirationDate, true, nil, "credential expirationDate")
        
        // credentialSubject
        value = json[Constants.credentialSubject]
        subject = try CredentialSubject.fromJson(json, ref)
        
        // IMPORTANT: help resolve full method in proof
        var re: DID
        if ref == nil {
            re = issuer
        }
        else {
            re = ref!
        }
        
        // proof
        value = json[Constants.proof] 
        proof = try Proof.fromJson(value as! Dictionary<String, Any>, re)
        self.type = proof.type
    }
    
    public func toJsonForSign(_ compact: Bool) -> String {

        return ""
    }
}
