import Foundation

public class DIDDocument: NSObject {
    public var subject: DID?
    private var publicKeys: Dictionary<DIDURL, DIDPublicKey> = [: ]
    private var authentications: Dictionary<DIDURL, DIDPublicKey> = [: ]
    private var authorizations: Dictionary<DIDURL, DIDPublicKey> = [: ]
    private var credentials: Dictionary<DIDURL, VerifiableCredential> = [: ]
    private var services: Dictionary<DIDURL, Service> = [: ]
    public var expires: Date?
    private var rootPath: String?
    
    public var readonly: Bool = false
    
    override init() {
        readonly = false
    }
    
    public func getPublicKeyCount() -> Int {
        return publicKeys.count
    }
    
    public func getPublicKeys() -> Array<DIDPublicKey> {
        return getEntries(publicKeys)
    }
    
    public func selectPublicKey(_ id: String, _ type: String) throws -> Array<DIDPublicKey> {
        var didurl: DIDURL
        do {
            didurl = try DIDURL(id)
        } catch {
            throw error
        }
        return try selectEntry(publicKeys, didurl, type)
    }
    
    public func selectPublicKey(_ id: DIDURL, _ type: String) throws -> Array<DIDPublicKey> {
        do {
            return try selectEntry(publicKeys, id, type)
        } catch {
            throw error
        }
    }
    
    public func getPublicKey(_ id: String) throws -> DIDPublicKey {
        var didurl: DIDURL
        do {
            didurl = try DIDURL(id)
        } catch {
            throw error
        }
        return getEntry(publicKeys, didurl)
    }
    
    public func getPublicKey(_ id: DIDURL) throws -> DIDPublicKey {
        return getEntry(publicKeys, id)
    }
    
    public func getDefaultPublicKey() -> DIDPublicKey? {
        var didpk: DIDPublicKey?
        publicKeys.values.forEach{ pk in
            if (pk.controller?.isEqual(self.subject))! {
                
                let pks = pk.getPublicKeyBytes()
                let idstring = DerivedKey.getIdString(pks)
                if idstring == subject?.methodSpecificId {
                    didpk = pk
                }
            }
        }
        return didpk
    }
    
    public func addPublicKey(_ pk: DIDPublicKey) -> Bool{
        if readonly { return false }
        publicKeys[pk.id] = pk
        return true
    }
    
    public func addPublicKey(_ id: String) -> Bool {
        // TODO generate a new key pair, and add it to document
        return false
    }
    
    // TODO: Add an exiting key to document!
    
    public func removePublicKey(_ id: DIDURL) -> Bool {
        if readonly { return false }
        
        // Cann't remove default public key
        if (getDefaultPublicKey()?.id.isEqual(id))! { return false }
        let removed: Bool = removeEntry(publicKeys, id)
        
        if removed {
            // TODO: remove private key if exist
        }
        return removed
    }
    
    public func getAuthenticationKeyCount() -> Int {
        return authentications.count
    }
    
    public func getAuthenticationKeys() -> Array<DIDPublicKey> {
        var list: Array<DIDPublicKey> = []
        authentications.forEach{ (key, value) in
            list.append(value)
        }
        return list
    }
    
    public func selectAuthenticationKey(_ id: DIDURL, type: String) throws -> Array<DIDPublicKey> {
        return try selectEntry(authentications, id, type)
    }
    
    public func selectAuthenticationKey(_ id: String, _ type: String) throws -> Array<DIDPublicKey>{
        let didurl = try DIDURL(id)
        return try selectEntry(authentications, didurl, type)
    }
    
    public func getAuthenticationKey(_ id: DIDURL) -> DIDPublicKey {
        return getEntry(authentications, id)
    }
    // TODO error handle
    public func getAuthenticationKey(_ id: String) throws -> DIDPublicKey {
        return try getEntry(authentications, DIDURL(id))
    }
    
    public func removeAuthenticationKey(_ id: DIDURL) -> Bool {
        if readonly { return false }
        return removeEntry(authentications, id)
    }
    
    public func addAuthenticationKey(_ id: DIDURL) throws -> Bool {
        if readonly { return false }
        let pk = try getPublicKey(id)
        return addAuthenticationKey(pk)
    }
    
    func addAuthenticationKey(_ pk: DIDPublicKey) -> Bool {
        if readonly { return false }
        // Check the controller is current DID subject
        if !((pk.controller?.isEqual(subject))!) { return false }
        authentications[pk.id] = pk
        return true
    }
    
    public func getAuthorizationKeyCount() -> Int {
        return authentications.count
    }
    
    public func getAuthorizationKeys() -> Array<DIDPublicKey> {
        return getEntries(authentications)
    }
    
    public func selectAuthorizationKey(_ id: DIDURL, _ type: String) throws -> Array<DIDPublicKey> {
        return try selectEntry(authentications, id, type)
    }
    
    public func selectAuthorizationKey(_ id: String, _ type: String) throws -> Array<DIDPublicKey> {
        return try selectEntry(authentications, DIDURL(id), type)
    }
    
    public func getAuthorizationKey(_ id: DIDURL) -> DIDPublicKey {
        return getEntry(authentications, id)
    }
    
    public func getAuthorizationKey(_ id: String) throws -> DIDPublicKey {
        return try getEntry(authentications, DIDURL(id))
    }
    
    public func addAuthorizationKey(_ id: String, _ did: DID, _ key: DIDURL) throws -> Bool {
        if readonly { return false }
        let doc: DIDDocument = try DIDStore.shareInstance()!.resolveDid(did)
        let refPk: DIDPublicKey = try doc.getPublicKey(key)
        if !((refPk.controller?.isEqual(did))!) { return false }
        let pk = try DIDPublicKey(DIDURL(subject!, id), refPk.type, did, refPk.keyBase58!)
        _ = addPublicKey(pk)
        return addAuthorizationKey(pk)
    }
    
    public func removeAuthorizationKey(_ id: DIDURL) -> Bool {
        guard !readonly else { return false }
        return removeEntry(authorizations, id)
    }
    
    public func getCredentialCount() -> Int {
        return credentials.count
    }
    
    public func getCredentials() -> Array<VerifiableCredential> {
        return getEntries(credentials)
    }
    
    public func selectCredential(_ id: DIDURL, _ type: String) throws -> Array<VerifiableCredential> {
        return try selectEntry(credentials, id, type)
    }
    
    public func selectCredential(_ id: String, _ type: String) throws -> Array<VerifiableCredential> {
        return try selectEntry(credentials, DIDURL(id), type)
    }
    
    public func getCredential(_ id: String) throws -> VerifiableCredential {
        return try getEntry(credentials, DIDURL(id))
    }
    
    public func getCredential(_ id: DIDURL) throws -> VerifiableCredential {
        return getEntry(credentials, id)
    }
    
    public func addCredential(_ vc: VerifiableCredential) -> Bool {
        if readonly { return false }
        let ec: EmbeddedCredential = EmbeddedCredential(vc)
        credentials[ec.id] = ec
        return true
    }
    
    public func removeCredential(_ id: DIDURL) -> Bool {
        if readonly { return false }
        return removeEntry(credentials, id)
    }
    
    public func getServiceCount() -> Int {
        return services.count
    }
    
    public func getServices() -> Array<Service> {
        return getEntries(services)
    }
    
    public func selectServices(_ id: DIDURL, _ type: String) throws -> Array<Service> {
        return try selectEntry(services, id, type)
    }
    
    public func selectServices(_ id: String, _ type: String) throws -> Array<Service> {
        return try selectEntry(services, DIDURL(id), type)
    }
    
    public func getService(_ id: DIDURL) -> Service {
        return getEntry(services, id)
    }
    
    public func getService(_ id: String) throws -> Service {
        return try getEntry(services, DIDURL(id))
    }
    
    public func addService(_ id: String, _ type: String, endpoint: String) throws -> Bool {
        let idurl: DIDURL = try DIDURL(subject!, id)
        let svc: Service = Service(idurl, type, endpoint)
        return addService(svc)
    }
    
    public func removeService(_ id: DIDURL) -> Bool {
        if readonly { return false }
        return removeEntry(services, id)
    }
    
    public func modify() -> Bool {
        // TODO: Check owner
        readonly = false
        return true
    }
    
    public func toJson(_ path: String, _ compact: Bool) throws {
        self.rootPath = path
        var dic: Dictionary<String, Any> = [: ]
        // subject
        dic[Constants.id] = subject?.toExternalForm()
        
        // publicKey
        var pks: Array<Dictionary<String, Any>> = []
        publicKeys.forEach { (didUrl, pk) in
            let dic = pk.toJson(subject!, compact)
            pks.append(dic)
        }
        dic[Constants.publicKey] = pks
        
        // authentication
        var authenPKs: Array<String> = []
        authentications.forEach { (didUrl, pk) in
            var value: String
            if compact && pk.id.did.isEqual(subject){
                value = "#" + pk.id.fragment!
            }
            else {
                value = pk.id.toExternalForm()
            }
            authenPKs.append(value)
        }
        dic[Constants.authentication] = authenPKs
        
        // authorization
        var authoriPks: Array<String> = []
        if !authorizations.isEmpty && authorizations.count != 0 {
            authorizations.forEach { (didUrl, pk) in
                var value: String
                if compact && pk.id.did.isEqual(subject) {
                    value = "#" + pk.id.fragment!
                }else {
                    value = pk.id.toExternalForm()
                }
                authoriPks.append(value)
            }
            dic[Constants.authorization] = authoriPks
        }
        
        // credential
        if !credentials.isEmpty && credentials.count != 0 {
            var vcs: Array<Dictionary<String, Any>> = []
            credentials.forEach { (didUrl, vc) in
                let dic = vc.toJson(subject!, compact, false)
                vcs.append(dic)
            }
            dic[Constants.credential] = vcs
        }
        
        // service
        if !services.isEmpty {
            var ser_s: Array<Dictionary<String, Any>> = [ ]
            services.forEach { (didUrl, service) in
                let dic = service.toJson(subject!, compact)
                ser_s.append(dic)
            }
            dic[Constants.service] = ser_s
        }
        
        // expires
//        dic[Constants.EXPIRES] = JsonHelper.format(expires!)
        
        // Change to jsonSting
        if (!JSONSerialization.isValidJSONObject(dic)) {
            // TODO: throws error
        }
        let data: Data = try JSONSerialization.data(withJSONObject: dic, options: [])
        
        // & Write to local
        let dirPath: String = PathExtracter(path).dirNamePart()
        let fileM = FileManager.default
        let re = fileM.fileExists(atPath: dirPath)
        if !re {
            try fileM.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
        }
        let dre = fileM.fileExists(atPath: path)
        if !dre {
            fileM.createFile(atPath: path, contents: nil, attributes: nil)
        }
        let writeHandle = FileHandle(forWritingAtPath: path)
        writeHandle?.write(data)
    }
    
    public static func fromJson(_ path: String) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        let urlPath = URL(fileURLWithPath: path)
        try doc.parse(url: urlPath)
        doc.readonly = true
        return doc
    }
    
    public static func fromJson(_ path: URL) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        try doc.parse(url: path)
        doc.readonly = true
        return doc
    }
    
    private func parse(url: URL) throws {
        let json = try! String(contentsOf: url)
        let data = json.data(using: .utf8)!
        let output = try JSONSerialization.jsonObject(with: data, options: .allowFragments) as? [String:Any]
        return try parse(output!)
    }
    
    private func parse(_ json: Dictionary<String, Any>) throws {
        self.subject = try JsonHelper.getDid(json, Constants.id, false, nil, "subject")
        
        try parsePublicKey(json)
        try parseAuthentication(json)
        try parseAuthorization(json)
        try parseCredential(json)
        try parseService(json)
        expires = JsonHelper.getDate(json, Constants.expires, true, nil, "expires")
    }
    
    // 解析公钥
    private func parsePublicKey(_ json: [String: Any]) throws {
        let publicKeys = json["publicKey"]
        
        if publicKeys is Array<Any>{
            
        } else {
            throw DIDError.failue("Invalid publicKey, should be an array.")
        }
        
        let publicKeysArray: [[String: Any]] = publicKeys as! Array<[String: Any]>
        
        if publicKeysArray.count == 0 {
            throw DIDError.failue("Invalid publicKey, should not be an empty array.")
        }
        
        self.publicKeys = [:]
        for publicKey in publicKeysArray {
            
            let pk = try DIDPublicKey.fromJson(publicKey, self.subject!)
            _ = addPublicKey(pk)
        }
    }
    
    // MARK: parseAuthentication
    private func parseAuthentication(_ json: [String: Any]) throws {
        let authentications = json[Constants.authentication] as? Array<Any>

        if !(authentications != nil) {
            throw DIDError.failue("Invalid authentication, should be an array.")
        }

        if authentications!.count == 0 {
            return
        }
        
        try authentications!.forEach { (obj) in
            var pk: DIDPublicKey
            if obj is Dictionary<String, Any> {
                let object: Dictionary<String, Any> = obj as! Dictionary<String, Any>
                pk = try DIDPublicKey.fromJson(object, subject!)
            }else {
                let objString: String = obj as! String
                let didUrl: DIDURL = try DIDURL(objString)
                pk = publicKeys[didUrl]!
            }
            _ = addAuthenticationKey(pk)
        }
    }
    
    private func parseAuthorization(_ json: [String: Any]) throws {
        let authorizations = json[Constants.authorization] as? Array<Any>
        if (authorizations == nil) {
            throw DIDError.failue("Invalid authorization, should be an array.")
        }
        if authorizations!.count == 0 {
            return
        }
        try authorizations!.forEach { (obj) in
            var pk: DIDPublicKey
            if obj is Dictionary<String, Any> {
                let object: Dictionary<String, Any> = obj as! Dictionary<String, Any>
                pk = try DIDPublicKey.fromJson(object, subject!)
            }else {
                let objString: String = obj as! String
                let didUrl: DIDURL = try DIDURL(objString)
                pk = publicKeys[didUrl]!
            }
            _ = addAuthorizationKey(pk)
        }
    }
    
    // mode parse
    private func parseCredential(_ json: [String: Any]) throws {
        let credentials = json[Constants.credential] as? Array<Dictionary<String, Any>>
        if (credentials == nil) {
            throw DIDError.failue("Invalid credentials, should be an array.")
        }
        if credentials!.count == 0 {
            return
        }
        try credentials!.forEach{ obj in
            let vc: VerifiableCredential = try VerifiableCredential.fromJson(obj, subject!)
            _ = addCredential(vc)
        }
    }
    
    private func parseService(_ json: [String: Any]) throws {
        let services = json[Constants.service] as? Array<Dictionary<String, Any>>
        if (services == nil) {
            throw DIDError.failue("Invalid services, should be an array.")
        }
        if services!.count == 0 {
            return
        }
        try services!.forEach { obj in
            let svc: Service = try Service.fromJson(obj, subject!)
            _ = addService(svc)
        }
    }
    
    private func addService(_ svc: Service) -> Bool {
        if readonly  { return false }
        services[svc.id] = svc
        return true
    }
    private func addAuthorizationKey(_ pk: DIDPublicKey) -> Bool {
        if readonly { return false }
        // Cann' authorize to self
        
        guard pk.controller != nil else { return false }
        if (pk.controller?.isEqual(subject))! { return false }
        authorizations[pk.id] = pk
        return true
    }
    
    private func selectEntry<T: DIDObject>(_ entries: Dictionary<DIDURL, T>, _ id: DIDURL, _ type: String) throws -> Array<T>  {
        var list: Array<T> = []
        entries.values.forEach { entry in
            if entry.id.isEqual(id) && !entry.type.isEmpty && entry.type.isEqual(type) {
                list.append(entry)
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
    
    private func getEntry<T: DIDObject>(_ entries: Dictionary<DIDURL, T>, _ id: DIDURL) -> T {
        return entries[id]!
    }
    
    private func removeEntry<T: DIDObject>(_ publickeys: Dictionary<DIDURL, T>, _ id: DIDURL) -> Bool {
        if publickeys.isEmpty { return false }
        return (self.publicKeys.removeValue(forKey: id) != nil)
    }
    
    public func toExternalForm(_ compact: Bool) throws -> String {
        try toJson(self.rootPath!, compact)
        let urlPath = URL(fileURLWithPath: self.rootPath!)
        let json = try String(contentsOf: urlPath)
        return json
    }
    
}
