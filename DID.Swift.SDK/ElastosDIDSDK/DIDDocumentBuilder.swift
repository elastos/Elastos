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

    public func appendPublicKey(_ id: DIDURL,
                                _ controller: DID,
                                _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.illegalArgument()
        }

        let publicKey = PublicKey(id, controller, keyBase58)
        guard self._document!.appendPublicKey(publicKey) else {
            throw DIDError.illegalArgument()
        }

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
        guard self._document!.removePublicKey(id, force) else {
            throw DIDError.illegalArgument()
        }

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
        guard self._document!.appendAuthenticationKey(key!) else {
            throw DIDError.illegalArgument()
        }

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
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.illegalArgument()
        }

        let publicKey: PublicKey
        do {
            publicKey = try PublicKey(id, getSubject(), keyBase58)
        } catch {
            throw DIDError.illegalArgument()
        }
        guard self._document!.appendAuthorizationKey(publicKey) else {
            throw DIDError.illegalArgument()
        }

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
        guard self._document!.removeAuthenticationKey(id) else {
            throw DIDError.illegalArgument()
        }

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
        guard self._document!.appendAuthorizationKey(key!) else {
            throw DIDError.illegalArgument()
        }

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
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.illegalArgument()
        }

        let publicKey = PublicKey(id, controller, keyBase58)
        guard self._document!.appendAuthorizationKey(publicKey) else {
            throw DIDError.illegalArgument()
        }

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
        guard try! controller != getSubject() else {
            throw DIDError.illegalArgument()
        }

        let controllerDoc = try controller.resolve()
        guard let _ = controllerDoc else {
            throw DIDError.didResolveError("Can not resolve \(controller) DID.")
        }

        var usedKey: DIDURL? = key
        if  usedKey == nil {
            usedKey = controllerDoc!.defaultPublicKey
        }

        // Check the key should be a authentication key
        let targetKey = controllerDoc!.authorizationKey(ofId: usedKey!)
        guard let _ = targetKey else {
            throw DIDError.illegalArgument()
        }

        let publicKey = PublicKey(id, targetKey!.getType(), controller,
                                  targetKey!.publicKeyBase58)

        guard self._document!.appendAuthorizationKey(publicKey) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func authorizationDid(_ id: DIDURL,
                                 _ controller: DID) throws -> DIDDocumentBuilder {
        return try authorizationDid(id, controller, nil)
    }

    public func authorizationDID(_ id: String,
                                 _ controller: String,
                                 _ key: String?) throws -> DIDDocumentBuilder {
        let controllerId = try DID(controller)
        let usedKey:DIDURL? = (key != nil ? try DIDURL(controllerId, key!) : nil)

        return try authorizationDid(DIDURL(getSubject(), id), controllerId, usedKey)
    }

    public func authorizationDid(_ id: String,
                                 _ controller: String) throws -> DIDDocumentBuilder {
        return try authorizationDID(id, controller, nil)
    }

    public func removeAuthorizationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard self._document!.removeAuthorizationKey(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeAuthorizationKey(_ id: String) throws -> DIDDocumentBuilder {
        return try removeAuthorizationKey(DIDURL(getSubject(), id))
    }

    public func appendCredential(_ credential: VerifiableCredential) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard self._document!.appendCredential(credential) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeCredential(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard self._document!.removeCredential(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeCredential(_ id: String) throws -> DIDDocumentBuilder {
        return try removeCredential(DIDURL(getSubject(), id))
    }

    public func appendService(_ id: DIDURL,
                              _ type: String,
                              _ endpoint: String) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard self._document!.appendService(Service(id, type, endpoint)) else {
            throw DIDError.illegalArgument()
        }

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
        guard self._document!.removeService(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeService(_ id: String) throws -> DIDDocumentBuilder {
        return try removeService(DIDURL(getSubject(), id))
    }

    public func withDefaultExpiresDate() throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        self._document!.setExpirationDate(DateHelper.maxExpirationDate())
        return self
    }

    public func withExpiresDate(_ expiresDate: Date) throws -> DIDDocumentBuilder {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let maxExpirationDate = DateHelper.maxExpirationDate()
        guard !DateHelper.isExpired(expiresDate, maxExpirationDate) else {
            throw DIDError.illegalArgument()
        }

        self._document!.setExpirationDate(expiresDate)
        return self
    }

    public func seal(using storePass: String) throws -> DIDDocument {
        guard let _ = self._document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }
        if (self._document!.expirationDate == nil) {
            self._document!.setExpirationDate(DateHelper.maxExpirationDate())
        }

        let signKey = self._document!.defaultPublicKey
        let json = try self._document!.toJson(true, true)
        let signature = try self._document!.sign(using: signKey, storePass: storePass, json.data(using: .utf8)!)
        self._document!.setProof(DIDDocumentProof(signKey, signature))

        // invalidate builder.
        let doc = self._document!
        self._document = nil
        
        return doc
    }
}
