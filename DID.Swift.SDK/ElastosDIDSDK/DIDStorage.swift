protocol DIDStorage {
    // Root private identity
    func containsPrivateIdentity() -> Bool
    func storePrivateIdentity(_ key: String) throws
    func loadPrivateIdentity() throws -> String

    func containsPublicIdentity() -> Bool
    func storePublicIdentity(_ key: String) throws
    func loadPublicIdentity() throws -> String

    func storePrivateIdentityIndex(_ index: Int) throws
    func loadPrivateIdentityIndex() throws -> Int

    func containMnemonic() -> Bool
    func storeMnemonic(_ mnemonic: String) throws
    func loadMnemonic() throws -> String
    
    // DIDs
    func storeDidMeta(_ did: DID, _ meta: DIDMeta) throws
    func loadDidMeta(_ did: DID) throws -> DIDMeta

    func storeDid(_ doc: DIDDocument) throws
    func loadDid(_ did: DID) throws -> DIDDocument
    func containsDid(_ did: DID) -> Bool
    func deleteDid(_ did: DID) -> Bool
    func listDids(_ filter: Int) throws -> Array<DID>

    // Credentials
    func storeCredentialMeta(_ did: DID, _ id: DIDURL, _ meta: CredentialMeta) throws
    func loadCredentialMeta(_ did: DID, _ id: DIDURL) throws -> CredentialMeta

    func storeCredential(_ credential: VerifiableCredential) throws
    func loadCredential(_ did: DID, _ id: DIDURL) throws -> VerifiableCredential
    func containsCredentials(_ did: DID) -> Bool
    func containsCredential(_ did: DID, _ id: DIDURL) -> Bool
    func deleteCredential(_ did: DID, _ id: DIDURL) -> Bool
    func listCredentials(_ did: DID) throws -> Array<DIDURL>
    func selectCredentials(_ did: DID, _ id: DIDURL?, _ type: Array<Any>?) throws -> Array<DIDURL>

    // Private keys
    func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String) throws
    func loadPrivateKey(_ did: DID, _ id: DIDURL) throws -> String
    func containsPrivateKeys(_ did: DID) -> Bool
    func containsPrivateKey(_ did: DID, _ id: DIDURL) -> Bool
    func deletePrivateKey(_ did: DID, _ id: DIDURL) -> Bool

    func changePassword(_  callback: (String) throws -> String) throws
}
