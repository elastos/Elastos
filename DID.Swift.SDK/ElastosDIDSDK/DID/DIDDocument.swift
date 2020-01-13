import Foundation

public class DIDDocument: NSObject {
    public var subject: DID?
    public var publicKeys: OrderedDictionary<DIDURL, DIDPublicKey> = OrderedDictionary()
    public var authentications: OrderedDictionary<DIDURL, DIDPublicKey> = OrderedDictionary()
    public var authorizations: OrderedDictionary<DIDURL, DIDPublicKey> = OrderedDictionary()
    public var credentials: OrderedDictionary<DIDURL, VerifiableCredential> = OrderedDictionary()
    public var services: OrderedDictionary<DIDURL, Service> = OrderedDictionary()
    public var expires: Date?
    public var proof: Proof!
    public var meta: DIDMeta = DIDMeta()
    
    override init() {
        super.init()
    }
    
    public init(_ subject: DID) {
        super.init()
        self.subject = subject
    }

    public init(_ doc: DIDDocument) {
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
            if (pk.controller?.isEqual(self.subject))! {
                
                let pks = pk.getPublicKeyBytes()
                let idstring = DerivedKey.getIdString(pks)
                if idstring == subject?.methodSpecificId {
                    didurl = pk.id
                }
            }
        }
        return didurl!
    }
    
    public func addPublicKey(_ pk: DIDPublicKey) -> Bool {
        guard publicKeys[pk.id] == nil else {
            return false
        }
        for key in publicKeys.values {
            if key.id == pk.id {
                return false
            }
            if key.keyBase58 == pk.keyBase58 {
                return false
            }
        }
        
        publicKeys[pk.id] = pk
        return true
    }
    
    public func removePublicKey(_ id: DIDURL, _ force: Bool) throws -> Bool {
        
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
        guard ((pk.controller?.isEqual(subject))!)  else {
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
    
    public func removeAuthenticationKey(_ id: DIDURL) -> Bool {
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
    
    public func addAuthorizationKey(_ pk: DIDPublicKey) throws -> Bool {
        var pk_ = pk
        // Can not authorize to self
        if (pk.controller?.isEqual(subject))! { return false }
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
    
    public func removeAuthorizationKey(_ id: DIDURL) -> Bool {
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
    
    public func addCredential(_ vc: VerifiableCredential) -> Bool {
        // Check the credential belongs to current DID.
        if vc.subject.id != subject {
            return false
        }
        guard credentials[vc.id] == nil else {
            return false
        }
        let ec: EmbeddedCredential = EmbeddedCredential(vc)
        credentials[ec.id] = ec
        return true
    }
    
    public func removeCredential(_ id: DIDURL) -> Bool {
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
    
    public func addService(_ svc: Service) throws -> Bool {
        guard services[svc.id] == nil else{
            return false
        }
        services[svc.id] = svc
        return true
    }
    
    public func removeService(_ id: DIDURL) -> Bool {
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
    
    public func getTransactionId() -> String {
        return meta.transactionId
    }
    public func getUpdated() -> Date? {
        return meta.updated
    }

    public func isDeactivated() throws -> Bool {
        return meta.isDeactivated()
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
        
        let json = try toJson(true, true)
        
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        
        return try verify(proof.creator!, proof.signature, count, inputs)
    }
    
    public func isValid() throws -> Bool {
        return try !isDeactivated() && !isExpired() && isGenuine()
    }
    
    public func sign(_ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        let key: DIDURL = getDefaultPublicKey()
        return try sign(key, storepass, count, inputs)
    }
    
    public func sign(_ id: DIDURL, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        if (!meta.attachedStore()) {
            throw DIDStoreError.failue("Not attached with DID store.")
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
        let pks: [UInt8] = pk.getPublicKeyBytes()
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
        let json = try! String(contentsOf: url)
        let string = JsonHelper.preHandleString(json)
        let ordDic = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        return try parse(ordDic)
    }
    
    private func parse(json: String) throws {
        let string = JsonHelper.preHandleString(json)
        let ordDic = JsonHelper.handleString(string) as! OrderedDictionary<String, Any>
        
        return try parse(ordDic)
    }
    
    private func parse(_ json: OrderedDictionary<String, Any>) throws {
        self.subject = try JsonHelper.getDid(json, ID, false, nil, "subject")
        
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
        expires = try DateFormater.getDate(json, EXPIRES, true, nil, "expires")
        
        let poorf = json[PROOF] as! OrderedDictionary<String, Any>
        if poorf.count == 0 {
            throw MalformedDocumentError.failue("Missing proof.")
        }
        proof = try Proof.fromJson_dc(poorf, defaultPk)
    }
    
    // 解析公钥
    private func parsePublicKey(_ json: OrderedDictionary<String, Any>) throws {
        let publicKeys = json["publicKey"] as? Array<Any>
        
        guard publicKeys != nil else {
            throw MalformedDocumentError.failue("Invalid publicKey, should be an array.")
        }
        
        let publicKeysArray: [OrderedDictionary<String, Any>] = publicKeys as! Array<OrderedDictionary<String, Any>>
        guard publicKeysArray.count != 0 else {
            throw MalformedDocumentError.failue("Invalid publicKey, should not be an empty array.")
        }
        
        for publicKey in publicKeysArray {
            let pk = try DIDPublicKey.fromJson(publicKey, self.subject!)
            _ = addPublicKey(pk)
        }
    }
    
    // MARK: parseAuthentication
    private func parseAuthentication(_ json: OrderedDictionary<String, Any>) throws {
        let authentications = json[AUTHENTICATION] as? Array<Any>
        
        guard (authentications != nil) else {
            throw MalformedDocumentError.failue("Invalid authentication, should be an array.")
        }
        
        guard authentications!.count != 0 else {
            return
        }
        
        try authentications!.forEach { (obj) in
            var pk: DIDPublicKey
            if obj is OrderedDictionary<String, Any> {
                let object: OrderedDictionary<String, Any> = obj as! OrderedDictionary<String, Any>
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
    
    private func parseAuthorization(_ json: OrderedDictionary<String, Any>) throws {
        let aus = json[AUTHORIZATION]
        guard (aus != nil) else {
            return
        }
        let authorizations = aus as? Array<Any>
        guard (authorizations != nil) else {
            throw MalformedDocumentError.failue("Invalid authorization, should be an array.")
        }
        guard authorizations!.count != 0  else {
            return
        }
        try authorizations!.forEach { (obj) in
            var pk: DIDPublicKey
            if obj is OrderedDictionary<String, Any> {
                let object: OrderedDictionary<String, Any> = obj as! OrderedDictionary<String, Any>
                pk = try DIDPublicKey.fromJson(object, subject!)
            }else {
                let id: DIDURL = try JsonHelper.getDidUrl(obj as! String, subject, "authorization publicKey id")
                pk = publicKeys[id]!
            }
            _ = try addAuthorizationKey(pk)
        }
    }
    
    // mode parse
    private func parseCredential(_ json: OrderedDictionary<String, Any>) throws {
        let crs = json[VERIFIABLE_CREDENTIAL]
        guard (crs != nil) else {
            return
        }
        let cdentials = crs as? Array<OrderedDictionary<String, Any>>
        guard (cdentials != nil) else {
            throw MalformedDocumentError.failue("Invalid credential, should be an array.")
        }
        guard cdentials!.count != 0  else {
            return
        }
        try cdentials!.forEach{ obj in
            let vc: VerifiableCredential = try VerifiableCredential.fromJson(obj, subject!)
            _ = addCredential(vc)
        }
    }
    
    private func parseService(_ json: OrderedDictionary<String, Any>) throws {
        let ses = json[SERVICE]
        guard (ses != nil) else {
            return
        }
        let services = ses as? Array<OrderedDictionary<String, Any>>
        guard (services != nil) else {
            throw MalformedDocumentError.failue("Invalid services, should be an array.")
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
    
    public func fromJson(_ dic: OrderedDictionary<String, Any>) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        let ordDic: OrderedDictionary<String, Any> = dic
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
    public func toJson(path: String? = nil, _ normalized: Bool, _ forSign: Bool) throws -> String {
           var dic: OrderedDictionary<String, Any> = OrderedDictionary()
           // subject
           dic[ID] = subject?.description
           
           // publicKey
           publicKeys = DIDURLComparator.DIDOrderedDictionaryComparator(publicKeys)
           var pks: Array<OrderedDictionary<String, Any>> = []
           publicKeys.forEach { (didUrl, pk) in
               let dic = pk.toJson_dc(subject!, normalized)
               pks.append(dic)
           }
           dic[PUBLICKEY] = pks
           
           // authentication
           authentications = DIDURLComparator.DIDOrderedDictionaryComparator(authentications)
           var authenPKs: Array<String> = []
           authentications.forEach { (didUrl, pk) in
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
               authorizations = DIDURLComparator.DIDOrderedDictionaryComparator(authorizations)
               var authoriPks: Array<String> = []
               authorizations.forEach { (didUrl, pk) in
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
               credentials = DIDURLComparator.DIDOrderedDictionaryComparatorByVerifiableCredential(credentials)
               var vcs: Array<OrderedDictionary<String, Any>> = []
               credentials.forEach { (didUrl, vc) in
                   let dic = vc.toJson(subject!, normalized, false)
                   vcs.append(dic)
               }
               dic[VERIFIABLE_CREDENTIAL] = vcs
           }
           
           // service
           if services.count > 0 {
               services = DIDURLComparator.DIDOrderedDictionaryComparatorByService(services)
               var ser_s: Array<OrderedDictionary<String, Any>> = [ ]
               services.forEach { (_, service) in
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
           if !forSign {
               dic[PROOF] = proof.toJson_dc(normalized)
           }
           
           let dicString = JsonHelper.creatJsonString(dic: dic)
           
           guard path != nil else {
               return dicString
           }
           let data: Data = dicString.data(using: .utf8)!
           // & Write to local
           let dirPath: String = PathExtracter(path!).dirNamePart()
           let fileM = FileManager.default
           let re = fileM.fileExists(atPath: dirPath)
           if !re {
               try fileM.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
           }
           let dre = fileM.fileExists(atPath: path!)
           if !dre {
               fileM.createFile(atPath: path!, contents: nil, attributes: nil)
           }
           let writeHandle = FileHandle(forWritingAtPath: path!)
           writeHandle?.write(data)
           return dicString
       }

    public func description() throws -> String {
        return try toJson(false, false)
    }
    
    public func description(_ normalized: Bool) throws -> String {
        return try toJson(normalized, false)
    }

    // ----------------------------------------
    public func addPublicKey(_ id: DIDURL, _ controller: DID, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, controller, pk)
        return addPublicKey(key)
    }
    
    public func addPublicKey(_ id: String, _ controller: String, _ pk: String) throws -> Bool {
        return try addPublicKey(DIDURL(subject!, id), DID(controller), pk)
    }
    
    public func removePublicKey(_ id: String, _ force: Bool) throws -> Bool {
        return try removePublicKey(DIDURL(subject!, id), force)
    }
    
    public func removePublicKey(_ id: DIDURL) throws -> Bool {
        return try removePublicKey(id, false)
    }
    
    public func removePublicKey(_ id: String) throws -> Bool {
        return try removePublicKey(DIDURL(id), false)
    }
    public func addAuthenticationKey(_ id: DIDURL) throws -> Bool {
        let pk = try getPublicKey(id)
        guard pk != nil else {
            return false
        }
        return try addAuthenticationKey(pk!)
    }
    
    public func addAuthenticationKey(_ id: String) throws -> Bool {
        return try addAuthenticationKey(DIDURL(subject!, id))
    }
    
    public func addAuthenticationKey(_ id: DIDURL, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, subject!, pk)
        return try addAuthenticationKey(key)
    }
    
    public func addAuthenticationKey(_ id: String, _ pk: String) throws -> Bool {
        return try addAuthenticationKey(DIDURL(subject!, id), pk)
    }
    
    public func removeAuthenticationKey(_ id: String) throws -> Bool {
        return try removeAuthenticationKey(DIDURL(subject!, id))
    }
    
    public func addAuthorizationKey(_ id: DIDURL) throws -> Bool {
        let pk = try getPublicKey(id)
        guard pk != nil else {
            return false
        }
        return try addAuthorizationKey(pk!)
    }
    
    public func addAuthorizationKey(_ id: String) throws -> Bool {
       return try  addAuthorizationKey(DIDURL(subject!, id))
    }
    
    public func addAuthorizationKey(_ id: DIDURL, _ controller: DID, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw MalformedDocumentError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, controller, pk)
        return try addAuthorizationKey(key)
    }
                
    public func addAuthorizationKey(_ id: String, _ controller: String, _ pk: String) throws -> Bool {
        return try addAuthorizationKey(DIDURL(subject!, id), DID(controller), pk)
    }
    
    public func authorizationDid(_ id: DIDURL, _ controller: DID, key: DIDURL? = nil) throws -> Bool {
        // Can not authorize to self
        if controller.isEqual(subject) {
            return false
        }
        let doc = try controller.resolve()
        var k: DIDURL
        if key == nil {
            k = doc!.getDefaultPublicKey()
        }else {
            k = key!
        }
        let refPk = try doc!.getPublicKey(k)
        
        // The public key should belongs to controller
        if (refPk!.controller! != controller) {
            return false
        }
        let pk: DIDPublicKey = DIDPublicKey(id, refPk!.type, controller, refPk!.keyBase58!)
        return try addAuthorizationKey(pk)
    }
    
    public func authorizationDid(_ id: String, _ controller: String, _ key: String?) throws -> Bool {
        let controllerId: DID = try DID(controller)
        let keyid = (key == nil ? nil : try DIDURL(controllerId, key!))
        return try authorizationDid(DIDURL(subject!, id), controllerId, key: keyid)
    }
    
    public func authorizationDid(_ id: String, _ controller: String) throws -> Bool {
        return try authorizationDid(id, controller, nil)
    }
    
    public func removeAuthorizationKey(_ id: String) throws -> Bool {
        return try removeAuthorizationKey(DIDURL(subject!, id))
    }

    public func removeCredential(_ id: String) throws -> Bool {
        return try removeCredential(DIDURL(subject!, id))
    }
    
    public func addService(_ id: DIDURL, _ type: String, _ endpoint: String) throws -> Bool {
        let svc: Service = Service(id, type, endpoint)
        return try addService(svc)
    }
    
    public func addService(_ id: String, _ type: String, _ endpoint: String) throws -> Bool {
        return try addService(DIDURL(subject!, id), type, endpoint)
    }
    
    public func removeService(_ id: String) throws -> Bool {
        return try removeService(DIDURL(subject!, id))
    }
    
    public func setDefaultExpires() {
        expires = DateFormater.currentDateToWantDate(MAX_VALID_YEARS)
    }
    
    public func setExpires(_ expiresDate: Date) -> Bool {
        let MaxExpires = DateFormater.currentDateToWantDate(MAX_VALID_YEARS)
        if DateFormater.comporsDate(expiresDate, MaxExpires) {
            self.expires = DateFormater.setExpires(expiresDate)
            return true
        }
        return false
    }
    
    public func seal(_ store: DIDStore, _ storepass: String) throws -> DIDDocument {
        
        setDefaultExpires()
        meta.store = store
        let signKey: DIDURL = getDefaultPublicKey()
        let json = try toJson(true, true)
        let inputs: [CVarArg] = [json, json.count]
        let count = inputs.count / 2
        let sig = try sign(signKey, storepass, count, inputs)
        _ = try verify(signKey, sig, count, inputs)

        let proof = Proof(signKey, sig)
        self.proof = proof
        return self
    }

    private func selectEntry<T: DIDObject>(_ entries: OrderedDictionary<DIDURL, T>, _ id: DIDURL?, _ type: String?) throws -> Array<T>  {
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
    
    private func  getEntries<T>(_ entries: OrderedDictionary<DIDURL, T>) -> Array<T> {
        var list: Array<T> = []
        
        entries.forEach{ (arg: (key: DIDURL, value: T)) in
            let (_, value) = arg
            list.append(value)
        }
        return list
    }
    
    private func getEntry<T: DIDObject>(_ entries: OrderedDictionary<DIDURL, T>, _ id: DIDURL) throws -> T? {
        let re = entries[id]
        return re
    }
    
    private func removeEntry(_ entriesType: String, _ id: DIDURL) -> Bool {
        if entriesType == "publicKeys" {
            return self.publicKeys.removeValueForKey(key: id)
        }
        else if entriesType == "authentications" {
            return self.authentications.removeValueForKey(key: id)
        }
        else if entriesType == "authorizations" {
            return self.authorizations.removeValueForKey(key: id)
        }
        else if entriesType == "credentials" {
            return self.credentials.removeValueForKey(key: id)
        }
        else {
            return self.services.removeValueForKey(key: id)
        }
    }
}
