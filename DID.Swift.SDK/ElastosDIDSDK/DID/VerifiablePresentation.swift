
import Foundation

public class VerifiablePresentation: NSObject{
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
    
    public func getCredential(_ id: DIDURL) throws -> VerifiableCredential? {
        return credentials![id]
    }
    
    public func getCredential(_ id: String) throws -> VerifiableCredential? {
        return try getCredential(DIDURL(getSigner(), id))
    }
    
    public func getSigner() -> DID {
        return (proof?.verificationMethod.did)!
    }
    
    public func isGenuine() throws -> Bool {
        let signer: DID = getSigner()
        let signerDoc: DIDDocument = try signer.resolve()!

        // Check the integrity of signer' document.
        if (try !signerDoc.isGenuine()) {
            return false
        }
        // Unsupported public key type;
        if (proof?.type != (Constants.defaultPublicKeyType)) {
            return false
        }
        // Credential should signed by authentication key.
        if (try !signerDoc.isAuthenticationKey(proof!.verificationMethod)) {
        return false
        }

        // All credentials should owned by signer
        for i in 0..<credentials.values.count {
            let vc = credentials.values[i]
            if vc.subject.id == signer {
                return false
            }
            
            if try vc.isGenuine() {
                return false
            }
        }
        let dic = toJson(true)
        let json = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        return try signerDoc.verify(proof!.verificationMethod, proof!.signature, count, inputs)
    }
    
    public func isValid() throws -> Bool {
        let signer: DID = getSigner()
        let signerDoc: DIDDocument = try signer.resolve()!

        // Check the validity of signer' document.
        if (try !signerDoc.isValid()){
            return false
        }

        // Unsupported public key type;
        if (proof!.type != (Constants.defaultPublicKeyType)){
            return false
        }

        // Credential should signed by authentication key.
        if (try !signerDoc.isAuthenticationKey(proof!.verificationMethod)){
            return false
        }

        // All credentials should owned by signer
        
        for i in 0..<credentials.values.count {
            let vc = credentials.values[0]
            if (vc.subject.id != signer) {
                return false
            }
            if try !vc.isValid() {
                return false
            }
        }
        let dic = toJson(true)
        let json = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        
        return try signerDoc.verify(proof!.verificationMethod, proof!.signature, count, inputs)
    }
    
    public func getCredentialCount() -> Int {
        return credentials.count
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
        
        let type: String = try JsonHelper.getString(presentation, Constants.TYPE, false, nil, "presentation type")
        guard type == Constants.defaultPresentationType else {
            throw MalformedCredentialError.failue("Unknown presentation type: \(type)")
        }
        self.type = type
        let created: Date = try DateFormater.getDate(presentation, Constants.CREATED, false, nil, "presentation created date")!
        self.created = created
        
        var d = presentation[Constants.VERIFIABLE_CREDENTIAL]
        guard d != nil else {
            throw MalformedCredentialError.failue("Missing credentials.")
        }
        guard d is Array<Any> else {
            throw MalformedCredentialError.failue("Invalid verifiableCredentia, should be an array.")
        }
        try parseCredential(d as! Array<OrderedDictionary<String, Any>>)
        
        d = presentation[Constants.PROOF]
        guard d != nil else {
           throw MalformedCredentialError.failue("Missing credentials.")
        }
        let proof: Proof = try Proof.fromJson_vp(d as! OrderedDictionary<String, Any>, nil)
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
     *   - realm
     *   - nonce
     *   - signature
     */
    public func toJson(_ forSign: Bool) -> OrderedDictionary<String, Any>  {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()

        // type
        dic[Constants.TYPE] = type

        // created
        dic[Constants.CREATED] = DateFormater.format(created)

        // credentials
        var arr: Array<OrderedDictionary<String, Any>> = []
        credentials.values.forEach { vc in
           let dic = vc.toJson(true)
            arr.append(dic)
        }
        dic[Constants.VERIFIABLE_CREDENTIAL] = arr

        // proof
        if (!forSign ) {
            let d = proof?.toJson_vp()
            dic[Constants.PROOF] = d
        }
        return dic
    }
    
    public func toJson() -> OrderedDictionary<String, Any> {
       return toJson(false)
    }

    func toJsonForSign(_ forSign: Bool) -> String {
        let dic = toJson(forSign)
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
    
    class public func seal(for did : DID, _ credentials: Array<VerifiableCredential>, _ realm: String, _ nonce: String, _ storepass: String) throws -> VerifiablePresentation {
        let signer = try did.resolve()
        if (signer == nil) {
            throw DIDError.failue("Can not resolve DID.")
        }
        
        let signKey = signer!.getDefaultPublicKey()
        if try !signer!.isAuthorizationKey(signKey) {
            throw DIDError.failue("Invalid sign key id.")
        }
        if (try signer!.hasPrivateKey(signKey)) {
            throw DIDError.failue("No private key.")
        }
        let presentation: VerifiablePresentation = VerifiablePresentation()
        for vc in credentials {
            presentation.addCredential(vc)
        }
        
        let dic = presentation.toJson(true)
        let json = JsonHelper.creatJsonString(dic: dic)
        let inputs: [CVarArg] = [json, json.count]
        let count: Int = inputs.count / 2
        let sig = try signer?.sign(signKey, storepass, count, inputs)
        let proof = Proof(signKey, realm, nonce, sig!)
        presentation.proof = proof
        
        return presentation
    }
}
