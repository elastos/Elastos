import Foundation

public class DIDDocument: NSObject {
    public var subject: DID?
    public var publicKeys: Dictionary<DIDURL, DIDPublicKey> = [: ]
    public var authentications: Dictionary<DIDURL, DIDPublicKey> = [: ]
    public var authorizations: Dictionary<DIDURL, DIDPublicKey> = [: ]
    public var credentials: Dictionary<DIDURL, VerifiableCredential> = [: ]
    public var services: Dictionary<DIDURL, Service> = [: ]
    public var expires: Date?
    public var proof: DIDDocumentProof!
    var meta: DIDMeta = DIDMeta()
    
    override init() {
        super.init()
    }
    
    init(_ subject: DID) {
        super.init()
        self.subject = subject
    }
    
    init(_ doc: DIDDocument) {
        super.init()
        // Copy constructor
        self.subject = doc.subject
        self.publicKeys = doc.publicKeys
        self.authorizations = doc.authorizations
        self.authentications = doc.authentications
        self.credentials = doc.credentials
        self.services = doc.services
        self.expires = doc.expires
        self.proof = doc.proof
        self.meta = doc.meta
    }
    
    public func getPublicKeyCount() -> Int {
        return publicKeys.count
    }
    
    public func getPublicKeys() -> Array<DIDPublicKey> {
        return getEntries(publicKeys)
    }
    
    public func selectPublicKeys(_ id: String, type: String? = nil) throws -> Array<DIDPublicKey> {
        let didurl = try DIDURL(subject!, id)
        return try selectEntry(publicKeys, didurl, type)
    }
    
    public func selectPublicKeys(id: DIDURL? = nil, type: String? = nil) throws -> Array<DIDPublicKey> {
        return try selectEntry(publicKeys, id, type)
    }
    
    public func getPublicKey(_ id: String) throws -> DIDPublicKey? {
        return try getEntry(publicKeys, try DIDURL(subject!, id))
    }
    
    public func getPublicKey(_ id: DIDURL) throws -> DIDPublicKey? {
        return try getEntry(publicKeys, id)
    }
    
    public func hasPublicKey(_ id: DIDURL) throws -> Bool {
        return try getEntry(publicKeys, id) != nil
    }
    
    public func hasPublicKey(_ id: String) throws -> Bool {
        return try hasPublicKey(DIDURL(subject!, id))
    }
    
    public func hasPrivateKey(_ id: DIDURL) throws -> Bool {
        if (try getEntry(publicKeys, id) == nil) {
            return false
        }
        if (!meta.attachedStore()) {
            return false
        }
        
        return try (meta.store?.containsPrivateKey(subject!, id))!
    }
    
    public func hasPrivateKey(_ id: String) throws -> Bool {
        return try hasPrivateKey(DIDURL(subject!, id))
    }
    
    public func getDefaultPublicKey() -> DIDURL {
        var didurl: DIDURL?
        publicKeys.values.forEach{ pk in
            if (pk.controller.isEqual(self.subject)) {
                
                let pks = pk.publicKeyBytes
                let idstring = DerivedKey.getIdString(pks)
                if idstring == subject?.methodSpecificId {
                    didurl = pk.id
                }
            }
        }
        return didurl!
    }
    
    func addPublicKey(_ pk: DIDPublicKey) -> Bool {
        guard publicKeys[pk.id] == nil else {
            return false
        }
        for key in publicKeys.values {
            if key.id == pk.id {
                return false
            }
            if key.publicKeyBase58 == pk.publicKeyBase58 {
                return false
            }
        }
        
        publicKeys[pk.id] = pk
        return true
    }
    
    func removePublicKey(_ id: DIDURL, _ force: Bool) throws -> Bool {
        
        // Cann't remove default public key
        if (getDefaultPublicKey().isEqual(id)) { return false }
        if force {
            _ = removeAuthenticationKey(id)
            _ = removeAuthorizationKey(id)
        }
        else {
            if try isAuthenticationKey(id) || isAuthorizationKey(id) {
                return false
            }
        }
        let removed: Bool = removeEntry("publicKeys", id)
        if removed {
            do {
                if meta.attachedStore() {
                    _ = try meta.store?.deletePrivateKey(subject!, id)
                }
            } catch {
                // TODO: CHECKME!
            }
        }
        
        return removed
    }
    
    public func getAuthenticationKeyCount() -> Int {
        return authentications.count
    }
    
    public func getAuthenticationKeys() -> Array<DIDPublicKey> {
        return getEntries(authentications)
    }
    
    public func selectAuthenticationKeys(id: DIDURL? = nil, type: String? = nil) throws -> Array<DIDPublicKey> {
        return try selectEntry(authentications, id, type)
    }
    
    public func selectAuthenticationKeys(_ id: String, type: String? = nil) throws -> Array<DIDPublicKey>{
        let didurl = try DIDURL(subject!, id)
        return try selectEntry(authentications, didurl, type)
    }
    
    public func getAuthenticationKey(_ id: DIDURL) throws -> DIDPublicKey? {
        return try getEntry(authentications, id)
    }
    
    public func getAuthenticationKey(_ id: String) throws -> DIDPublicKey? {
        return try getEntry(authentications, DIDURL(subject!, id))
    }
    
    public func isAuthenticationKey(_ id: DIDURL) throws -> Bool {
        return try getAuthenticationKey(id) != nil
    }
    
    public func isAuthenticationKey(_ id: String) throws -> Bool {
        return try isAuthenticationKey(DIDURL(id))
    }
    
    func addAuthenticationKey(_ pk: DIDPublicKey) throws -> Bool {
        var pk_ = pk
        // Check the controller is current DID subject
        guard (pk.controller.isEqual(subject))  else {
            return false
        }
        let key = try getPublicKey(pk.id)
        if key == nil {
            // Add the new pk to PublicKeys if not exist.
            _ = addPublicKey(pk)
        }
        else {
            if key != pk { // Key conflict.
                return false
            }
            else { // Already has this key.
                pk_ = key!
            }
        }
        guard authentications[pk_.id] == nil else {
            return false
        }
        authentications[pk_.id] = pk_
        
        return true
    }
    
    func removeAuthenticationKey(_ id: DIDURL) -> Bool {
        // Can not remove default public key
        if (getDefaultPublicKey().isEqual(id)) {
            return false
        }
        return removeEntry("authentications", id)
    }
    
    public func getAuthorizationKeyCount() -> Int {
        return authorizations.count
    }
    
    public func getAuthorizationKeys() -> Array<DIDPublicKey> {
        return getEntries(authorizations)
    }
    
    public func selectAuthorizationKeys(id: DIDURL? = nil, type: String? = nil) throws -> Array<DIDPublicKey> {
        return try selectEntry(authorizations, id, type)
    }
    
    public func selectAuthorizationKeys(_ id: String, type: String? = nil) throws -> Array<DIDPublicKey> {
        let didurl = try DIDURL(id)
        return try selectEntry(authorizations, didurl, type)
    }
    
    public func getAuthorizationKey(_ id: DIDURL) throws -> DIDPublicKey? {
        return try getEntry(authorizations, id)
    }
    
    public func getAuthorizationKey(_ id: String) throws -> DIDPublicKey? {
        return try getEntry(authorizations, DIDURL(subject!, id))
    }
    
    public func isAuthorizationKey(_ id: DIDURL) throws -> Bool {
        return try (getAuthorizationKey(id) != nil)
    }
    
    public func isAuthorizationKey(_ id: String) throws -> Bool {
        return try isAuthorizationKey(DIDURL(subject!, id))
    }
    
    func addAuthorizationKey(_ pk: DIDPublicKey) throws -> Bool {
        var pk_ = pk
        // Can not authorize to self
        if pk.controller.isEqual(subject) { return false }
        let key = try getPublicKey(pk.id)
        if key == nil {
            // Add the new pk to PublicKeys if not exist.
            _ = addPublicKey(pk)
        }
        else {
            if key != pk { // Key conflict.
                return false
            }
            else {
                // Already has this key.
                pk_ = key!
            }
        }
        guard authorizations[pk_.id] == nil else {
            return false
        }
        authorizations[pk_.id] = pk_
        
        return true
    }
    
    func removeAuthorizationKey(_ id: DIDURL) -> Bool {
        return removeEntry("authorizations", id)
    }
    
    public func getCredentialCount() -> Int {
        return credentials.count
    }
    
    public func getCredentials() -> Array<VerifiableCredential> {
        return getEntries(credentials)
    }
    
    public func selectCredentials(id: DIDURL? = nil, type: String? = nil) throws -> Array<VerifiableCredential> {
        return try selectEntry(credentials, id, type)
    }
    
    public func selectCredentials(_ id: String, type: String? = nil) throws -> Array<VerifiableCredential> {
        let didurl = try DIDURL(subject!, id)
        return try selectEntry(credentials, didurl, type)
    }
    
    public func getCredential(_ id: DIDURL) throws -> VerifiableCredential? {
        return try getEntry(credentials, id)
    }
    
    public func getCredential(_ id: String) throws -> VerifiableCredential? {
        return try getEntry(credentials, DIDURL(subject!, id))
    }
    
    func addCredential(_ vc: VerifiableCredential) -> Bool {
        // Check the credential belongs to current DID.
        if vc.subject.id != subject {
            return false
        }
        guard credentials[vc.id] == nil else {
            return false
        }
        credentials[vc.id] = vc
        return true
    }
    
    func removeCredential(_ id: DIDURL) -> Bool {
        return removeEntry("credentials", id)
    }
    
    public func getServiceCount() -> Int {
        return services.count
    }
    
    public func getServices() -> Array<Service> {
        return getEntries(services)
    }
    
    public func selectServices(id: DIDURL? = nil, type: String? = nil) throws -> Array<Service> {
        return try selectEntry(services, id, type)
    }
    
    public func selectServices(_ id: String, type: String? = nil) throws -> Array<Service> {
        let didurl = try DIDURL(subject!, id)
        return try selectEntry(services, didurl, type)
    }
    
    public func getService(_ id: DIDURL) throws -> Service? {
        return try getEntry(services, id)
    }
    
    public func getService(_ id: String) throws -> Service? {
        return try getEntry(services, DIDURL(subject!, id))
    }
    
    func addService(_ svc: Service) throws -> Bool {
        guard services[svc.id] == nil else{
            return false
        }
        services[svc.id] = svc
        return true
    }
    
    func removeService(_ id: DIDURL) -> Bool {
        return removeEntry("services", id)
    }
    
    public func setExtra(_ name: String, _ value: String) throws {
        meta.setExtra(name, value)
        if meta.attachedStore() {
            try meta.store?.storeDidMeta(subject!, meta)
        }
    }
    
    public func getExtra(_ name: String) -> String? {
        return meta.getExtra(name)
    }
    
    public func setAlias(_ alias: String) throws {
        meta.alias = alias
        if (meta.attachedStore()) {
            try meta.store?.storeDidMeta(subject!, meta)
        }
    }
    
    public func getAlias() throws -> String {
        return meta.alias
    }
    
    public func getTransactionId() -> String? {
        return meta.transactionId
    }
    public func getUpdated() -> Date? {
        return meta.updated
    }
    
    public func isDeactivated() throws -> Bool {
        return meta.isDeactivated
    }
    
    public func isExpired() -> Bool {
        let now = DateFormater.currentDate()
        return DateFormater.comporsDate(expires!, now)
        
    }
    
    public func isGenuine() throws -> Bool {
        // Document should signed(only) by default public key.
        if (proof.creator != getDefaultPublicKey()){
            return false
        }
        
        // Unsupported public key type;
        if (proof.type != DEFAULT_PUBLICKEY_TYPE){
            return false
        }
        
        let json = toJson(true, forSign: true)
        
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        
        return try verify(proof.creator!, proof.signature, count, inputs)
    }
    
    public func isValid() throws -> Bool {
        return try !isDeactivated() && !isExpired() && isGenuine()
    }
    
    public func edit() -> DIDDocumentBuilder {
        return DIDDocumentBuilder(doc: self)
    }
    
    public func sign(_ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        let key: DIDURL = getDefaultPublicKey()
        return try sign(key, storepass, count, inputs)
    }
    
    public func sign(_ id: DIDURL, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        if (!meta.attachedStore()) {
            throw DIDError.didStoreError(_desc: "Not attached with DID store.")
        }
        return try meta.store!.sign(subject!, id: id, storepass, count, inputs)
    }
    
    public func sign(_ id: String, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        
        return try sign(DIDURL(subject!, id), storepass, count, inputs)
    }
    
    public func verify(_ signature: String, _ count: Int, _ inputs: [CVarArg]) throws -> Bool {
        let key: DIDURL = getDefaultPublicKey()
        return try verify(key, signature, count, inputs)
    }
    
    public func verify(_ id: String, _ signature: String, _ count: Int, _ inputs: [CVarArg]) throws -> Bool {
        return try verify(DIDURL(subject!, id), signature, count, inputs)
    }
    
    public func verify(_ id: DIDURL, _ signature: String, _ count: Int, _ inputs: [CVarArg]) throws -> Bool {
        var cinputs: [CVarArg] = []
        for i in 0..<inputs.count {
            if (i % 2) == 0 {
                let json: String = inputs[i] as! String
                let cjson  = json.toUnsafePointerInt8()
                cinputs.append(cjson!)
            }
            else {
                let count: Int = inputs[i] as! Int
                cinputs.append(count)
            }
        }
        let pk: DIDPublicKey = try getPublicKey(id)!
        let pks: [UInt8] = pk.publicKeyBytes
        var pkData: Data = Data(bytes: pks, count: pks.count)
        let cpk: UnsafeMutablePointer<UInt8> = pkData.withUnsafeMutableBytes { (pk: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8> in
            return pk
        }
        let csignature = signature.toUnsafeMutablePointerInt8()
        let c_inputs = getVaList(cinputs)
        
        let re = ecdsa_verify_base64v(csignature, cpk, Int32(count), c_inputs)
        return re == 0 ? true : false
    }
    
    private func parse(url: URL) throws {
        let json = try String(contentsOf: url)
        let string = JsonHelper.preHandleString(json)
        let ordDic = JsonHelper.handleString(jsonString: string) as! Dictionary<String, Any>
        return try parse(ordDic)
    }
    
    private func parse(json: String) throws {
        let string = JsonHelper.preHandleString(json)
        let ordDic = JsonHelper.handleString(jsonString: string) as! Dictionary<String, Any>
        
        return try parse(ordDic)
    }
    
    private func parse(_ json: Dictionary<String, Any>) throws {
        self.subject = try JsonHelper.getDid(json, ID, false, "subject")
        
        try parsePublicKey(json)
        try parseAuthentication(json)
        
        let defaultPk = getDefaultPublicKey()
        // Add default public key to authentication keys if needed.
        if (try isAuthenticationKey(defaultPk)){
            _ = try addAuthenticationKey(getPublicKey(defaultPk)!)
        }
        
        try parseAuthorization(json)
        try parseCredential(json)
        try parseService(json)
        expires = try JsonHelper.getDate(json, EXPIRES, true, "expires")
        
        let poorf = json[PROOF] as! Dictionary<String, Any>
        if poorf.count == 0 {
            throw DIDError.malFormedDocumentError(_desc: "Missing proof.")
        }
        proof = try DIDDocumentProof.fromJson(poorf, defaultPk)
    }
    
    // 解析公钥
    private func parsePublicKey(_ json: Dictionary<String, Any>) throws {
        let publicKeys = json["publicKey"] as? Array<Any>
        
        guard publicKeys != nil else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid publicKey, should be an array.")
        }
        
        let publicKeysArray: [Dictionary<String, Any>] = publicKeys as! Array<Dictionary<String, Any>>
        guard publicKeysArray.count != 0 else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid publicKey, should not be an empty array.")
        }
        
        for publicKey in publicKeysArray {
            let pk = try DIDPublicKey.fromJson(publicKey, self.subject!)
            _ = addPublicKey(pk)
        }
    }
    
    // MARK: parseAuthentication
    private func parseAuthentication(_ json: Dictionary<String, Any>) throws {
        let authentications = json[AUTHENTICATION] as? Array<Any>
        
        guard (authentications != nil) else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid authentication, should be an array.")
        }
        
        guard authentications!.count != 0 else {
            return
        }
        
        try authentications!.forEach { (obj) in
            var pk: DIDPublicKey
            if obj is Dictionary<String, Any> {
                let object: Dictionary<String, Any> = obj as! Dictionary<String, Any>
                pk = try DIDPublicKey.fromJson(object, subject!)
            }else {
                let objString: String = obj as! String
                let index = objString.index(objString.startIndex, offsetBy: 1)
                let str: String = String(objString[..<index])
                var didString: String = objString
                if str == "#" {
                    let id: String = json[ID] as! String
                    didString = id + objString
                }
                let didUrl: DIDURL = try DIDURL(didString)
                pk = publicKeys[didUrl]!
            }
            _ = try addAuthenticationKey(pk)
        }
    }
    
    private func parseAuthorization(_ json: Dictionary<String, Any>) throws {
        let aus = json[AUTHORIZATION]
        guard (aus != nil) else {
            return
        }
        let authorizations = aus as? Array<Any>
        guard (authorizations != nil) else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid authorization, should be an array.")
        }
        guard authorizations!.count != 0  else {
            return
        }
        try authorizations!.forEach { (obj) in
            var pk: DIDPublicKey
            if obj is Dictionary<String, Any> {
                let object: Dictionary<String, Any> = obj as! Dictionary<String, Any>
                pk = try DIDPublicKey.fromJson(object, subject!)
            }else {
                let id: DIDURL = try JsonHelper.getDidUrl(obj as! String, subject, "authorization publicKey id")
                pk = publicKeys[id]!
            }
            _ = try addAuthorizationKey(pk)
        }
    }
    
    // mode parse
    private func parseCredential(_ json: Dictionary<String, Any>) throws {
        let crs = json[VERIFIABLE_CREDENTIAL]
        guard (crs != nil) else {
            return
        }
        let cdentials = crs as? Array<Dictionary<String, Any>>
        guard (cdentials != nil) else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid credential, should be an array.")
        }
        guard cdentials!.count != 0  else {
            return
        }
        try cdentials!.forEach{ obj in
            let vc: VerifiableCredential = try VerifiableCredential.fromJson(obj, subject!)
            _ = addCredential(vc)
        }
    }
    
    private func parseService(_ json: Dictionary<String, Any>) throws {
        let ses = json[SERVICE]
        guard (ses != nil) else {
            return
        }
        let services = ses as? Array<Dictionary<String, Any>>
        guard (services != nil) else {
            throw DIDError.malFormedDocumentError(_desc: "Invalid services, should be an array.")
        }
        guard services!.count != 0  else {
            return
        }
        
        try services!.forEach { obj in
            let svc: Service = try Service.fromJson(obj, subject!)
            _ = try addService(svc)
        }
    }
    
    public class func fromJson(path: String) throws -> DIDDocument{
        
        let doc: DIDDocument = DIDDocument()
        let urlPath = URL(fileURLWithPath: path)
        try doc.parse(url: urlPath)
        return doc
    }
    
    public class func fromJson(_ path: URL) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        try doc.parse(url: path)
        return doc
    }
    
    public class func fromJson(_ json: String) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        try doc.parse(json: json)
        return doc
    }
    
    public func fromJson(_ dic: Dictionary<String, Any>) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        let ordDic: Dictionary<String, Any> = dic
        try doc.parse(ordDic)
        return doc
    }
    
    /*
     * Normalized serialization order:
     *
     * - id
     * + publickey
     *   + public keys array ordered by id(case insensitive/ascending)
     *     - id
     *     - type
     *     - controller
     *     - publicKeyBase58
     * + authentication
     *   - ordered by public key' ids(case insensitive/ascending)
     * + authorization
     *   - ordered by public key' ids(case insensitive/ascending)
     * + verifiableCredential
     *   - credentials array ordered by id(case insensitive/ascending)
     * + service
     *   + services array ordered by id(case insensitive/ascending)
     *     - id
     *     - type
     *     - endpoint
     * - expires
     * + proof
     *   - type
     *   - created
     *   - creator
     *   - signatureValue
     */
    public func toJson(_ normalized: Bool, forSign: Bool? = false) -> String {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        // subject
        dic[ID] = subject?.description
        
        // publicKey
       let _publicKeys = DIDURLComparator.DIDOrderedDictionaryComparator(publicKeys)
        var pks: Array<OrderedDictionary<String, Any>> = []
        _publicKeys.forEach { (didUrl, pk) in
            let dic = pk.toJson(subject!, normalized)
            pks.append(dic)
        }
        dic[PUBLICKEY] = pks
        
        // authentication
        let _authentications = DIDURLComparator.DIDOrderedDictionaryComparator(authentications)
        var authenPKs: Array<String> = []
        _authentications.forEach { (didUrl, pk) in
            var value: String
            if normalized ||  pk.id.did != subject{
                value = pk.id.toExternalForm()
            }
            else {
                value = "#" + pk.id.fragment!
            }
            authenPKs.append(value)
        }
        dic[AUTHENTICATION] = authenPKs
        
        // authorization
        if authorizations.count > 0 {
           let _authorizations = DIDURLComparator.DIDOrderedDictionaryComparator(authorizations)
            var authoriPks: Array<String> = []
            _authorizations.forEach { (didUrl, pk) in
                var value: String
                if normalized || pk.id.did != subject {
                    value = pk.id.toExternalForm()
                }else {
                    value = "#" + pk.id.fragment!
                }
                authoriPks.append(value)
            }
            dic[AUTHORIZATION] = authoriPks
        }
        
        // credential
        if credentials.count > 0 {
            let _credentials = DIDURLComparator.DIDOrderedDictionaryComparatorByVerifiableCredential(credentials)
            var vcs: Array<OrderedDictionary<String, Any>> = []
            _credentials.forEach { (didUrl, vc) in
                let dic = vc.toJson(subject!, normalized, false)
                vcs.append(dic)
            }
            dic[VERIFIABLE_CREDENTIAL] = vcs
        }
        
        // service
        if services.count > 0 {
            let _services = DIDURLComparator.DIDOrderedDictionaryComparatorByService(services)
            var ser_s: Array<OrderedDictionary<String, Any>> = [ ]
            _services.forEach { (_, service) in
                let dic = service.toJson(subject!, normalized)
                ser_s.append(dic)
            }
            dic[SERVICE] = ser_s
        }
        
        // expires
        if expires != nil {
            dic[EXPIRES] = DateFormater.format(expires!)
        }
        
        // proof
        if !forSign! {
            dic[PROOF] = proof.toJson(normalized)
        }
        let dicString = JsonHelper.creatJsonString(dic: dic)
        
        return dicString
    }
    
    public override var description: String {
        return toJson(false, forSign: false)
    }
    
    public func description(_ normalized: Bool) -> String {
        return toJson(normalized, forSign: false)
    }
    
    private func selectEntry<T: DIDObject>(_ entries: Dictionary<DIDURL, T>, _ id: DIDURL?, _ type: String?) throws -> Array<T>  {
        var list: Array<T> = []
        entries.values.forEach { entry in
            var isId: Bool = true
            if id != nil { isId = entry.id.isEqual(id)}
            
            if (type != nil) {
                // Credential's type is a list.
                if (entry is VerifiableCredential) {
                    let vc: VerifiableCredential = entry as! VerifiableCredential
                    if vc.types.contains(type!) && isId {
                        list.append(entry)
                    }
                } else {
                    if entry.type.isEqual(type) && isId {
                        list.append(entry)
                    }
                }
            }
            else
            {
                if isId { list.append(entry) }
            }
        }
        return list
    }
    
    private func  getEntries<T>(_ entries: Dictionary<DIDURL, T>) -> Array<T> {
        var list: Array<T> = []
        
        entries.forEach{ (arg: (key: DIDURL, value: T)) in
            let (_, value) = arg
            list.append(value)
        }
        return list
    }
    
    private func getEntry<T: DIDObject>(_ entries: Dictionary<DIDURL, T>, _ id: DIDURL) throws -> T? {
        let re = entries[id]
        return re
    }
    
    private func removeEntry(_ entriesType: String, _ id: DIDURL) -> Bool {
        if entriesType == "publicKeys" {
            return self.publicKeys.removeValue(forKey: id) != nil
        }
        else if entriesType == "authentications" {
            return self.authentications.removeValue(forKey: id) != nil
        }
        else if entriesType == "authorizations" {
            return self.authorizations.removeValue(forKey: id) != nil
        }
        else if entriesType == "credentials" {
            return self.credentials.removeValue(forKey: id) != nil
        }
        else {
            return self.services.removeValue(forKey: id) != nil
        }
    }
}

