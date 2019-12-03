import Foundation

public class DIDStore: NSObject {

    public static let DID_HAS_PRIVATEKEY = 0
    public static let DID_NO_PRIVATEKEY = 1
    public static let DID_ALL = 2
    private static var instance: DIDStore!
    var storeRootPath: String!
    private var backend: DIDBackend!

    override init() {
    }

    public static func creatInstance(_ type: String, _ location: String, _ adapter: DIDAdaptor) throws {
        guard type == "filesystem" else {
            throw DIDStoreError.failue("Unsupported store type:\(type)")
        }
        if instance == nil {
            instance = try FileSystemStore(location)
            instance.backend = DIDBackend(adapter)
        }
        instance.storeRootPath = location
    }

    public static func shareInstance() throws -> DIDStore? {
        guard (instance != nil) else {
            throw DIDStoreError.failue("Please call the creatInstance first.")
        }
        return instance
    }

    public func synchronize(_ storepass: String) throws {
        let privateIdentity: HDKey = try loadPrivateIdentity(storepass)!
        let nextIndex: Int = try loadPrivateIdentityIndex();
        var blanks: Int = 0
        var i: Int = 0
        
        if i < nextIndex || blanks < 10{
            let key: DerivedKey = try privateIdentity.derive(i++)
            let did: DID = DID(DID.METHOD, key.methodidString!)
            let doc = try backend.resolve(did)
            if (doc != nil) {
                // TODO: check local conflict
                try storeDid(doc!)
                
                // Save private key
                let privatekeyData: Data = try key.getPrivateKeyData()
                let encryptedKey: String = try encryptToBase64(storepass, privatekeyData)
                try storePrivateKey(did, doc!.getDefaultPublicKey()!, encryptedKey)
                
                if (i >= nextIndex){
                    try storePrivateIdentityIndex(i)
                }
                blanks = 0
            } else {
                blanks++
            }
        }
    }
    
    public func hasPrivateIdentity() throws -> Bool { return false }

    func storePrivateIdentity(_ key: String) throws {}

    func loadPrivateIdentity() throws -> String { return "" }

    func storePrivateIdentityIndex(_ index: Int) throws {}

    func loadPrivateIdentityIndex() throws -> Int { return 0 }

    private func encryptToBase64(_ passwd: String ,_ input: Data) throws -> String {
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
        
        if (try hasPrivateIdentity() && !force) {
            throw DIDStoreError.failue("Already has private indentity.")
        }
        let privateIdentity: HDKey = try HDKey.fromMnemonic(mnemonic, passphrase)

        // Save seed instead of root private key,
        // keep compatible with Native SDK
        let seedData = privateIdentity.getSeed()
        let encryptedIdentity = try encryptToBase64(storepass, seedData)
        try storePrivateIdentity(encryptedIdentity)
        try storePrivateIdentityIndex(0)
    }

    func initPrivateIdentity(_ language: Int, _ mnemonic: String, _ passphrase: String, _ storepass: String) throws -> Void {
        try initPrivateIdentity(language, mnemonic, passphrase, storepass, false)
    }
    
    func loadPrivateIdentity(_ storepass: String) throws -> HDKey? {
        guard try hasPrivateIdentity() else {
            return nil
        }
        let seed: Data = try decryptFromBase64(storepass, loadPrivateIdentity())
        return HDKey.fromSeed(seed)
    }
    
    public func newDid(_ storepass:String, _ hint:String?) throws -> DIDDocument {
        let privateIdentity =  try loadPrivateIdentity(storepass)

        guard (privateIdentity != nil) else {
            throw DIDStoreError.failue("DID Store not contains private identity.")
        }
        var nextIndex: Int = try loadPrivateIdentityIndex()

        let key: DerivedKey = try! privateIdentity!.derive(nextIndex++)
        let pks: [UInt8] = try key.getPublicKeyBytes()
        let methodIdString: String = DerivedKey.getIdString(pks)
        let did: DID = DID(DID.METHOD, methodIdString)
        let pk: DIDPublicKey = DIDPublicKey(try DIDURL(did, "primary"), Constants.defaultPublicKeyType, did, try key.getPublicKeyBase58())
        let doc: DIDDocument = DIDDocument()
        doc.subject = did
        _ = doc.addPublicKey(pk)
        _ = doc.addAuthenticationKey(pk)
        doc.readonly = true
        try storeDid(doc, hint)
        let privatekeyData: Data = try key.getPrivateKeyData()
        let encryptedKey = try encryptToBase64(storepass, privatekeyData)
        try storePrivateKey(did, pk.id, encryptedKey)
        try storePrivateIdentityIndex(nextIndex)
        
        return doc
    }

    public func newDid(_ storepass: String) throws -> DIDDocument {
        return try newDid(storepass, nil)
    }

    public func publishDid(_ doc: DIDDocument, _ signKey: DIDURL, _ storepass: String) throws -> Bool {
        try storeDid(doc)
        return try backend.create(doc, signKey, storepass)
    }

    public func publishDid(_ doc: DIDDocument, _ signKey: String, _ storepass :String) throws -> Bool {
        return try publishDid(doc, DIDURL(doc.subject!, signKey), storepass)
    }
    
    public func publishDid(_ doc: DIDDocument, _ storepass :String) throws -> Bool {
        try storeDid(doc)
        let signKey: DIDURL = doc.getDefaultPublicKey()!
        return try backend.create(doc, signKey, storepass)
    }

    public func updateDid(_ doc: DIDDocument,_ signKey: DIDURL,_ storepass :String) throws -> Bool {
        try storeDid(doc)
        return try backend.update(doc, signKey, storepass)
    }
    
    public func updateDid(_ doc: DIDDocument,_ signKey: String,_ storepass :String) throws -> Bool {
        return try updateDid(doc, DIDURL(doc.subject!, signKey), storepass)
    }
    
    public func updateDid(_ doc: DIDDocument, _ storepass :String) throws -> Bool {
        try storeDid(doc)
        let signKey: DIDURL = doc.getDefaultPublicKey()!
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
            return try backend.deactivate(did, signKey!, storepass)
        } catch {
            throw DIDError.failue("Can not resolve DID document.")
        }
    }
    
    public func resolveDid(_ did: DID, _ force: Bool) throws -> DIDDocument? {
        var doc = try backend.resolve(did)
        if doc !== nil {
            try storeDid(doc!)
        }
        else {
            if (!force){
                doc = try loadDid(did)
            }
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

    public func storeDid(_ doc: DIDDocument, _ hint: String?) throws {}
    public func storeDid(_ doc: DIDDocument) throws {
        try storeDid(doc, nil)
    }

    public func setDidHint(_ did: DID , _ hint:String ) throws {}
    
    public func setDidHint(_ did: String , _ hint:String ) throws {
        try setDidHint(DID(did), hint)
    }
    
    public func getDidHint(_ did: DID) throws -> String { return "" }
    
    public func getDidHint(_ did: String) throws -> String {
        return try getDidHint(DID(did))
    }

    public func loadDid(_ did: DID) throws -> DIDDocument? { return DIDDocument() }

    public func loadDid(_ did: String) throws -> DIDDocument? {
        return try loadDid(DID(did))
    }

    public func containsDid(_ did: DID) throws -> Bool { return false }

    public func containsDid(_ did: String) throws -> Bool {
        return try containsDid(DID(did))
    }

    public func deleteDid(_ did: DID) throws -> Bool { return false }

    public func deleteDid(_ did: String) throws -> Bool {
        return try deleteDid(DID(did))
    }

    public func listDids(_ filter: Int) throws -> Array<Entry<DID, String>> { return [Entry]() }

    public func storeCredential(_ credential: VerifiableCredential, _ hint: String?) throws { }
    public func storeCredential(_ credential: VerifiableCredential ) throws {
        return try storeCredential(credential, nil)
    }
                
    public func setCredentialHint(_ did: DID, _ id: DIDURL, _ hint: String) throws {}
    
    public func setCredentialHint(_ did: String, _ id: String, _ hint: String) throws {
        let _did: DID = try DID(did)
        try setCredentialHint(_did, DIDURL(_did, id), hint);
    }

    public func getCredentialHint(_ did: DID, _ id: DIDURL) throws -> String { return "" }
    
    public func getCredentialHint(_ did: String, _ id: String) throws -> String {
        let _did: DID = try DID(did)
        return try getCredentialHint(_did, DIDURL(_did, id))
    }
    
    public func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential? { return VerifiableCredential() }
    public func loadCredential(_ did: String, _ id: String) throws -> VerifiableCredential? {
        let _did: DID = try DID(did)
        return try loadCredential(_did, DIDURL(_did, id))
    }

    public func containsCredentials(_ did:DID) throws -> Bool { return false }
    public func containsCredentials(_ did: String) throws -> Bool {
        return try containsCredentials(DID(did))
    }
    
    public func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool { return false }
    public func containsCredential(_ did: String, _ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsCredential(_did, DIDURL(_did, id))
    }

    public func deleteCredential(_ did: DID , _ id: DIDURL) throws -> Bool{ return false }
    public func deleteCredential(_ did: String , _ id: String) throws -> Bool{
        let _did: DID = try DID(did)
        return try deleteCredential(_did, DIDURL(_did, id))
    }

    public func listCredentials(_ did: DID) throws -> Array<Entry<DIDURL, String>> { return [Entry]() }
    public func listCredentials(_ did: String) throws -> Array<Entry<DIDURL, String>> {
        return try listCredentials(DID(did))
    }

    public func selectCredentials(_ did: DID, _ id: DIDURL,_ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> { return [Entry]() }
    public func selectCredentials(_ did: String, _ id: String,_ type: Array<Any>) throws -> Array<Entry<DIDURL, String>> {
        let _did: DID = try DID(did)
        return try selectCredentials(_did, DIDURL(_did, id), type)
    }

    public func containsPrivateKeys(_ did: DID) throws -> Bool { return false }
    public func containsPrivateKeys(_ did: String) throws -> Bool {
        return try containsPrivateKeys(DID(did))
    }
    
    public func containsPrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool { return false }
    public func containsPrivateKey(_ did: String,_ id: String) throws -> Bool {
        let _did: DID = try DID(did)
        return try containsPrivateKey(_did, DIDURL(_did, id))
    }

    public func storePrivateKey(_ did: DID,_ id: DIDURL, _ privateKey:String) throws {}
    public func storePrivateKey(_ did: String,_ id: String, _ privateKey:String) throws {
        let _did: DID = try DID(did)
       try storePrivateKey(_did, DIDURL(_did, id), privateKey)
    }

    func loadPrivateKey(_ did: DID, id: DIDURL) throws -> String { return "" }
    
    public func deletePrivateKey(_ did: DID,_ id: DIDURL) throws -> Bool { return false }
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
            privatekeys = try decryptFromBase64(storepass,try loadPrivateKey(did, id: id_1!))
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

