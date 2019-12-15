
public class DIDStoreBackend {
   
    // Root private identity
    public func containsPrivateIdentity() throws -> Bool { return false }

    public func storePrivateIdentity(_ key: String) throws { }

    public func loadPrivateIdentity() throws -> String { return "" }

    public func  storePrivateIdentityIndex(_ index: Int) throws {}

    public func loadPrivateIdentityIndex() throws -> Int { return 0 }

    // DIDs
    public func storeDidAlias(_ did: DID, _ alias: String) throws { }

    public func loadDidAlias(_ did: DID) throws -> String { return "" }

    public func storeDid(_ doc: DIDDocument) throws { }

    public func loadDid(_ did: DID)
        throws -> DIDDocument { return DIDDocument() }

    public func containsDid(_ did: DID) throws -> Bool { return false }

    public func deleteDid(_ did: DID) throws -> Bool { return false }

    public func listDids(_ filter: Int) throws -> Array<DID> { return [] }

    // Credentials
    public func storeCredentialAlias(_ did: DID, _ id: DIDURL, _ alias: String)
        throws { }

    public func loadCredentialAlias(_ did: DID, _ id: DIDURL)
        throws -> String { return "" }

    public func storeCredential(_ credential: VerifiableCredential)
        throws { }

    public func loadCredential(_ did: DID, _ id: DIDURL)
        throws -> VerifiableCredential { return VerifiableCredential() }

    public func containsCredentials(_ did: DID) throws -> Bool { return false }

    public func containsCredential(_ did: DID, _ id: DIDURL)
        throws -> Bool { return false }

    public func deleteCredential(_ did: DID, _ id: DIDURL)
        throws -> Bool { return false }

    public func listCredentials(_ did: DID) throws -> Array<DIDURL> { return [] }

    public func selectCredentials(_ did: DID, _ id: DIDURL, _ type: Array<Any>)
        throws -> Array<DIDURL> { return [] }

    // Private keys
    public func storePrivateKey(_ did: DID, _ id: DIDURL, _ privateKey: String)
        throws { }

    public func loadPrivateKey(_ did: DID, _ id: DIDURL)
        throws -> String { return "" }

    public func containsPrivateKeys(_ did: DID) throws -> Bool { return false }

    public func containsPrivateKey(_ did: DID, _ id: DIDURL)
        throws -> Bool { return false }

    public func deletePrivateKey(_ did: DID, _ id: DIDURL)
        throws -> Bool { return false }
    
}
