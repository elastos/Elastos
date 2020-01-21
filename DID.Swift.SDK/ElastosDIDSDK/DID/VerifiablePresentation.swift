
import Foundation

public class VerifiablePresentation: NSObject{
   public var type: String!
   public var created: Date!
    public var credentials: Dictionary<DIDURL, VerifiableCredential> = [: ]
   public var proof: PresentationProof?
    
    override init() {
        type = DEFAULT_PRESENTTATION_TYPE
        created = DateFormater.currentDate()
    }
    
    public func getCredentials() -> Array<VerifiableCredential> {
        var arr: Array<VerifiableCredential> = []
        for vc in credentials.values {
            arr.append(vc)
        }
        return arr
    }
    
    public func addCredential(_ credential: VerifiableCredential) {
        credentials[credential.id] = credential
    }
    
    public func getCredential(_ id: DIDURL) throws -> VerifiableCredential? {
        return credentials[id]
    }
    
    public func getCredential(_ id: String) throws -> VerifiableCredential? {
        return try getCredential(DIDURL(getSigner(), id))
    }
    
    public func getSigner() -> DID {
        return (proof?.verificationMethod.did)!
    }
    
    public func isGenuine() throws -> Bool {
        let signer: DID = getSigner()
        let signerDoc = try signer.resolve()
        if (signerDoc == nil) {
            return false
        }
        
        // Check the integrity of signer' document.
        if (try !signerDoc!.isGenuine()) {
            return false
        }
        // Unsupported public key type;
        if (proof?.type != (DEFAULT_PUBLICKEY_TYPE)) {
            return false
        }
        // Credential should signed by authentication key.
        if (try !signerDoc!.isAuthenticationKey(proof!.verificationMethod)) {
        return false
        }
        
        // All credentials should owned by signer
        for vc in credentials.values {
            if vc.subject.id != signer {
                return false
            }
            
            if try !vc.isGenuine() {
                return false
            }
        }
        let dic = toJson(true)
        let json = JsonHelper.creatJsonString(dic: dic)
        var inputs: [CVarArg] = []
        if json.count > 0 {
            inputs.append(json)
            inputs.append(json.count)
        }
        if proof?.realm != nil && proof!.realm!.count > 0 {
            inputs.append(proof!.realm!)
            inputs.append(proof!.realm!.count)
        }
        if proof?.nonce != nil && proof!.nonce!.count > 0 {
            inputs.append(proof!.nonce!)
            inputs.append(proof!.nonce!.count)
        }
        let count = inputs.count / 2
        return try signerDoc!.verify(proof!.verificationMethod, proof!.signature, count, inputs)
    }
    
    public func isValid() throws -> Bool {
        let signer: DID = getSigner()
        let signerDoc = try signer.resolve()

        if signerDoc == nil {
            return false
        }
        
        // Check the validity of signer' document.
        if (try !signerDoc!.isValid()){
            return false
        }

        // Unsupported public key type;
        if (proof!.type != (DEFAULT_PUBLICKEY_TYPE)){
            return false
        }

        // Credential should signed by authentication key.
        if (try !signerDoc!.isAuthenticationKey(proof!.verificationMethod)){
            return false
        }

        // All credentials should owned by signer
        
        for vc in credentials.values {
            if (vc.subject.id != signer) {
                return false
            }
            if try !vc.isValid() {
                return false
            }
        }
        let dic = toJson(true)
        let json = JsonHelper.creatJsonString(dic: dic)
        var inputs: [CVarArg] = []
        if json.count > 0 {
            inputs.append(json)
            inputs.append(json.count)
        }
        
        if proof?.realm != nil && proof!.realm!.count > 0 {
            inputs.append(proof!.realm!)
            inputs.append(proof!.realm!.count)
        }
        if proof?.nonce != nil && proof!.nonce!.count > 0 {
            inputs.append(proof!.nonce!)
            inputs.append(proof!.nonce!.count)
        }
        let count = inputs.count / 2
        
        return try signerDoc!.verify(proof!.verificationMethod, proof!.signature, count, inputs)
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
        let string = JsonHelper.preHandleString(jsonString)
        let dic = JsonHelper.handleString(jsonString: string) as! Dictionary<String, Any>
        try parse(dic)
    }
    
    func parse(_ presentation: Dictionary<String, Any>) throws {
        
        let type: String = try JsonHelper.getString(presentation, TYPE, false, "presentation type")
        guard type == DEFAULT_PRESENTTATION_TYPE else {
            throw DIDError.malformedCredentialError(_desc: "Unknown presentation type: \(type)")
        }
        self.type = type
        let created: Date = try JsonHelper.getDate(presentation, CREATED, false, "presentation created date")!
        self.created = created
        
        var d = presentation[VERIFIABLE_CREDENTIAL]
        guard d != nil else {
            throw DIDError.malformedCredentialError(_desc: "Missing credentials.")
        }
        guard d is Array<Any> else {
            throw DIDError.malformedCredentialError(_desc: "Invalid verifiableCredentia, should be an array.")
        }
        try parseCredential(d as! Array<Dictionary<String, Any>>)
        
        d = presentation[PROOF]
        guard d != nil else {
            throw DIDError.malformedCredentialError(_desc: "Missing credentials.")
        }
        let proof: PresentationProof = try PresentationProof.fromJson(d as! Dictionary<String, Any>, nil)
        self.proof = proof
    }
    
    func parseCredential(_ jsonArry: Array<Dictionary<String, Any>>) throws {
        guard jsonArry.count != 0 else {
            throw DIDError.malformedCredentialError(_desc: "Invalid verifiableCredentia, should not be an empty array.") 
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
        dic[TYPE] = type

        // created
        dic[CREATED] = DateFormater.format(created)

        // credentials
        var arr: Array<OrderedDictionary<String, Any>> = []
        let _credentials = DIDURLComparator.DIDOrderedDictionaryComparatorByVerifiableCredential(credentials)
        _credentials.values.forEach { vc in
           let dic = vc.toJson(true)
            arr.append(dic)
        }
        dic[VERIFIABLE_CREDENTIAL] = arr

        // proof
        if (!forSign ) {
            let d = proof?.toJson()
            dic[PROOF] = d
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
    
    public class func createFor(_ did: DID, signKey: DIDURL? = nil, _ store: DIDStore) throws -> VerifiablePresentationBuilder {
        var sigK: DIDURL? = signKey
        let signer: DIDDocument? = try store.loadDid(did)
        guard signer != nil else {
            throw DIDError.illegalArgument("")
        }
        if sigK == nil {
            sigK = signer!.getDefaultPublicKey()
        }
        else {
            guard try signer!.isAuthenticationKey(sigK!) else {
                throw DIDError.didExpiredError(_desc: "Invalid sign key id.")
            }
        }
        guard try signer!.hasPrivateKey(sigK!) else {
            throw DIDError.didExpiredError(_desc: "No private key.")
        }
        return VerifiablePresentationBuilder(signer!, sigK!, store)
    }
}
