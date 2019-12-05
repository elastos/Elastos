
import Foundation

public class VerifiablePresentation: NSObject {
   public var type: String!
   public var created: Date!
   public var credentials: OrderedDictionary<DIDURL, VerifiableCredential>!
   public var proof: Proof?
    
    override init() {
        type = Constants.defaultPresentationType
        created = DateFormater.currentDate()
        credentials = OrderedDictionary()
    }
    
    public func getCredentials() -> Array<VerifiableCredential> {
        return credentials!.values
    }
    
    public func addCredential(_ credential: VerifiableCredential) {
        credentials![credential.id] = credential
    }
    
    public func getCredential(_ id: DIDURL) -> VerifiableCredential {
        return credentials![id]!
    }
    
    public func getSigner() -> DID {
        return (proof?.verificationMethod.did)!
    }
    
    public func verify() throws -> Bool {
        let signer: DID = getSigner()
        
        var b: Bool = true
        try credentials.values.forEach { vc  in
            guard vc.subject.id == signer else {
                b = false
               return
            }
            guard try vc.verify() || !vc.isExpired() else {
                b = false
                return
            }
        }
        
        guard b else {
            return false
        }
        let json: String = toJsonForSign()
        let signerDoc: DIDDocument = try signer.resolve()!
        
        let inputs: [CVarArg] = [json, json.count]
        let count: Int = inputs.count / 2
    
        return try signerDoc.verify(proof!.verificationMethod, proof!.signature, count, inputs)
     }
    
    public class func fromJson(_ jsonString: String) throws -> VerifiablePresentation {
        let vp: VerifiablePresentation = VerifiablePresentation()
        try vp.parse(jsonString)
        return vp
    }
    
    func parse(_ jsonString: String) throws {
        let dic = JsonHelper.handleString(jsonString) as! OrderedDictionary<String, Any>
        try parse(dic)
    }
    
    func parse(_ presentation: OrderedDictionary<String, Any>) throws {
        
        let type: String = try JsonHelper.getString(presentation, Constants.type, false, nil, "presentation type")
        guard type == Constants.defaultPresentationType else {
            throw MalformedCredentialError.failue("Unknown presentation type: \(type)")
        }
        self.type = type
        let created: Date = try DateFormater.getDate(presentation, Constants.created, false, nil, "presentation created date")!
        self.created = created
        
        var d = presentation[Constants.verifiableCredential]
        guard d != nil else {
            throw MalformedCredentialError.failue("Missing credentials.")
        }
        guard d is Array<Any> else {
            throw MalformedCredentialError.failue("Invalid verifiableCredentia, should be an array.")
        }
        try parseCredential(d as! Array<OrderedDictionary<String, Any>>)
        
        d = presentation[Constants.proof]
        guard d != nil else {
           throw MalformedCredentialError.failue("Missing credentials.")
        }
        let proof: Proof = try Proof.fromJson_vp(presentation, nil)
        self.proof = proof
    }
    
    func parseCredential(_ jsonArry: Array<OrderedDictionary<String, Any>>) throws {
        guard jsonArry.count != 0 else {
            throw MalformedCredentialError.failue("Invalid verifiableCredentia, should not be an empty array.")
        }
        try jsonArry.forEach { vc in
            let vc: VerifiableCredential = try VerifiableCredential.fromJson(vc)
            addCredential(vc)
        }
    }
    
    /*
     * Normalized serialization order:
     *
     * - type
     * - created
     * - verifiableCredential (ordered by name(case insensitive/ascending)
     * + proof
     *   - type
     *   - verificationMethod
     *   - nonce
     *   - realm
     *   - signature
     */
    public func toJson(_ forSign: Bool) -> OrderedDictionary<String, Any>  {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()

        // type
        dic[Constants.type] = type

        // created
        dic[Constants.created] = created

        // credentials
        var arr: Array<OrderedDictionary<String, Any>> = []
        credentials.values.forEach { vc in
           let dic = vc.toJson(nil, false)
            arr.append(dic)
        }
        dic[Constants.verifiableCredential] = arr

        // proof
        if (!forSign ) {
            let d = proof?.toJson_vp(nil, false)
            dic[Constants.proof] = d
        }
        return dic
    }
    
    public func toJson() -> OrderedDictionary<String, Any> {
       return toJson(false)
    }

    func toJsonForSign() -> String {
        let dic = toJson(true)
        let jsonString: String = JsonHelper.creatJsonString(dic: dic)
        return jsonString
    }
    
    func toExternalForm() -> String {
        let dic = toJson()
        let jsonstring: String = JsonHelper.creatJsonString(dic: dic)
        return jsonstring
    }
    
    public override var description: String {
        return toExternalForm()
    }
}
