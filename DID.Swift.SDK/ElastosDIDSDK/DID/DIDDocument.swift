import Foundation

public class DIDDocument: NSObject {
    public var subject: DID?
    private var publicKeys: Dictionary<DIDURL, PublicKey> = [: ]
    private var authentications: Dictionary<DIDURL, PublicKey> = [: ]
    private var authorizations: Dictionary<DIDURL, PublicKey> = [: ]
    private var credentials: Dictionary<DIDURL, VerifiableCredential> = [: ]
    private var services: Dictionary<DIDURL, Service> = [: ]
    public var expires: Date?

    public var readonly: Bool = false

    override init() {
        readonly = false
    }

    public func getPublicKeyCount() -> Int {
        return publicKeys.count
    }

    public func getPublicKeys() -> Array<PublicKey> {
        return getEntries(publicKeys)
    }

    public func selectPublicKey(_ id: String, _ type: String) throws -> Array<PublicKey> {
        var didurl: DIDURL
        do {
            didurl = try DIDURL(id)
        } catch {
            throw error
        }
        return try selectEntry(publicKeys, didurl, type)
    }

    public func selectPublicKey(_ id: DIDURL, _ type: String) throws -> Array<PublicKey> {
        do {
            return try selectEntry(publicKeys, id, type)
        } catch {
            throw error
        }
    }

    public func getPublicKey(_ id: String) throws -> PublicKey {
        var didurl: DIDURL
        do {
            didurl = try DIDURL(id)
        } catch {
            throw error
        }
        return getEntry(publicKeys, didurl)
    }

    public func getPublicKey(_ id: DIDURL) throws -> PublicKey {
        return getEntry(publicKeys, id)
    }

    public func getDefaultPublicKey() -> PublicKey? {
        publicKeys.values.forEach{ pk in
            if (pk.controller?.isEqual(self.subject))! {
                // TODO HDKey
            }
        }
        return nil
    }

    public func addPublicKey(_ pk: PublicKey) -> Bool{
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

    public func getAuthenticationKeys() -> Array<PublicKey> {
        var list: Array<PublicKey> = []
        authentications.forEach{ (key, value) in
            list.append(value)
        }
        return list
    }

    public func selectAuthenticationKey(_ id: DIDURL, type: String) throws -> Array<PublicKey> {
        return try selectEntry(authentications, id, type)
    }

    public func selectAuthenticationKey(_ id: String, _ type: String) throws -> Array<PublicKey>{
        let didurl = try DIDURL(id)
        return try selectEntry(authentications, didurl, type)
    }

    public func getAuthenticationKey(_ id: DIDURL) -> PublicKey {
        return getEntry(authentications, id)
    }
    // TODO error handle
    public func getAuthenticationKey(_ id: String) throws -> PublicKey {
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

    func addAuthenticationKey(_ pk: PublicKey) -> Bool {
        if readonly { return false }
        // Check the controller is current DID subject
        if !((pk.controller?.isEqual(subject))!) { return false }
        authentications[pk.id] = pk
        return true
    }

    public func getAuthorizationKeyCount() -> Int {
        return authentications.count
    }

    public func getAuthorizationKeys() -> Array<PublicKey> {
        return getEntries(authentications)
    }

    public func selectAuthorizationKey(_ id: DIDURL, _ type: String) throws -> Array<PublicKey> {
        return try selectEntry(authentications, id, type)
    }

    public func selectAuthorizationKey(_ id: String, _ type: String) throws -> Array<PublicKey> {
        return try selectEntry(authentications, DIDURL(id), type)
    }

    public func getAuthorizationKey(_ id: DIDURL) -> PublicKey {
        return getEntry(authentications, id)
    }

    public func getAuthorizationKey(_ id: String) throws -> PublicKey {
        return try getEntry(authentications, DIDURL(id))
    }

    public func addAuthorizationKey(_ id: String, _ did: DID, _ key: DIDURL) throws -> Bool {
        if readonly { return false }
        let doc: DIDDocument = try DIDStore.shareInstance().resolveDid(did)
        let refPk: PublicKey = try doc.getPublicKey(key)
        if !((refPk.controller?.isEqual(did))!) { return false }
        let pk = try PublicKey(DIDURL(subject!, id), refPk.type, did, refPk.keyBase58!)
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

    public static func fromJson(url: URL) throws -> DIDDocument {
        // url of local path
        let doc: DIDDocument = DIDDocument()
        try doc.parse(url: url)
        doc.readonly = true
        return doc
    }

    private func parse(url: URL) throws {
        let data = try Data(contentsOf: url)
        let jsonData:Any = try JSONSerialization.jsonObject(with: data, options: JSONSerialization.ReadingOptions.mutableContainers)
        return try parse(jsonData as! Dictionary<String, Any>)
    }

    private func parse(_ dic: Dictionary<String, Any>) throws {
        // TODO: set did

        // parse
        var mode = dic[Constants.publicKey]
        guard (mode != nil) else {
            // TODO: throw error
            return
        }
        try parsePublick(mode as Any)

        mode = dic[Constants.authentication]
        guard (mode != nil) else {
            // TODO: throw error
            return
        }
        try parseAuthentication(mode as Any)

        mode = dic[Constants.authorization]
        guard (mode != nil) else {
            // TODO: throw error
            return
        }
        try parseAuthorization(mode as Any)

        mode = dic[Constants.credential]
        guard (mode != nil) else {
            // TODO: throw error
            return
        }
        parseCredential(mode as Any)

        mode = dic[Constants.service]
        guard (mode != nil) else {
            // TODO: throw error
            return
        }
        parseService(mode as Any)

        // TODO: ERROR
        expires = JsonHelper.getDate(dic, Constants.EXPIRES, true, nil, "expires")
    }

    private func parsePublick(_ md: Any) throws {
        if !(md is Array<Any>) {
            // TODO: error
        }
        let arr = md as! Array<Dictionary<String, Any>>
        if arr.count == 0 {
            // TODO: error
        }

       try arr.forEach { (obj) in
            let pk: PublicKey = try PublicKey.fromJson(obj, subject!)
            _ = addPublicKey(pk)
        }
    }

    // MARK: parseAuthentication
    private func parseAuthentication(_ md: Any) throws {
        if !(md is Array<Any>) {
            // TODO: error
        }
        let arr = md as! Array<Dictionary<String, Any>>
        if arr.count == 0 {
            // TODO: error
        }

        try arr.forEach { (obj) in
            let pk: PublicKey = try PublicKey.fromJson(obj, subject!)
            _ = addAuthenticationKey(pk)
        }
    }

    private func parseAuthorization(_ md: Any) throws {
        if !(md is Array<Any>) {
            // TODO: error
        }
        let arr = md as! Array<Dictionary<String, Any>>
        if arr.count == 0 {
            // TODO: error
        }

        try arr.forEach { (obj) in
            let pk: PublicKey = try PublicKey.fromJson(obj, subject!)
            _ = addAuthorizationKey(pk)
        }
    }

    private func parseCredential(_ md: Any) {
        // TODO:
    }

    private func parseService(_ md: Any) {
        // TODO:
    }

    private func addService(_ svc: Service) -> Bool {
        if readonly  { return false }
        services[svc.id] = svc
        return true
    }
    private func addAuthorizationKey(_ pk: PublicKey) -> Bool {
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
    
    public func toExternalForm(_ compact: Bool) -> String {
        // todo
        return ""
    }

}
