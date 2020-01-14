import Foundation

public class VerifiableCredential: DIDObject {
    public var types: Array<String> = [String]()
    public var issuer: DID!
    public var issuanceDate: Date?
    public var expirationDate: Date?
    public var subject: CredentialSubject!
    public var proof: Proof!
    var alias: String!
    
    private let  RULE_EXPIRE: Int = 1
    private let  RULE_GENUINE: Int = 2
    private let  RULE_VALID: Int = 3
    public var meta: CredentialMeta = CredentialMeta()

    public override init() {
        super.init()
    }
    
    init(_ vc: VerifiableCredential) {
        super.init()
        self.id = vc.id
        self.types = vc.types
        self.issuer = vc.issuer
        self.issuanceDate = vc.issuanceDate
        self.expirationDate = vc.expirationDate
        self.subject = vc.subject
        self.proof = vc.proof
    }
    
    public func setExtra(_ name: String, _ value: String) throws {
        meta.setExtra(name, value)
        if (meta.attachedStore()) {
            try meta.store?.storeCredentialMeta(subject.id, id, meta)
        }
    }
    
    public func getExtra(_ name: String) throws -> String? {
        return meta.getExtra(name)
    }
    
    public func setAlias(_ alias: String) throws {
        meta.alias = alias

        if (meta.attachedStore()) {
            try meta.store?.storeCredentialMeta(subject.id, id, meta)
        }
    }

    public func getAlias() -> String{
        return meta.alias
    }
    
    public func isSelfProclaimed() throws -> Bool {
        return issuer.isEqual(subject.id)
    }
    
    private func traceCheck(_ rule: Int) throws -> Bool {
        let controllerDoc = try subject.id.resolve()
        if controllerDoc == nil {
            return false
        }
        
        switch rule {
        case RULE_EXPIRE: do {
            if controllerDoc!.isExpired() {
                return true
            }
            break
            }
        case RULE_EXPIRE: do {
            if try !controllerDoc!.isGenuine() {
                return false
            }
            break
            }
        default: do {
                if ( try !controllerDoc!.isValid()) {
                    return false
                }
            }
        }
        return rule != RULE_EXPIRE
    }
    
    public func checkExpired() throws -> Bool {
        if (expirationDate != nil) {
            let date = DateFormater.currentDate()
            return DateFormater.comporsDate(expirationDate!, date)
        }

        return false
    }
    
    public func isExpired() throws -> Bool {
        if (try traceCheck(RULE_EXPIRE)){
            return true
        }
        
        return try checkExpired()
    }

    private func checkGenuine() throws -> Bool {
        let issuerDoc = try issuer.resolve()
        
        // Credential should signed by authentication key.
        if (try !issuerDoc!.isAuthenticationKey(proof.verificationMethod)){
            return false
        }
        
        // Unsupported public key type;
        if (proof!.type != DEFAULT_PUBLICKEY_TYPE){
            return false
        }
        let dic = toJson(true, true)
        let json: String = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        
        return try issuerDoc!.verify(proof.verificationMethod, proof.signature, count, inputs)
    }
    
    public func isValid() throws -> Bool {
        if (try !traceCheck(RULE_VALID)) {
            return false
        }

        return try !checkExpired() && checkGenuine()
    }

    public func verify() throws -> Bool {
        let issuerDoc: DIDDocument = try issuer.resolve()!
        let json: String = toJsonForSign(false)
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        
        return try issuerDoc.verify(proof!.verificationMethod, proof!.signature, count, inputs)
    }
    
    public func isGenuine() throws -> Bool {
        if (try !traceCheck(RULE_GENUINE)) {
            return false
        }
        
        return try !checkExpired() && checkGenuine()
    }
    
    public func toJson(_ normalized: Bool, _ forSign: Bool) -> OrderedDictionary<String, Any> {
        return toJson(ref: nil, normalized, forSign)
    }
    
    public func toJson(_ ref: DID, _ normalized: Bool, _ forSign: Bool) -> OrderedDictionary<String, Any> {
        return toJson(ref: ref, normalized, forSign)
    }
    
    public func toJson(_ ref: DID, _ normalized: Bool) -> OrderedDictionary<String, Any> {
       return toJson(ref, normalized, false)
    }
    
    public func toJson(_ normalized: Bool) -> OrderedDictionary<String, Any> {
        return toJson(ref: nil, normalized, false)
    }
    
    private func toJson(ref: DID?, _ normalized: Bool, _ forSign: Bool) -> OrderedDictionary<String, Any> {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        var value: String
        
        // id
        if normalized || ref == nil || id.did != ref {
            value = id.toExternalForm()
        }
        else {
            value = "#" + id.fragment!
        }
        dic[ID] = value
        
        // type
        var strs: Array<String> = []
        types.sort { (a, b) -> Bool in
            return a.compare(b) == ComparisonResult.orderedAscending
        }
        types.forEach{ str in
            strs.append(str)
        }
        dic[TYPE] = strs
        
        // issuer
        if normalized || issuer != subject.id {
            dic[ISSUER] = issuer.description
        }
        
        // issuanceDate
        if (issuanceDate != nil) {
            dic[ISSUANCE_DATE] = DateFormater.format(issuanceDate!)
        }
        
        // expirationDate
        if (expirationDate != nil) {
            dic[EXPIRATION_DATE] = DateFormater.format(expirationDate!)
        }
        
        let credSubject = subject.toJson(ref, normalized)
        let orderCredSubject = DIDURLComparator.DIDOrderedDictionaryComparatorByKey(credSubject)

        // credentialSubject
        dic[CREDENTIAL_SUBJECT] = orderCredSubject
        
        // proof
        if !forSign {
            dic[PROOF] = proof.toJson_vc(issuer, normalized)
        }
        return dic
    }

   public class func fromJsonInPath(_ path: String) throws -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        let url = URL(fileURLWithPath: path)
        let json = try! String(contentsOf: url)
        let string = JsonHelper.preHandleString(json)
        let ordDic = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        try vc.parse(ordDic, nil)
        return vc
    }
    
   public class func fromJson(_ json: String) throws -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        let string = JsonHelper.preHandleString(json)
        let ordDic = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        try vc.parse(ordDic, nil)
        return vc
    }
    
   public class func fromJson(_ json: OrderedDictionary<String, Any>, _ ref: DID) throws -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        try vc.parse(json, ref)
        return vc
    }

   public class func fromJson(_ json: OrderedDictionary<String, Any>) throws -> VerifiableCredential {
        let vc: VerifiableCredential = VerifiableCredential()
        try vc.parse(json, nil)
        return vc
    }
    
    func parse(_ json: OrderedDictionary<String, Any>, _ ref: DID?) throws {
        // id
        let id: DIDURL = try JsonHelper.getDidUrl(json, ID, ref, "crendential id")!
        self.id = id
        
        // type
        var value = json[TYPE]
        guard (value is Array<Any>) else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid type, should be an array.")
        }
        
        let arr = value as! Array<Any>
        guard arr.count != 0 else {
            throw DIDError.malformedCredentialError(_desc: "Invalid type, should be an array.")
        }
        
        arr.forEach { obj in
            let t: String = obj as! String
            if !t.isEmpty { types.append(t) }
        }
        
        // issuer
        issuer = try JsonHelper.getDid(json, ISSUER, true, ref, "crendential issuer")
        
        // issuanceDate
        issuanceDate = try DateFormater.getDate(json, ISSUANCE_DATE, false, nil, "credential issuanceDate")
        
        // expirationDate
        expirationDate = try DateFormater.getDate(json, EXPIRATION_DATE, true, nil, "credential expirationDate")
        
        // credentialSubject
        value = json[CREDENTIAL_SUBJECT]
        subject = try CredentialSubject.fromJson(value as! OrderedDictionary<String, Any>, ref)
        
        // IMPORTANT: help resolve full method in proof
        if (issuer == nil) {
            issuer = subject.id
        }
        
        // proof
        value = json[PROOF]
        proof = try Proof.fromJson_vc(value as! OrderedDictionary<String, Any>, issuer)
        self.type = proof.type
    }
    
    public func toJsonForSign(_ normalized: Bool) -> String {
        let dic = toJson(normalized, true)
        let json = JsonHelper.creatJsonString(dic: dic)
        return json
    }
    
    public func description(_ normalized: Bool) -> String {
        let dic = toJson(normalized, false)
        let json = JsonHelper.creatJsonString(dic: dic)
        return json
    }
    
    public override var description: String{
        return description(false)
    }
}
