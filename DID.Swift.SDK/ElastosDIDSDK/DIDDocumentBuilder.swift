/*
* Copyright (c) 2020 Elastos Foundation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

import Foundation

/// A DIDDocument Builder to modify DIDDocument elems.
public class DIDDocumentBuilder {
    private var document: DIDDocument?

    init(_ did: DID, _ store: DIDStore) {
        self.document = DIDDocument(did)
        self.document!.getMetadata().setStore(store)
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

    /// Add public key to DID Document.
    /// Each public key has an identifier (id) of its own, a type, and a controller,
    /// as well as other properties publicKeyBase58 depend on which depend on what type of key it is.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - keyBase58: Key propertie depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
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

    /// Add public key to DID Document.
    /// Each public key has an identifier (id) of its own, a type, and a controller,
    /// as well as other properties publicKeyBase58 depend on which depend on what type of key it is.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - keyBase58: Key propertie depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendPublicKey(with id: DIDURL,
                             controller: String,
                              keyBase58: String) throws -> DIDDocumentBuilder {
        return try appendPublicKey(id, DID(controller), keyBase58)
    }

    /// Add public key to DID Document.
    /// Each public key has an identifier (id) of its own, a type, and a controller,
    /// as well as other properties publicKeyBase58 depend on which depend on what type of key it is.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - keyBase58: Key propertie depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendPublicKey(with id: String,
                             controller: String,
                              keyBase58: String) throws -> DIDDocumentBuilder {

        return try appendPublicKey(DIDURL(getSubject(), id), DID(controller), keyBase58)
    }

    private func removePublicKey(_ id: DIDURL,
                                 _ force: Bool) throws -> DIDDocumentBuilder {
    
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard try document!.removePublicKey(id, force) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Remove specified public key from DID Document.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - force: True, must to remove key; false,
    ///    if key is authentication or authorization key, not to remove.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removePublicKey(with id: DIDURL,
                               _ force: Bool) throws -> DIDDocumentBuilder {
        return try removePublicKey(id, force)
    }

    /// Remove specified public key from DID Document.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - force: True, must to remove key; false,
    ///    if key is authentication or authorization key, not to remove.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removePublicKey(with id: String,
                               _ force: Bool) throws -> DIDDocumentBuilder {
        return try removePublicKey(DIDURL(getSubject(), id), force)
    }

    /// Remove specified public key from DID Document.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removePublicKey(with id: DIDURL) throws -> DIDDocumentBuilder {
        return try removePublicKey(id, false)
    }

    /// Remove specified public key from DID Document.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removePublicKey(with id: String) throws -> DIDDocumentBuilder {
        return try removePublicKey(DIDURL(getSubject(), id), false)
    }

    // authenticationKey scope
    private func appendAuthenticationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let key = document!.publicKey(ofId: id)
        guard let _ = key else {
            throw DIDError.illegalArgument()
        }
        guard document!.appendAuthenticationKey(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Add public key to Authenticate.
    /// Authentication is the mechanism by which the controller(s) of a DID can
    /// cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthenticationKey(with id: DIDURL) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(id)
    }

    /// Add public key to Authenticate.
    /// Authentication is the mechanism by which the controller(s) of a DID can
    /// cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthenticationKey(with id: String) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(DIDURL(getSubject(), id))
    }

    private func appendAuthenticationKey(_ id: DIDURL,
                                         _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.illegalArgument()
        }

        let key = PublicKey(id, try getSubject(), keyBase58)
        key.setAuthenticationKey(true)
        guard document!.appendPublicKey(key) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Add public key to Authenticate.
    /// Authentication is the mechanism by which the controller(s) of a DID can
    /// cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - keyBase58: Key propertie depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthenticationKey(with id: DIDURL,
                                      keyBase58: String) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(id, keyBase58)
    }

    /// Add public key to Authenticate.
    /// Authentication is the mechanism by which the controller(s) of a DID can
    /// cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - keyBase58: Key propertie depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthenticationKey(with id: String,
                                      keyBase58: String) throws -> DIDDocumentBuilder {
        return try appendAuthenticationKey(DIDURL(getSubject(), id), keyBase58)
    }

    private func removeAuthenticationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeAuthenticationKey(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Remove authentication key from Authenticate.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeAuthenticationKey(with id: DIDURL) throws -> DIDDocumentBuilder {
        return try removeAuthenticationKey(id)
    }

    /// Remove authentication key from Authenticate.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeAuthenticationKey(with id: String) throws -> DIDDocumentBuilder {
        return try removeAuthenticationKey(DIDURL(getSubject(), id))
    }

    private func appendAuthorizationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        let key = document!.publicKey(ofId: id)
        guard let _ = key else {
            throw DIDError.illegalArgument()
        }
        // use the ref "key" rather than parameter "id".
        guard document!.appendAuthorizationKey(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Add public key to authorizate.
    /// Authorization is the mechanism used to state how operations may be performed on behalf of the DID subject.
    /// - Parameter id: An identifier of authorization key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthorizationKey(with id: DIDURL) throws -> DIDDocumentBuilder {
        return try appendAuthorizationKey(id)
    }

    /// Add public key to authorizate.
    /// Authorization is the mechanism used to state how operations may be performed on behalf of the DID subject.
    /// - Parameter id: An identifier of authorization key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthorizationKey(with id: String) throws -> DIDDocumentBuilder  {
        return try appendAuthorizationKey(DIDURL(getSubject(), id))
    }

    /// Add public key to authorizate.
    /// Authorization is the mechanism used to state how operations may be performed on behalf of the DID subject.
    /// - Parameters:
    ///   - id: An identifier of authorization key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - keyBase58: Key property depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthorizationKey(_ id: DIDURL,
                                       _ controller: DID,
                                       _ keyBase58: String) throws -> DIDDocumentBuilder {

        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard Base58.bytesFromBase58(keyBase58).count == HDKey.PUBLICKEY_BYTES else {
            throw DIDError.illegalArgument()
        }

        let key = PublicKey(id, controller, keyBase58)
        key.setAthorizationKey(true)
        _ = document!.appendPublicKey(key)

        return self
    }

    /// Add public key to authorizate.
    /// Authorization is the mechanism used to state how operations may be performed on behalf of the DID subject.
    /// - Parameters:
    ///   - id: An identifier of authorization key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - keyBase58: Key property depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthorizationKey(with id: DIDURL,
                                    controller: DID,
                                     keyBase58: String) throws -> DIDDocumentBuilder {

        return try appendAuthorizationKey(id, controller, keyBase58)
    }

    /// Add public key to authorizate.
    /// Authorization is the mechanism used to state how operations may be performed on behalf of the DID subject.
    /// - Parameters:
    ///   - id: An identifier of authorization key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - keyBase58: Key property depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthorizationKey(with id: String,
                                    controller: String,
                                     keyBase58: String) throws -> DIDDocumentBuilder {

        return try appendAuthorizationKey(DIDURL(getSubject(), id), DID(controller), keyBase58)
    }

    /// Add public key to authorizate.
    /// Authorization is the mechanism used to state how operations may be performed on behalf of the DID subject.
    /// - Parameters:
    ///   - id: An identifier of authorization key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - keyBase58: Key property depend on key type.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendAuthorizationKey(with id: String,
                                    controller: DID,
                                     keyBase58: String) throws -> DIDDocumentBuilder {

        return try appendAuthorizationKey(DIDURL(getSubject(), id), controller, keyBase58)
    }

    private func authorizationDid(_ id: DIDURL,
                                  _ controller: DID,
                                  _ key: DIDURL?) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard try controller != getSubject() else {
            throw DIDError.illegalArgument()
        }

        let controllerDoc: DIDDocument?
        do {
            controllerDoc = try controller.resolve()
        } catch {
            throw DIDError.didResolveError("Can not resolve \(controller) DID.")
        }

        guard let _ = controllerDoc else {
            throw DIDError.notFoundError(id.toString())
        }

        var usedKey: DIDURL? = key
        if  usedKey == nil {
            usedKey = controllerDoc!.defaultPublicKey
        }

        // Check the key should be a authentication key
        let targetKey = controllerDoc!.authenticationKey(ofId: usedKey!)
        guard let _ = targetKey else {
            throw DIDError.illegalArgument()
        }

        let pk = PublicKey(id, targetKey!.getType(), controller, targetKey!.publicKeyBase58)
        pk.setAthorizationKey(true)
        _ = document!.appendPublicKey(pk)

        return self
    }

    /// Add Authorization key to Authentication array according to DID.
    /// Authentication is the mechanism by which the controller(s) of a DID can cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - key: An identifier of authorization key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func authorizationDid(with id: DIDURL,
                              controller: DID,
                                     key: DIDURL) throws -> DIDDocumentBuilder {

        return try authorizationDid(id, controller, key)
    }

    /// Add Authorization key to Authentication array according to DID.
    /// Authentication is the mechanism by which the controller(s) of a DID can cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func authorizationDid(with id: DIDURL,
                              controller: DID) throws -> DIDDocumentBuilder {

        return try authorizationDid(id, controller, nil)
    }

    /// Add Authorization key to Authentication array according to DID.
    /// Authentication is the mechanism by which the controller(s) of a DID can cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    ///   - key: An identifier of authorization key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func authorizationDid(with id: String,
                              controller: String,
                                     key: String) throws -> DIDDocumentBuilder {
        let controllerId = try DID(controller)
        let usedKey:DIDURL = try DIDURL(controllerId, key)

        return try authorizationDid(DIDURL(getSubject(), id), controllerId, usedKey)
    }

    /// Add Authorization key to Authentication array according to DID.
    /// Authentication is the mechanism by which the controller(s) of a DID can cryptographically prove that they are associated with that DID.
    /// A DID Document must include an authentication property.
    /// - Parameters:
    ///   - id: An identifier of public key.
    ///   - controller: A controller property, identifies the controller of the corresponding private key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func authorizationDid(with id: String,
                              controller: String) throws -> DIDDocumentBuilder {

        return try authorizationDid(DIDURL(getSubject(), id), DID(controller), nil)
    }

    private func removeAuthorizationKey(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeAuthorizationKey(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Remove authorization key from Authenticate.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeAuthorizationKey(with id: DIDURL) throws -> DIDDocumentBuilder {
        return try removeAuthorizationKey(id)
    }

    /// Remove authorization key from Authenticate.
    /// - Parameter id: An identifier of public key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeAuthorizationKey(with id: String) throws -> DIDDocumentBuilder {
        return try removeAuthorizationKey(DIDURL(getSubject(), id))
    }

    /// Add one credential to credential array.
    /// - Parameter credential: The handle to Credential.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with credential: VerifiableCredential) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.appendCredential(credential) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    private func appendCredential(_ id: DIDURL,
                                  _ types: Array<String>?,
                                  _ subject: Dictionary<String, String>,
                                  _ expirationDate: Date?,
                                  _ storePassword: String) throws -> DIDDocumentBuilder  {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        guard !subject.isEmpty && !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let realTypes: Array<String>
        if let _ = types {
            realTypes = types!
        } else {
            realTypes = Array<String>(["SelfProclaimedCredential"])
        }

        let realExpires: Date
        if let _ = expirationDate {
            realExpires = expirationDate!
        } else {
            realExpires = document!.expirationDate!
        }

        let issuer  = try VerifiableCredentialIssuer(document!)
        let builder = issuer.editingVerifiableCredentialFor(did: document!.subject)

        do {
            let credential = try builder.withId(id)
                                    .withTypes(realTypes)
                                    .withProperties(subject)
                                    .withExpirationDate(realExpires)
                                    .sealed(using: storePassword)
            _ =  document!.appendCredential(credential)
        } catch {
            throw DIDError.malformedCredential()
        }
        
        return self
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - subject: The array of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                   types: Array<String>,
                                 subject: Dictionary<String, String>,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(id, types, subject, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - subject: The array of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                   types: Array<String>,
                                 subject: Dictionary<String, String>,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), types, subject, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - subject: The array of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                 subject: Dictionary<String, String>,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(id, nil, subject, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - subject: The array of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                 subject: Dictionary<String, String>,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), nil, subject, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - subject: The array of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                   types: Array<String>,
                                 subject: Dictionary<String, String>,
                     using storePassword: String) throws -> DIDDocumentBuilder {
        return try appendCredential(id, types, subject, nil, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - subject: The array of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                   types: Array<String>,
                                 subject: Dictionary<String, String>,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), types, subject, nil, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - subject: The array of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                 subject: Dictionary<String, String>,
                     using storePassword: String) throws -> DIDDocumentBuilder {
        return try appendCredential(id, nil, subject, nil, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - subject: The array of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                 subject: Dictionary<String, String>,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), nil, subject, nil, storePassword)
    }

    private func appendCredential(_ id: DIDURL,
                                  _ types: Array<String>?,
                                  _ json: String,
                                  _ expirationDate: Date?,
                                  _ storePassword: String) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        guard !json.isEmpty && !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let realTypes: Array<String>
        if let _ = types {
            realTypes = types!
        } else {
            realTypes = Array<String>(["SelfProclaimedCredential"])
        }

        let realExpires: Date
        if let _ = expirationDate {
            realExpires = expirationDate!
        } else {
            realExpires = document!.expirationDate!
        }

        let issuer  = try VerifiableCredentialIssuer(document!)
        let builder = issuer.editingVerifiableCredentialFor(did: document!.subject)

        do {
            let credential = try builder.withId(id)
                                    .withTypes(realTypes)
                                    .withProperties(json)
                                    .withExpirationDate(realExpires)
                                    .sealed(using: storePassword)
            _ =  document!.appendCredential(credential)
        } catch {
            throw DIDError.malformedCredential()
        }
        
        return self
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - json: The json string of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                   types: Array<String>,
                                    json: String,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {
        return try appendCredential(id, types, json, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - json: The json string of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                   types: Array<String>,
                                    json: String,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), types, json, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - json: The json string of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                    json: String,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(id, nil, json, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - json: The json string of credential subject property.
    ///   - expirationDate: The time to credential be expired.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                    json: String,
                          expirationDate: Date,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), nil, json, expirationDate, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - json: The json string of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                   types: Array<String>,
                                    json: String,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(id, types, json, nil, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - types: The array of credential types.
    ///   - json: The json string of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                   types: Array<String>,
                                    json: String,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), types, json, nil, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - json: The json string of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: DIDURL,
                                    json: String,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(id, nil, json, nil, storePassword)
    }

    /// Add one credential to credential array.
    /// - Parameters:
    ///   - id: The handle to DIDURL.
    ///   - json: The json string of credential subject property.
    ///   - storePassword: Password for DIDStores.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendCredential(with id: String,
                                    json: String,
                     using storePassword: String) throws -> DIDDocumentBuilder {

        return try appendCredential(DIDURL(getSubject(), id), nil, json, nil, storePassword)
    }

    private func removeCredential(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeCredential(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Remove specified credential from credential array.
    /// - Parameter id: An identifier of Credential.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeCredential(with id: DIDURL) throws -> DIDDocumentBuilder {
        return try removeCredential(id)
    }

    /// Remove specified credential from credential array.
    /// - Parameter id: An identifier of Credential.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeCredential(with id: String) throws -> DIDDocumentBuilder {
        return try removeCredential(DIDURL(getSubject(), id))
    }

    private func appendService(_ id: DIDURL,
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

    /// Add one Service to services array.
    /// - Parameters:
    ///   - id: The identifier of Service.
    ///   - type: The type of Service.
    ///   - endpoint: ServiceEndpoint property is a valid URI.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendService(with id: DIDURL,
                                 type: String,
                             endpoint: String) throws -> DIDDocumentBuilder {
        return try appendService(id, type, endpoint)
    }

    /// Add one Service to services array.
    /// - Parameters:
    ///   - id: The identifier of Service.
    ///   - type: The type of Service.
    ///   - endpoint: ServiceEndpoint property is a valid URI.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func appendService(with id: String,
                                 type: String,
                             endpoint: String) throws -> DIDDocumentBuilder {
        return try appendService(DIDURL(getSubject(), id), type, endpoint)
    }

    private func removeService(_ id: DIDURL) throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }
        guard document!.removeService(id) else {
            throw DIDError.illegalArgument()
        }

        return self
    }

    /// Remove specified Service to services array.
    /// - Parameter id: The identifier of Service.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeService(with id: DIDURL) throws -> DIDDocumentBuilder {
        return try removeService(id)
    }

    /// Remove specified Service to services array.
    /// - Parameter id: The identifier of Service.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func removeService(with id: String) throws -> DIDDocumentBuilder {
        return try removeService(DIDURL(getSubject(), id))
    }

    /// Set default expire time about DID Document.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
    public func withDefaultExpiresDate() throws -> DIDDocumentBuilder {
        guard let _ = document else {
            throw DIDError.invalidState(Errors.DOCUMENT_ALREADY_SEALED)
        }

        document!.setExpirationDate(DateHelper.maxExpirationDate())
        return self
    }

    /// Set expire time about DID Document.
    /// - Parameter expiresDate: ime to expire.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: DIDDocumentBuilder instance.
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

    /// Finish modiy document.
    /// - Parameter storePassword: Pass word to sign.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: A handle to DIDDocument
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
        let data: Data = try document!.toJson(true, true)
        let signature = try document!.sign(signKey, storePassword, [data])

        document!.setProof(DIDDocumentProof(signKey, signature))

        // invalidate builder.
        let doc = self.document!
        self.document = nil

        return doc
    }
}
