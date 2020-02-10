import Foundation

public class DIDDocumentBuilder {
    private var _document: DIDDocument?

    init(_ did: DID, _ store: DIDStore) {
        self._document = DIDDocument(did)
        self._document!.getMeta().setStore(store)
    }

    init(_ doc: DIDDocument) { // Make a copy
        self._document = DIDDocument(doc)
    }

    private func getSubject() throws -> DID {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        return self._document!.subject
    }

    // publicKey scope
    public func appendPublicKey(_ id: DIDURL,
                                _ controller: DID,
                                _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else { // TODO: checkMe.
            throw DIDError.illegalArgument()
        }

        let publicKey = PublicKey(id, controller, keyBase58)
        _ = self._document!.appendPublicKey(publicKey)
        return self
    }

    public func appendPublicKey(_ id: String,
                                _ controller: String,
                                _ keyBase58: String) throws -> DIDDocumentBuilder {

        return try appendPublicKey(DIDURL(getSubject(), id), DID(controller), keyBase58)
    }

    public func removePublicKey(_ id: DIDURL,
                                _ force: Bool) throws -> DIDDocumentBuilder {
    
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        _ = self._document!.removePublicKey(atId: id, force)
        return self
    }

    public func removePublicKey(_ id: String,
                                _ force: Bool) throws -> DIDDocumentBuilder {
        return try removePublicKey(DIDURL(getSubject(), id), force)
    }

    public func removePublicKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        return try removePublicKey(id, false)
    }

    public func removePublicKey(_ id: String) throws -> DIDDocumentBuilder {
        return try removePublicKey(DIDURL(id), false)
    }

    // authenticationKey scope
    public func appendAuthenticationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let key = self._document!.publicKey(ofId: id)
        guard let _ = key else {
            throw DIDError.illegalArgument()
        }

        // use the ref "key" rather than parameter "id".
        _ = self._document!.appendAuthenticationKey(key!)
        return self
    }

    public func appendAuthenticationKey(_ id: String) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(DIDURL(getSubject(), id))
    }

    public func appendAuthenticationKey(_ id: DIDURL,
                                        _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else { //TODO: checkMe.
            throw DIDError.illegalArgument()
        }

        let publicKey = try PublicKey(id, getSubject(), keyBase58)
        _ = self._document!.appendAuthorizationKey(publicKey)
        return self
    }

    public func appendAuthenticationKey(_ id: String,
                                        _ keyBase58: String) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(DIDURL(getSubject(), id), keyBase58)
    }

    public func removeAuthenticationKey( _ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        _ = self._document!.removeAuthenticationKey(atId: id)
        return self
    }

    public func removeAuthenticationKey(_ id: String) throws -> DIDDocumentBuilder {
        return try removeAuthenticationKey(DIDURL(getSubject(), id))
    }

    // authorizationKey scope
    public func appendAuthorizationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let key = self._document!.publicKey(ofId: id)
        guard let _ = key else {
            throw DIDError.illegalArgument()
        }

        // use the ref "key" rather than parameter "id".
        _ = _document!.appendAuthorizationKey(key!)
        return self
    }

    public func appendAuthorizationKey(_ id: String) throws -> DIDDocumentBuilder  {
        return try appendAuthorizationKey(DIDURL(getSubject(), id))
    }

    public func appendAuthorizationKey(_ id: DIDURL,
                                       _ controller: DID,
                                       _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else { // TODO: checkMe
            throw DIDError.illegalArgument()
        }


        let publicKey = PublicKey(id, controller, keyBase58)
        _ = self._document!.appendAuthorizationKey(publicKey)
        return self
    }

    public func appendAuthorizationKey(_ id: String,
                                       _ controller: String,
                                       _ keyBase58: String) throws -> DIDDocumentBuilder {
        return try appendAuthorizationKey(DIDURL(getSubject(), id), DID(controller), keyBase58)
    }

    public func authorizationDid(_ id: DIDURL,
                                 _ controller: DID,
                                 _ key: DIDURL?) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        //TODO:
        return self
    }

    public func authorizationDID(_ id: DIDURL,
                                 _ controller: DID) throws -> DIDDocumentBuilder {
        // TODO
        return self
    }

    public func authorizationDID(_ id: String,
                                 _ controller: String,
                                 _ key: DIDURL?) throws -> DIDDocumentBuilder {
        // TODO:
        return self
    }

    public func authorizationDID(_ id: String,
                                 _ controller: String) throws -> DIDDocumentBuilder {
        // TODO:
        return self
    }

    public func removeAuthorizationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        _ = self._document!.removeAuthorizationKey(atId: id)
        return self
    }

    public func removeAuthorizationKey(_ id: String) throws -> DIDDocumentBuilder {
        return try removeAuthorizationKey(DIDURL(getSubject(), id))
    }

    // verifiableCredentials Scope
    public func appendCredential(_ credential: VerifiableCredential) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        _ = self._document!.appendCredential(credential)
        return self
    }

    public func removeCredential(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        _ = self._document!.removeCredential(atId: id)
        return self
    }

    public func removeCredential(_ id: String) throws -> DIDDocumentBuilder {
        return try removeCredential(DIDURL(getSubject(), id))
    }

    // Services
    public func appendService(_ id: DIDURL,
                              _ type: String,
                              _ endpoint: String) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        _ = self._document!.appendService(Service(id, type, endpoint))
        return self
    }

    public func appendService(_ id: String,
                              _ type: String,
                              _ endpoint: String) throws -> DIDDocumentBuilder {
        return try appendService(DIDURL(getSubject(), id), type, endpoint)
    }

    public func removeService(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        _ = self._document!.removeService(atId: id)
        return self
    }

    public func removeService(_ id: String) throws -> DIDDocumentBuilder {
        return try removeService(DIDURL(getSubject(), id))
    }

    public func withDefaultExpiresDate() throws -> DIDDocumentBuilder {
        // TODO:
        return self
    }

    public func withExpiresDate(_ expiresDate: Date) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        // TODO:
        return self
    }

    public func seal(using storePass: String) throws -> DIDDocument {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let _ = self._document!.defaultPublicKey
        // TODO

        // invalidate builder.
        let doc = self._document!
        self._document = nil
        return doc
    }
}
