
typealias ReEncryptor = (String) -> String

protocol DIDStorage {

    // Root private identity
    func containsPrivateIdentity() throws -> Bool

    func storePrivateIdentity(_ key: String) throws

    func loadPrivateIdentity() throws -> String

    func storePrivateIdentityIndex(_ index: Int) throws

    func loadPrivateIdentityIndex() throws -> Int
    
    func storeMnemonic(_ mnemonic: String) throws

    func loadMnemonic() throws -> String
    
    // DIDs
    func storeDidMeta(_ did: DID, _ alias: DIDMeta?) throws

    func loadDidMeta(_ did: DID) throws -> DIDMeta

    func storeDid(_ doc: DIDDocument) throws

    func loadDid(_ did: DID) throws -> DIDDocument

    func containsDid(_ did: DID) throws -> Bool

    func deleteDid(_ did: DID) throws -> Bool

    func listDids(_ filter: Int) throws -> Array<DID>

    // Credentials
    func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta?) throws

    func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta

    func storeCredential(_ credential: VerifiableCredential) throws

    func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential?

    func containsCredentials(_ did: DID) throws -> Bool

    func containsCredential(_ did: DID, _ id: DIDURL) throws -> Bool

    func deleteCredential(_ did: DID, _ id: DIDURL) throws -> Bool

    func listCredentials(_ did: DID) throws -> Array<DIDURL>

    func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>)
        throws -> Array<DIDURL>

    // Private keys
    func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws

    func loadPrivateKey(_ did: DID, _ id: DIDURL) throws -> String

    func containsPrivateKeys(_ did: DID) throws -> Bool

    func containsPrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool

    func deletePrivateKey(_ did: DID, _ id: DIDURL) throws -> Bool
    
    func changePassword(_ reEncryptor: ReEncryptor) throws
}
