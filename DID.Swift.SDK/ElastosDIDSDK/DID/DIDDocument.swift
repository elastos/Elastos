import Foundation

public class DIDDocument: NSObject {
    public var subject: DID?
    private var publicKeys: OrderedDictionary<DIDURL, DIDPublicKey> = OrderedDictionary()
    private var authentications: OrderedDictionary<DIDURL, DIDPublicKey> = OrderedDictionary()
    private var authorizations: OrderedDictionary<DIDURL, DIDPublicKey> = OrderedDictionary()
    private var credentials: OrderedDictionary<DIDURL, VerifiableCredential> = OrderedDictionary()
    private var services: OrderedDictionary<DIDURL, Service> = OrderedDictionary()
    public var expires: Date?
    
    public var readonly: Bool = false
    
    override init() {
        super.init()
        readonly = false
        setDefaultExpires()
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
        return getEntry(publicKeys, didurl)!
    }
    
    public func getPublicKey(_ id: DIDURL) throws -> DIDPublicKey? {
        return getEntry(publicKeys, id)
    }
    
    public func getDefaultPublicKey() -> DIDURL? {
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
        return didurl
    }
    
    public func addPublicKey(_ pk: DIDPublicKey) -> Bool {
        if readonly { return false }
        guard publicKeys[pk.id] != nil else {
            return false
        }
            
        publicKeys[pk.id] = pk
        return true
    }
    
    public func addPublicKey(_ id: DIDURL, _ controller: DID, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count != HDKey.PUBLICKEY_BYTES else {
            throw DIDError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, controller, pk)
        return addPublicKey(key)
    }
    
    public func addPublicKey(_ id: String, _ controller: String, _ pk: String) throws -> Bool {
        return try addPublicKey(DIDURL(id), DID(controller), pk)
    }
    
    public func removePublicKey(_ id: DIDURL, _ force: Bool) throws -> Bool {
        if readonly { return false }
        
        // Cann't remove default public key
        if (getDefaultPublicKey()?.isEqual(id))! { return false }
        if try isAuthenticationKey(id) {
            if force {
                _ = removeAuthenticationKey(id)
            }
            else {
                return false
            }
        }
        if try isAuthorizationKey(id) {
            if force {
                _ = removeAuthorizationKey(id)
            }
            else {
                return false
            }
        }
        let removed: Bool = removeEntry(publicKeys, id)
        
        if removed {
            do {
                try _ = DIDStore.shareInstance()?.deletePrivateKey(subject!, id)
            } catch {
             // TODO: CHECKME!
            }
        }
        return removed
    }

    public func removePublicKey(_ id: String, _ force: Bool) throws -> Bool {
        return try removePublicKey(DIDURL(id), force)
    }
    
    public func removePublicKey(_ id: DIDURL) throws -> Bool {
        return try removePublicKey(id, false)
    }
    
    public func removePublicKey(_ id: String) throws -> Bool {
        return try removePublicKey(DIDURL(id), false)
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
    
    public func getAuthenticationKey(_ id: DIDURL) -> DIDPublicKey? {
        return getEntry(authentications, id)
    }
    
    public func getAuthenticationKey(_ id: String) throws -> DIDPublicKey {
        return try getEntry(authentications, DIDURL(id))!
    }

    public func isAuthenticationKey(_ id: DIDURL) throws -> Bool {
        return getAuthenticationKey(id) != nil
    }
    
    public func isAuthenticationKey(_ id: String) throws -> Bool {
        return try isAuthenticationKey(DIDURL(id))
    }
    
    func addAuthenticationKey(_ pk: DIDPublicKey) -> Bool {
        // Check the controller is current DID subject
        guard !readonly && ((pk.controller?.isEqual(subject))!)  else {
            return false
        }
        // Confirm add the new pk to PublicKeys
        _ = addPublicKey(pk)
        guard authentications[pk.id] == nil else {
            return false
        }
        authentications[pk.id] = pk
        return true
    }
    
    public func addAuthenticationKey(_ id: DIDURL) throws -> Bool {
        let pk = try getPublicKey(id)
        if pk == nil {
            return false
        }
        return addAuthenticationKey(pk!)
    }
    
    public func addAuthenticationKey(_ id: String) throws -> Bool {
        return try addAuthenticationKey(DIDURL(id))
    }
    
    public func addAuthenticationKey(_ id: DIDURL, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, subject!, pk)
        return addAuthenticationKey(key)
    }
    
    public func addAuthenticationKey(_ id: String, _ pk: String) throws -> Bool {
        return try addAuthenticationKey(DIDURL(id), pk)
    }
                
    public func removeAuthenticationKey(_ id: DIDURL) -> Bool {
        guard !readonly else { return false }
        // Can not remove default public key
        if (getDefaultPublicKey()?.isEqual(id))! {
            return false
        }
        return removeEntry(authentications, id)
    }
    
    public func removeAuthenticationKey(_ id: String) throws -> Bool {
        return try removeAuthenticationKey(DIDURL(id))
    }

    public func getAuthorizationKeyCount() -> Int {
        return authorizations.count
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
    
    public func getAuthorizationKey(_ id: DIDURL) -> DIDPublicKey? {
        return getEntry(authentications, id)
    }
    
    public func getAuthorizationKey(_ id: String) throws -> DIDPublicKey {
        return try getEntry(authentications, DIDURL(id))!
    }

    public func isAuthorizationKey(_ id: DIDURL) throws -> Bool {
        return getAuthorizationKey(id) != nil
    }
    
    public func isAuthorizationKey(_ id: String) throws -> Bool {
        return try isAuthorizationKey(DIDURL(id))
    }
    
    public func addAuthorizationKey(_ pk: DIDPublicKey) throws -> Bool {
        if readonly { return false }
        // Can not authorize to self
        if (pk.controller?.isEqual(subject))! { return false }
        // Confirm add the new pk to PublicKeys
        _ = addPublicKey(pk)
        if authorizations[pk.id] != nil {
            return false
        }
        authorizations[pk.id] = pk
        return true
    }
    
    public func addAuthorizationKey(_ id: DIDURL) throws -> Bool {
        let pk = try getPublicKey(id)
        if pk == nil {
            return false
        }
        return try addAuthorizationKey(pk!)
    }
    
    public func addAuthorizationKey(_ id: String) throws -> Bool {
       return try addAuthorizationKey(DIDURL(id))
    }
    
    public func addAuthorizationKey(_ id: DIDURL, _ controller: DID, _ pk: String) throws -> Bool {
        guard Base58.bytesFromBase58(pk).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.failue("Invalid public key.")
        }
        let key: DIDPublicKey = DIDPublicKey(id, controller, pk)
        return try addAuthorizationKey(key)
    }
                
    public func addAuthorizationKey(_ id: String, _ controller: String, _ pk: String) throws -> Bool {
        return try addAuthorizationKey(DIDURL(id), DID(controller), pk)
    }
    
    public func authorizationDid(_ id: DIDURL, _ controller: DID, _ key: DIDURL?) throws -> Bool {
        // Can not authorize to self
        if controller.isEqual(subject) {
            return false
        }
        let doc = try controller.resolve()
        if doc == nil { return false }
        var k: DIDURL
        if key == nil {
            k = doc!.getDefaultPublicKey()!
        }else {
            k = key!
        }
        let refPk = try doc?.getPublicKey(k)
        
        if refPk == nil {
            return false
        }
        // The public key should belongs to controller
        if !(refPk?.controller?.isEqual(controller))! {
            return false
        }
        let pk: DIDPublicKey = DIDPublicKey(id, refPk!.type, controller, refPk!.keyBase58!)
        return try addAuthorizationKey(pk)
    }
    
    public func authorizationDid(_ id: DIDURL, _ controller: DID) throws -> Bool {
        return try authorizationDid(id, controller)
    }
    
    public func authorizationDid(_ id: String, _ controller: String, _ key: String?) throws -> Bool {
        if key != nil {
            return try authorizationDid(DIDURL(id), DID(controller), DIDURL(key!))
        }
        else {
            return try authorizationDid(DIDURL(id), DID(controller), nil)
        }
    }
    
    public func authorizationDid(_ id: String, _ controller: String) throws -> Bool {
        return try authorizationDid(id, controller, nil)
    }
                
    public func removeAuthorizationKey(_ id: DIDURL) -> Bool {
        guard !readonly else { return false }
        return removeEntry(authorizations, id)
    }
    
    public func removeAuthorizationKey(_ id: String) throws -> Bool {
        return try removeAuthorizationKey(DIDURL(id))
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
        return try getEntry(credentials, DIDURL(id))!
    }
    
    public func getCredential(_ id: DIDURL) throws -> VerifiableCredential {
        return getEntry(credentials, id)!
    }
    
    public func addCredential(_ vc: VerifiableCredential) -> Bool {
        guard !readonly else { return false }
        // Check the credential belongs to current DID.
        if vc.subject.id == subject {
            return false
        }
        if credentials[vc.id] != nil {
            return false
        }
        let ec: EmbeddedCredential = EmbeddedCredential(vc)
        credentials[ec.id] = ec
        return true
    }
    
    public func removeCredential(_ id: DIDURL) -> Bool {
        guard !readonly else { return false }
        return removeEntry(credentials, id)
    }
    
    public func removeCredential(_ id: String) throws -> Bool {
        
        return try removeCredential(DIDURL(id))
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
        return getEntry(services, id)!
    }
    
    public func getService(_ id: String) throws -> Service {
        return try getEntry(services, DIDURL(id))!
    }
    
    public func addService(_ svc: Service) throws -> Bool {
        if readonly {
            return false
        }
        if services[svc.id] != nil {
            return false
        }
        services[svc.id] = svc
        return true
    }
    
    public func addService(_ id: DIDURL, _ type: String, _ endpoint: String) throws -> Bool {
        let svc: Service = Service(id, type, endpoint)
        return try addService(svc)
    }
    
    public func addService(_ id: String, _ type: String, _ endpoint: String) throws -> Bool {
        return try addService(DIDURL(id), type, endpoint)
    }
    
    public func removeService(_ id: DIDURL) -> Bool {
        guard !readonly else { return false }
        return removeEntry(services, id)
    }
    
    public func removeService(_ id: String) throws -> Bool {
        return try removeService(DIDURL(id))
    }
    
    public func setDefaultExpires() {
        expires = DateFormater.currentDateToWantDate(Constants.MAX_VALID_YEARS)
    }
    
    public func setExpires(_ expiresDate: Date) -> Bool {
        guard !readonly else {
            return false
        }
        
        if DateFormater.comporsDate(expiresDate, expires!) {
            self.expires = DateFormater.setExpires(expiresDate)
            return true
        }
        
        return false
    }
    
    public func modify() -> Bool {
        readonly = false
        return true
    }
    
    public func toJson(_ path: String?, _ compact: Bool) throws -> String? {
        var dic: OrderedDictionary<String, Any> = OrderedDictionary()
        // subject
        dic[Constants.id] = subject?.toExternalForm()
        
        // publicKey
        publicKeys = DIDURLComparator.DIDOrderedDictionaryComparator(publicKeys)
        var pks: Array<OrderedDictionary<String, Any>> = []
        publicKeys.forEach { (didUrl, pk) in
            let dic = pk.toJson(subject!, compact)
            pks.append(dic)
        }
        dic[Constants.publicKey] = pks
        
        // authentication
        authentications = DIDURLComparator.DIDOrderedDictionaryComparator(authentications)
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
        if authorizations.count > 0 {
            authorizations = DIDURLComparator.DIDOrderedDictionaryComparator(authorizations)
            var authoriPks: Array<String> = []
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
        if credentials.count > 0 {
            credentials = DIDURLComparator.DIDOrderedDictionaryComparatorByVerifiableCredential(credentials)
            var vcs: Array<OrderedDictionary<String, Any>> = []
            credentials.forEach { (didUrl, vc) in
                let dic = vc.toJson(subject!, compact, false)
                vcs.append(dic)
            }
            dic[Constants.credential] = vcs
        }
        
        // service
        if services.count > 0 {
            services = DIDURLComparator.DIDOrderedDictionaryComparatorByService(services)
            var ser_s: Array<OrderedDictionary<String, Any>> = [ ]
            services.forEach { (_, service) in
                let dic = service.toJson(subject!, compact)
                ser_s.append(dic)
            }
            dic[Constants.service] = ser_s
        }
        
        // expires
        if expires != nil {
            dic[Constants.expires] = DateFormater.format(expires!)
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
    
    public class func fromJson(_ path: String) throws -> DIDDocument{
        
        let doc: DIDDocument = DIDDocument()
//        doc.rootPath = path
        let urlPath = URL(fileURLWithPath: path)
        try doc.parse(url: urlPath)
        doc.readonly = true
        return doc
    }
    
    public class func fromJson(_ path: URL) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        try doc.parse(url: path)
        doc.readonly = true
        return doc
    }
    
    public class func fromJson(json: String) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        try doc.parse(json: json)
        doc.readonly = true
        return doc
    }
    
    public func fromJson(_ dic: OrderedDictionary<String, Any>) throws -> DIDDocument {
        let doc: DIDDocument = DIDDocument()
        let ordDic: OrderedDictionary<String, Any> = dic
        try doc.parse(ordDic)
        return doc
    }
    
    private func parse(url: URL) throws {
        let json = try! String(contentsOf: url)
        var jsonString = json.replacingOccurrences(of: " ", with: "")
        jsonString = jsonString.replacingOccurrences(of: "\n", with: "")
        let ordDic = JsonHelper.handleString(jsonString) as! OrderedDictionary<String, Any>
        return try parse(ordDic)
    }
    
    private func parse(json: String) throws {
        var jsonString = json.replacingOccurrences(of: " ", with: "")
        jsonString = jsonString.replacingOccurrences(of: "\n", with: "")
        let ordDic = JsonHelper.handleString(jsonString) as! OrderedDictionary<String, Any>
        return try parse(ordDic)
    }
    
    private func parse(_ json: OrderedDictionary<String, Any>) throws {
        self.subject = try JsonHelper.getDid(json, Constants.id, false, nil, "subject")
        
        try parsePublicKey(json)
        try parseAuthentication(json)
        try parseAuthorization(json)
        try parseCredential(json)
        try parseService(json)
        expires = try DateFormater.getDate(json, Constants.expires, true, nil, "expires")
    }
    
    // 解析公钥
    private func parsePublicKey(_ json: OrderedDictionary<String, Any>) throws {
        let publicKeys = json["publicKey"] as? Array<Any>
        
        guard publicKeys != nil else {
            throw DIDError.failue("Invalid publicKey, should be an array.")
        }
        
        let publicKeysArray: [OrderedDictionary<String, Any>] = publicKeys as! Array<OrderedDictionary<String, Any>>
        guard publicKeysArray.count != 0 else {
            throw DIDError.failue("Invalid publicKey, should not be an empty array.")
        }
        
        for publicKey in publicKeysArray {
            let pk = try DIDPublicKey.fromJson(publicKey, self.subject!)
            _ = addPublicKey(pk)
        }
    }
    
    // MARK: parseAuthentication
    private func parseAuthentication(_ json: OrderedDictionary<String, Any>) throws {
        let authentications = json[Constants.authentication] as? Array<Any>

        guard (authentications != nil) else {
            throw DIDError.failue("Invalid authentication, should be an array.")
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
                    let id: String = json[Constants.id] as! String
                    didString = id + objString
                }
                let didUrl: DIDURL = try DIDURL(didString)
                pk = publicKeys[didUrl]!
            }
            _ = addAuthenticationKey(pk)
            _ = addPublicKey(pk)
        }
    }
    
    private func parseAuthorization(_ json: OrderedDictionary<String, Any>) throws {
        let aus = json[Constants.authorization]
        guard (aus != nil) else {
            return
        }
        let authorizations = aus as? Array<Any>
        guard (authorizations != nil) else {
            throw DIDError.failue("Invalid authorization, should be an array.")
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
                let objString: String = obj as! String
                let didUrl: DIDURL = try DIDURL(objString)
                pk = publicKeys[didUrl]!
            }
            _ = try addAuthorizationKey(pk)
        }
    }
    
    // mode parse
    private func parseCredential(_ json: OrderedDictionary<String, Any>) throws {
        let crs = json[Constants.credential]
        guard (crs != nil) else {
            return
        }
        let credentials = crs as? Array<OrderedDictionary<String, Any>>
        guard (credentials != nil) else {
            throw DIDError.failue("Invalid credential, should be an array.")
        }
        guard credentials!.count != 0  else {
            return
        }
        
        try credentials!.forEach{ obj in
            let vc: VerifiableCredential = try VerifiableCredential.fromJson(obj, subject!)
            _ = addCredential(vc)
        }
    }
    
    private func parseService(_ json: OrderedDictionary<String, Any>) throws {
        let ses = json[Constants.service]
        guard (ses != nil) else {
            return
        }
        let services = ses as? Array<OrderedDictionary<String, Any>>
        guard (services != nil) else {
            throw DIDError.failue("Invalid services, should be an array.")
        }
        guard services!.count != 0  else {
            return
        }
        
        try services!.forEach { obj in
            let svc: Service = try Service.fromJson(obj, subject!)
            _ = try addService(svc)
        }
    }
    
    private func selectEntry<T: DIDObject>(_ entries: OrderedDictionary<DIDURL, T>, _ id: DIDURL, _ type: String) throws -> Array<T>  {
        var list: Array<T> = []
        entries.values.forEach { entry in
            if entry.id.isEqual(id) && !entry.type.isEmpty && entry.type.isEqual(type) {
                list.append(entry)
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
    
    private func getEntry<T: DIDObject>(_ entries: OrderedDictionary<DIDURL, T>, _ id: DIDURL) -> T? {
        return entries[id]
    }
    
    private func removeEntry<T: DIDObject>(_ publickeys: OrderedDictionary<DIDURL, T>, _ id: DIDURL) -> Bool {
        if publickeys.keys.count == 0 { return false }
        return self.publicKeys.removeValueForKey(key: id)
    }
    
    public func toExternalForm(_ compact: Bool) throws -> String {
        return try (toJson(nil, compact) ?? "")
    }
    
    public func sign(_ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        let key: DIDURL = getDefaultPublicKey()!
        return try sign(key, storepass, count, inputs)
    }
    
    public func sign(_ id: DIDURL, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        return try (DIDStore.shareInstance()?.sign(subject!, id, storepass, count, inputs))!
    }
    
    public func sign(_ id: String, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        
        return try (DIDStore.shareInstance()?.sign(subject!, DIDURL(id), storepass, count, inputs))!
    }
    
    public func verify(_ signature: String, _ count: Int, _ inputs: [CVarArg]) throws -> Bool {
        let key: DIDURL = getDefaultPublicKey()!
        return try verify(key, signature, count, inputs)
    }
    
    public func verify(_ id: String, _ signature: String, _ count: Int, _ inputs: [CVarArg]) throws -> Bool {
        return try verify(DIDURL(id), signature, count, inputs)
    }
    
    public func verify(_ id: DIDURL, _ signature: String, _ count: Int, _ inputs: [CVarArg]) throws -> Bool {
        var cinputs: [CVarArg] = []
        for i in 0..<inputs.count {
            if (i % 2) == 0 {
                let json: String = inputs[i] as! String
                let cjson = json.withCString { re -> UnsafePointer<Int8> in
                    return re
                }
                cinputs.append(cjson)
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
        let csignature = signature.withCString { re -> UnsafeMutablePointer<Int8> in
            let mre = UnsafeMutablePointer<Int8>.init(mutating: re)
            return mre
        }
        let c_inputs = getVaList(cinputs)
        let count = inputs.count / 2
       let re = ecdsa_verify_base64v(csignature, cpk, Int32(count), c_inputs)
        
        return re == 0 ? true : false
    }
}
