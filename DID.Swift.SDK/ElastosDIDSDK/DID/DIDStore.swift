import Foundation

public class DIDStore: NSObject {
    public static let CACHE_INITIAL_CAPACITY = 16
    public static let CACHE_MAX_CAPACITY = 32
    
    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2
    
    private static var instance: DIDStore!
    private var didCache: LRUCache!
    private var vcCache: LRUCache!
    
    private var backend: DIDBackend!
    private var storage: DIDStoreBackend!

    init(_ initialCacheCapacity: Int, _ maxCacheCapacity: Int, _ adapter: DIDAdapter, _ storage: DIDStoreBackend) {
        if (maxCacheCapacity > 0) {
            self.didCache = LRUCache(maxCacheCapacity)
            self.vcCache = LRUCache(maxCacheCapacity)
        }

        self.backend = DIDBackend(adapter)
        self.storage = storage
    }

    public static func creatInstance(_ type: String, _ location: String, _ adapter: DIDAdapter, _ initialCacheCapacity: Int, _ maxCacheCapacity: Int) throws {
        guard type == "filesystem" else {
            throw DIDStoreError.failue("Unsupported store type:\(type)")
        }

        if instance == nil {
            let storage = try FileSystemStoreBackend(location)
            instance = DIDStore(initialCacheCapacity, maxCacheCapacity,
            adapter, storage)
        }
    }
    
    public static func creatInstance(_ type: String, _ location: String, _ adapter: DIDAdapter) throws {
        try creatInstance(type, location, adapter, CACHE_INITIAL_CAPACITY, CACHE_MAX_CAPACITY)
    }

    public static func shareInstance() throws -> DIDStore? {
        guard let _ = instance else {
            throw DIDStoreError.failue("Please call the creatInstance first.")
        }
        return instance
    }
    
    public func getAdapter() -> DIDAdapter {
        return backend.adapter
    }
    
    public static func isInitialized() -> Bool {
        return instance != nil
    }

    public func containsPrivateIdentity() throws -> Bool {
        return try storage.containsPrivateIdentity()
    }
    
    func encryptToBase64(_ passwd: String ,_ input: Data) throws -> String {
        let cinput: UnsafePointer<UInt8> = input.withUnsafeBytes{ (by: UnsafePointer<UInt8>) -> UnsafePointer<UInt8> in
            return by
        }
        let base64url: UnsafeMutablePointer<Int8> = UnsafeMutablePointer.allocate(capacity: 108)
          let re = encrypt_to_base64(base64url, passwd, cinput, input.count)
        guard re >= 0 else {
            throw DIDStoreError.failue("encryptToBase64 error.")
        }
        return String(cString: base64url)
    }
    
    private func decryptFromBase64(_ passwd: String ,_ input: String) throws -> UnsafeMutablePointer<UInt8>  {
        let plain: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 108)
        let re = decrypt_from_base64(plain, passwd, input)
        guard re >= 0 else {
            throw DIDStoreError.failue("decryptFromBase64 error.")
        }
        return plain
    }
    
    private func decryptFromBase64(_ passwd: String ,_ input: String) throws -> Data {
        let plain: UnsafeMutablePointer<UInt8> = UnsafeMutablePointer<UInt8>.allocate(capacity: 108)
        let re = decrypt_from_base64(plain, passwd, input)
        guard re >= 0 else {
            throw DIDStoreError.failue("decryptFromBase64 error.")
        }
        let data = Data(bytes: plain, count: 64)
        return data
    }
    // Initialize & create new private identity and save it to DIDStore.
    public func initPrivateIdentity(_ language: Int,_ mnemonic: String ,_ passphrase: String, _ storepass: String, _ force: Bool ) throws {
        // TODO: CHECK mnemonic isValid
        
        if (try containsPrivateIdentity() && !force) {
            throw DIDStoreError.failue("Already has private indentity.")
        }
        let privateIdentity: HDKey = try HDKey.fromMnemonic(mnemonic, passphrase)

        // Save seed instead of root private key,
        // keep compatible with Native SDK
        let seedData = privateIdentity.getSeed()
        let encryptedIdentity = try encryptToBase64(storepass, seedData)
        try storage.storePrivateIdentity(encryptedIdentity)
        try storage.storePrivateIdentityIndex(0)
    }
    
    func initPrivateIdentity(_ language: Int, _ mnemonic: String, _ passphrase: String, _ storepass: String) throws -> Void {
        try initPrivateIdentity(language, mnemonic, passphrase, storepass, false)
    }
    
    func loadPrivateIdentity(_ storepass: String) throws -> HDKey {
        guard try containsPrivateIdentity() else {
            throw DIDStoreError.failue("DID Store not contains private identity.")
        }
        let seed: Data = try decryptFromBase64(storepass, storage.loadPrivateIdentity())
        return HDKey.fromSeed(seed)
    }

    public func synchronize(_ storepass: String) throws {
        let privateIdentity: HDKey = try loadPrivateIdentity(storepass)
        let nextIndex: Int = try storage.loadPrivateIdentityIndex()
        var blanks: Int = 0
        var i: Int = 0
        
        while i < nextIndex || blanks < 10 {
            let key: DerivedKey = try privateIdentity.derive(i++)
            let pks: [UInt8] = try key.getPublicKeyBytes()
            let methodIdString: String = DerivedKey.getIdString(pks)
            let did: DID = DID(DID.METHOD, methodIdString)
            
            let doc = try backend.resolve(did)
            if (doc != nil) {
                // TODO: check local conflict
                try storeDid(doc!)
                
                // Save private key
                let privatekeyData: Data = try key.getPrivateKeyData()
                let encryptedKey: String = try encryptToBase64(storepass, privatekeyData)
                try storePrivateKey(did, doc!.getDefaultPublicKey(), encryptedKey, storepass)
                if (i >= nextIndex){
                    try storage.storePrivateIdentityIndex(i)
                }
                blanks = 0
            } else {
                blanks = blanks + 1
            }
        }
    }
    
    public func newDid(_ storepass:String, _ alias: String?) throws -> DIDDocument {
        let privateIdentity =  try loadPrivateIdentity(storepass)
        var nextIndex: Int = try storage.loadPrivateIdentityIndex()

        let key: DerivedKey = try! privateIdentity.derive(nextIndex++)
        let pks: [UInt8] = try key.getPublicKeyBytes()
        let methodIdString: String = DerivedKey.getIdString(pks)
        let did: DID = DID(DID.METHOD, methodIdString)
        let id: DIDURL = try DIDURL(did, "primary")
        let privatekeyData: Data = try key.getPrivateKeyData()
        let encryptedKey = try encryptToBase64(storepass, privatekeyData)
        // TODO: get real private key bytes
        try storePrivateKey(did, id, encryptedKey, storepass)
        
        var doc: DIDDocument = DIDDocument(did)
        _ = try doc.addAuthenticationKey(id, try key.getPublicKeyBase58())
        
        doc = try doc.seal(storepass)
        try storeDid(doc, alias!)
        
        try storage.storePrivateIdentityIndex(nextIndex)
        
        return doc
    }
    
    public func newDid(_ storepass: String) throws -> DIDDocument {
        return try newDid(storepass, nil)
    }
    
    public func publishDid(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> Bool {
        return try backend.create(doc, signKey, storepass)
    }

    public func publishDid(_ doc: DIDDocument, _ signKey: String, _ storepass :String) throws -> Bool {
        return try publishDid(doc, DIDURL(doc.subject!, signKey), storepass)
    }
    
    public func publishDid(_ doc: DIDDocument, _ storepass :String) throws -> Bool {
        let signKey: DIDURL = doc.getDefaultPublicKey()
        return try backend.create(doc, signKey, storepass)
    }
    
    public func updateDid(_ doc: DIDDocument,_ signKey: DIDURL,_ storepass :String) throws -> Bool {
        return try backend.update(doc, signKey, storepass)
    }
    
    public func updateDid(_ doc: DIDDocument,_ signKey: String,_ storepass :String) throws -> Bool {
        return try updateDid(doc, DIDURL(doc.subject!, signKey), storepass)
    }
    
    public func updateDid(_ doc: DIDDocument, _ storepass :String) throws -> Bool {
        let signKey: DIDURL = doc.getDefaultPublicKey()
        return try backend.update(doc, signKey, storepass)
    }
    
    public func deactivateDid(_ did: DID,_ signKey: DIDURL, _ storepass :String) throws -> Bool {
        // TODO: how to handle locally?
        return try backend.deactivate(did, signKey, storepass)
    }
    
    public func deactivateDid(_ did: DID,_ signKey: String, _ storepass :String) throws -> Bool {
        return try deactivateDid(did, DIDURL(did, signKey), storepass)
    }
    
    public func deactivateDid(_ did: DID, _ storepass :String) throws -> Bool {
        do {
            let doc = try resolveDid(did)
            let signKey = doc!.getDefaultPublicKey()
            return try backend.deactivate(did, signKey, storepass)
        } catch {
            throw DIDError.failue("Can not resolve DID document.")
        }
    }
    
    public func resolveDid(_ did: DID, _ force: Bool) throws -> DIDDocument? {
        var doc = try backend.resolve(did)
        if doc !== nil {
            try storeDid(doc!)
        }
        if (!force){
                doc = try loadDid(did)
            }
        return doc
    }
    
    public func resolveDid(_ did: String, _ force: Bool) throws -> DIDDocument? {
        return try resolveDid(DID(did), force)
    }
    
    public func resolveDid(_ did: DID) throws -> DIDDocument? {
        return try resolveDid(did, false)
    }
    
    public func resolveDid(_ did: String) throws -> DIDDocument? {
        return try resolveDid(DID(did), false)
    }
    
    public func storeDid(_ doc: DIDDocument, _ alias: String) throws {
        try storeDid(doc)
        try storeDidAlias(doc.subject!, alias)
    }
    
    public func storeDid(_ doc: DIDDocument) throws {
        try storage.storeDid(doc)
        let count = didCache.getCount()
        if count != 0 {
            didCache.put(doc.subject!, data: doc)
        }
    }

    public func storeDidAlias(_ did: DID, _ alias: String) throws {
        try storage.storeDidAlias(did, alias)
        let doc = didCache.get(did)
        if (doc != nil) {
           let d = doc as! DIDDocument
            d.alias = alias
        }
    }
    
    public func storeDidAlias(_ did: String, _ alias: String) throws {
        try storeDidAlias(DID(did), alias)
    }

    public func loadDidAlias(_ did: DID) throws -> String {
        
        let doc = didCache.get(did)
        var al: String = ""
        if doc != nil {
            let d = doc as! DIDDocument
            al = d.alias
            return al
        }
        al = try storage.loadDidAlias(did)
        if (doc != nil) {
            let d = doc as! DIDDocument
            d.alias = al
        }
        return al
    }

    public func loadDidAlias(_ did: String) throws -> String {
        return try loadDidAlias(DID(did))
    }
    
    public func loadDid(_ did: DID) throws -> DIDDocument {
        var doc = didCache.get(did)
        if doc != nil {
            let d = doc as! DIDDocument
            return d
        }
        
        doc = try storage.loadDid(did)
        if (doc != nil) {
            let d = doc as! DIDDocument
            didCache.put(d.subject!, data: d)
        }
        let d = doc as! DIDDocument
        return d
    }

    public func loadDid(_ did: String) throws -> DIDDocument {
        return try loadDid(DID(did))
    }
    
    public func containsDid(_ did: DID) throws -> Bool {
        return try storage.containsDid(did)
    }

    public func containsDid(_ did: String) throws -> Bool {
        return try containsDid(DID(did))
    }

    public func deleteDid(_ did: DID) throws -> Bool {
        return try storage.deleteDid(did)
    }

    public func deleteDid(_ did: String) throws -> Bool {
        return try deleteDid(DID(did))
    }

    public func listDids(_ filter: Int) throws -> Array<DID> {
        return try storage.listDids(filter)
    }
    
    public func storeCredential(_ credential: VerifiableCredential, _ alias: String) throws {
        try storeCredential(credential)
        try storeCredentialAlias(credential.subject.id, credential.id, alias)
    }
    
    public func storeCredential(_ credential: VerifiableCredential ) throws {
        try storage.storeCredential(credential)
        vcCache.put(credential.id, data: credential)
    }
    
    public func storeCredentialAlias(_ did: DID, _ id: DIDURL, _ alias: String) throws {
        try storage.storeCredentialAlias(did, id, alias)
        let count = vcCache.getCount()
        if count != 0 {
            let vc = vcCache.get(id)
            if vc != nil {
                let v = vc as! VerifiableCredential
                v.alias = alias
            }
        }
    }
    
    public func storeCredentialAlias(_ did: String, _ id: String, _ alias: String) throws {
        let _did = try DID(did)
        try storeCredentialAlias(_did, try DIDURL(_did, id), alias)
    }
    
    public func loadCredentialAlias(_ did: DID, _ id: DIDURL) throws -> String {
        
        var al: String = ""
        let vc = vcCache.get(id)
        let count = vcCache.getCount()
        if count != 0 {
            if vc != nil {
                let v = vc as! VerifiableCredential
                al = v.alias
                return al
            }
        }
        al = try storage.loadCredentialAlias(did, id)
        if vc != nil {
            let v = vc as! VerifiableCredential
            v.alias = al
        }
        return al
    }
    
    public func loadCredentialAlias(_ did: String, _ id: String) throws -> String {
        let _did = try DID(did)
        return try loadCredentialAlias(_did, DIDURL(_did, id))
    }
    
    
    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential {
        var vc = vcCache.get(id)
        let count = vcCache.getCount()

        if count != 0 {
            if vc != nil {
                let v = vc as! VerifiableCredential
                return v
            }
        }
        vc = try storage.loadCredential(did, id)
        if (vc != nil) {
            let v = vc as! VerifiableCredential
            vcCache.put(v.id, data: v)
        }
        let v = vc as! VerifiableCredential
        return v
    }
    
    public func loadCredential(_ did: String, _ id: String) throws -> VerifiableCredential? {
        let _did: DID = try DID(did)
        return try loadCredential(_did, DIDURL(_did, id))
    }
    
    public func containsCredentials(_ did:DID) throws -> Bool {
        return try storage.containsCredentials(did)
    }
    
    public func containsCredentials(_ did: String) throws -> Bool {
        return try containsCredentials(DID(did))
    }
    
    public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool {
        return try storage.containsCredential(did, id)
    }
    
    public func containsCredential(_ did: String, _ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsCredential(_did, DIDURL(_did, id))
    }
    
    public func deleteCredential(_ did: DID , _ id: DIDURL) throws -> Bool{
        return try storage.deleteCredential(did, id)
    }
    
    public func deleteCredential(_ did: String , _ id: String) throws -> Bool{
        let _did: DID = try DID(did)
        return try deleteCredential(_did, DIDURL(_did, id))
    }
    
    public func listCredentials(_ did: DID) throws -> Array<DIDURL> {
        return try storage.listCredentials(did)
    }
    
    public func listCredentials(_ did: String) throws -> Array<DIDURL> {
        return try listCredentials(DID(did))
    }

    public func selectCredentials(_ did: DID, _ id: DIDURL,_ type: Array<Any>) throws -> Array<DIDURL> {
        return try storage.selectCredentials(did, id, type)
    }
    
    public func selectCredentials(_ did: String, _ id: String,_ type: Array<Any>) throws -> Array<DIDURL> {
        let _did: DID = try DID(did)
        return try selectCredentials(_did, DIDURL(_did, id), type)
    }
    
    public func storePrivateKey(_ did: DID,_ id: DIDURL, _ privateKey:String, _ storepass: String) throws {
        let data: Data = privateKey.data(using: .utf8)!
        let encryptedKey = try encryptToBase64(storepass, data)
        try storage.storePrivateKey(did, id, encryptedKey)
    }
    
    public func storePrivateKey(_ did: String,_ id: String, _ privateKey:String, _ storepass: String) throws {
        let _did: DID = try DID(did)
       try storePrivateKey(_did, DIDURL(_did, id), privateKey, storepass)
    }
    
    func loadPrivateKey(_ did: DID, id: DIDURL) throws -> String {
        return try storage.loadPrivateKey(did, id)
    }
    
    public func containsPrivateKeys(_ did: DID) throws -> Bool {
        return try storage.containsPrivateKeys(did)
    }
    
    public func containsPrivateKeys(_ did: String) throws -> Bool {
        return try containsPrivateKeys(DID(did))
    }
    
    public func containsPrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool {
        return try storage.containsPrivateKey(did, id)
    }
    
    public func containsPrivateKey(_ did: String,_ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsPrivateKey(_did, DIDURL(_did, id))
    }
    
    public func deletePrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool {
        return try storage.deletePrivateKey(did, id)
    }
    
    public func deletePrivateKey(_ did: String,_ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try deletePrivateKey(_did, DIDURL(_did, id))
    }

    public func sign(_ did: DID, _ id: DIDURL?, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        let sig: UnsafeMutablePointer<Int8> = UnsafeMutablePointer<Int8>.allocate(capacity: 88)
        var privatekeys: UnsafeMutablePointer<UInt8>
        if id == nil {
            let doc = try resolveDid(did)
            let id_1 = doc!.getDefaultPublicKey()
            privatekeys = try decryptFromBase64(storepass,try loadPrivateKey(did, id: id_1))
        }
        else {
         privatekeys = try decryptFromBase64(storepass,try loadPrivateKey(did, id: id!))
        }
        var cinputs: [CVarArg] = []
        for i in 0..<inputs.count {
            
            if (i % 2 == 0) {
                let json: String = inputs[i] as! String
                let cjson = json.toUnsafePointerInt8()!
                cinputs.append(cjson)
            }
            else {
                let count = inputs[i]
                cinputs.append(count)
            }
        }
        
        let c_inputs = getVaList(cinputs)
        let re = ecdsa_sign_base64v(sig, privatekeys, Int32(count), c_inputs)
        guard re >= 0 else {
            throw DIDStoreError.failue("sign error.")
        }
        return String(cString: sig)
    }
    
    public func sign(_ did: DID, _ storepass: String, _ count: Int, _ inputs: [CVarArg]) throws -> String {
        return try sign(did, nil, storepass, count, inputs)
    }

}

