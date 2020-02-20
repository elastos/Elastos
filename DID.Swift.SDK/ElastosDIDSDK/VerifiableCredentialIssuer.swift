import Foundation

public class VerifiableCredentialIssuer {
    private var _issuerDoc: DIDDocument
    private var _signKey: DIDURL

    private init(_ doc: DIDDocument, _ signKey: DIDURL? = nil) throws {
        // use the default public key if no signKey provided.
        var key = signKey
        if  key == nil {
            key = doc.defaultPublicKey
        }

        // The key would be used to sign verifiable crendetial when using
        // builder to create a new verifiable credential. So,
        // should make sure the key would be authenticationKey and
        // has corresponding private key to make sign.
        guard doc.containsAuthenticationKey(forId: key!) else {
            throw DIDError.illegalArgument()
        }
        guard doc.containsPrivateKey(forId: key!) else {
            throw DIDError.illegalArgument(Errors.NO_PRIVATE_KEY_EXIST)
        }

        self._issuerDoc = doc
        self._signKey = key!
    }

    public convenience init(_ doc: DIDDocument, _ signKey: DIDURL) throws {
        try self.init(doc, signKey)
    }

    public convenience init(_ doc: DIDDocument) throws {
        try self.init(doc)
    }

    private convenience init(_ did: DID, signKey: DIDURL?=nil, _ store: DIDStore) throws {
        let doc: DIDDocument
        do {
            doc = try store.loadDid(did)
        } catch {
            throw DIDError.didResolveError("Can not resolve did")
        }
        try self.init(doc, signKey)
    }

    public convenience init(_ did: DID, _ signKey: DIDURL, _ store: DIDStore) throws {
        try self.init(did, signKey, store)
    }

    public convenience init(_ did: DID, _ store: DIDStore) throws {
        try self.init(did, store)
    }

    public var did: DID {
        return _issuerDoc.subject
    }
    
    public var signKey: DIDURL {
        return _signKey
    }

    public func editingVerifiableCredential(for did: String) throws -> VerifiableCredentialBuilder {
        return VerifiableCredentialBuilder(try DID(did), _issuerDoc, signKey)
    }

    public func editingVerifiableCredential(for did: DID) -> VerifiableCredentialBuilder {
        return VerifiableCredentialBuilder(did, _issuerDoc, signKey)
    }
}
