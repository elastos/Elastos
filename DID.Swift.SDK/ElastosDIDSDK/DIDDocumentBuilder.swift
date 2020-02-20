import Foundation

public class DIDDocumentBuilder {
    private var document: DIDDocument?

    init(_ did: DID, _ store: DIDStore) {
        self.document = DIDDocument(did)
        self.document!.getMeta().setStore(store)
    }

    init(_ doc: DIDDocument) { // Make a copy
        self.document = DIDDocument(doc)
    }

    private func getSubject() throws -> DID {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        return document!.subject
    }

    public func appendPublicKey(_ id: DIDURL,
                                _ controller: DID,
                                _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.illegalArgument()
        }

        let publicKey = PublicKey(id, controller, keyBase58)
        guard document!.appendPublicKey(publicKey) else {
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
    
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removePublicKey(id, force) else {
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
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let key = document!.publicKey(ofId: id)
        guard let _ = key else {
            throw DIDError.illegalArgument()
        }
        guard document!.appendAuthenticationKey(key!) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func appendAuthenticationKey(_ id: String) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(DIDURL(getSubject(), id))
    }

    public func appendAuthenticationKey(_ id: DIDURL,
                                        _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = document else {
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
        guard document!.appendAuthorizationKey(publicKey) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func appendAuthenticationKey(_ id: String,
                                        _ keyBase58: String) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(DIDURL(getSubject(), id), keyBase58)
    }

    public func removeAuthenticationKey( _ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeAuthenticationKey(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeAuthenticationKey(_ id: String) throws -> DIDDocumentBuilder {
        return try removeAuthenticationKey(DIDURL(getSubject(), id))
    }

    public func appendAuthorizationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let key = document!.publicKey(ofId: id)
        guard let _ = key else {
            throw DIDError.illegalArgument()
        }
        // use the ref "key" rather than parameter "id".
        guard document!.appendAuthorizationKey(key!) else {
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

        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.illegalArgument()
        }

        let publicKey = PublicKey(id, controller, keyBase58)
        guard document!.appendAuthorizationKey(publicKey) else {
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
        guard let _ = document else {
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

        guard document!.appendAuthorizationKey(publicKey) else {
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
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeAuthorizationKey(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeAuthorizationKey(_ id: String) throws -> DIDDocumentBuilder {
        return try removeAuthorizationKey(DIDURL(getSubject(), id))
    }

    public func appendCredential(_ credential: VerifiableCredential) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.appendCredential(credential) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeCredential(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeCredential(id) else {
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
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.appendService(Service(id, type, endpoint)) else {
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
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeService(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    public func removeService(_ id: String) throws -> DIDDocumentBuilder {
        return try removeService(DIDURL(getSubject(), id))
    }

    public func withDefaultExpiresDate() throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        document!.setExpirationDate(DateHelper.maxExpirationDate())
        return self
    }

    public func withExpiresDate(_ expiresDate: Date) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let maxExpirationDate = DateHelper.maxExpirationDate()
        guard !DateHelper.isExpired(expiresDate, maxExpirationDate) else {
            throw DIDError.illegalArgument()
        }

        document!.setExpirationDate(expiresDate)
        return self
    }

    public func sealed(using storePassword: String) throws -> DIDDocument {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }
        if  document!.expirationDate == nil {
            document!.setExpirationDate(DateHelper.maxExpirationDate())
        }

        let signKey = document!.defaultPublicKey
        let data:Data = try document!.toJson(true, true)
        let signature = try document!.makeSignWithIdentiy(signKey, storePassword, [data])

        document!.setProof(DIDDocumentProof(signKey, signature))

        // invalidate builder.
        let doc = self.document!
        self.document = nil

        return doc
    }
}
