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

public class DIDDocument {
    private static let TAG = "DIDDocument"
    private var _capacity: Int = 0

    private var _subject: DID?
    private var _expirationDate: Date?
    private var _proof: DIDDocumentProof?
    private var _metadata: DIDMeta?

    private var publicKeyMap: EntryMap<PublicKey>
    private var credentialMap: EntryMap<VerifiableCredential>
    private var serviceMap: EntryMap<Service>

    class EntryMap<T: DIDObject> {
        private var map: Dictionary<DIDURL, T>?

        init() {}

        init(_ source: EntryMap) {
            guard source.map != nil else {
                return
            }
            map = Dictionary<DIDURL, T>()
            for (id, value) in source.map! {
                map![id] = value
            }
        }

        func count(_ fulfill: (T) -> Bool) -> Int {
            var total: Int = 0

            guard map?.count ?? 0 > 0 else {
                return 0
            }

            for value in map!.values {
                if fulfill(value) {
                    total += 1
                }
            }
            return total
        }

        func get(forKey: DIDURL, _ fulfill: (T) -> Bool) -> T? {
            let value = map?[forKey]

            guard let _ = value else {
                return nil
            }
            guard fulfill(value!) else {
                return nil
            }

            return value!
        }

        func values(_ fulfill: (T) -> Bool) -> Array<T> {
            var result = Array<T>()
            var preKeys = Array<DIDURL>()

            guard let _ = map else {
                return result
            }

            for (key, value) in map! {
                if fulfill(value) {
                    preKeys.append(key)
                }
            }

            let sortedKeys = preKeys.sorted { (d1, d2) -> Bool in
                let compareResult = d1.toString().compare(d2.toString())
                return compareResult == ComparisonResult.orderedAscending
            }

            for key in sortedKeys {
                result.append(map![key]!)
            }

            return result
        }

        func select(_ id: DIDURL?, _ type: String?, _ filter: (T) -> Bool) -> Array<T> {
            var result = Array<T>()

            guard id != nil || type != nil else {
                return result
            }
            if map?.isEmpty ?? true {
                return result
            }

            for value in map!.values {
                if id != nil && value.getId() != id! {
                    continue
                }

                if type != nil {
                    // Credential' type is a list.
                    if value is VerifiableCredential {
                        let credential = value as! VerifiableCredential
                        if !credential.getTypes().contains(type!) {
                            continue
                        }
                    } else {
                        if value.getType() != type! {
                            continue
                        }
                    }
                }
                if filter(value) {
                    result.append(value)
                }
            }
            return result
        }

        func append(_ value: T) {
            if  map == nil {
                map = Dictionary<DIDURL, T>()
            }

            map![value.getId()] = value
        }

        func remove(_ key: DIDURL) -> Bool {
            return map?.removeValue(forKey: key) != nil
        }
    }

    private init() {
        publicKeyMap = EntryMap<PublicKey>()
        credentialMap = EntryMap<VerifiableCredential>()
        serviceMap = EntryMap<Service>()
    }

    init(_ subject: DID) {
        self._subject = subject

        publicKeyMap = EntryMap<PublicKey>()
        credentialMap = EntryMap<VerifiableCredential>()
        serviceMap = EntryMap<Service>()
    }

    init(_ doc: DIDDocument) {
        publicKeyMap = EntryMap<PublicKey>(doc.publicKeyMap)
        credentialMap = EntryMap<VerifiableCredential>(doc.credentialMap)
        serviceMap = EntryMap<Service>(doc.serviceMap)

        self._subject = doc.subject
        self._expirationDate = doc.expirationDate
        self._proof = doc.proof
        let metadata = doc.getMetadata()
        metadata.clearLastModified()
        self.setMetadata(metadata)
    }

    /// The unique identifier of the DID document
    public var subject: DID {
        return self._subject!
    }

    private func setSubject(_ subject: DID) {
        self._subject = subject
    }

    /// Get the count of public keys.
    /// A DID Document must include a publicKey property.
    public var publicKeyCount: Int {
        return self.publicKeyMap.count() { value -> Bool in return true }
    }

    /// Get the array of publicKeys.
    /// - Returns: The array of publicKeys
    public func publicKeys() -> Array<PublicKey> {
        return self.publicKeyMap.values() { value -> Bool in return true }
    }

    /// Get public key conforming to type or identifier.
    /// - Parameters:
    ///   - byId: An identifier of public key to be selected.
    ///   - andType: The type of public key to be selected.
    /// - Returns: Array of public keys selected.
    public func selectPublicKeys(byId: DIDURL, andType: String?) -> Array<PublicKey> {
        return self.publicKeyMap.select(byId, andType) { value -> Bool in return true }
    }

    /// Get public key conforming to type or identifier.
    /// - Parameters:
    ///   - byId: An identifier of public key to be selected.
    ///   - andType: The type of public key to be selected.
    /// - Throws: If an error occurred, throw error
    /// - Returns: Array of public keys selected.
    public func selectPublicKeys(byId: String, andType: String?) throws -> Array<PublicKey> {
        let id = try DIDURL(subject, byId)
        return selectPublicKeys(byId: id, andType: andType)
    }

    /// Get public key conforming to type or identifier.
    /// - Parameter byType: The type of public key to be selected.
    /// - Returns: Array of public keys selected.
    public func selectPublicKeys(byType: String) -> Array<PublicKey> {
        return self.publicKeyMap.select(nil, byType) { value -> Bool in return true }
    }

    /// Get public key according to identifier of public key.
    /// - Parameter ofId: An identifier of public key.
    /// - Returns: The handle to public key
    public func publicKey(ofId: DIDURL) -> PublicKey? {
        return self.publicKeyMap.get(forKey: ofId) { value -> Bool in return true }
    }

    /// Get public key according to identifier of public key.
    /// - Parameter ofId: An identifier of public key.
    /// - Throws: If an error occurred, throw error
    /// - Returns: The handle to public key
    public func publicKey(ofId: String) throws -> PublicKey? {
        return publicKey(ofId: try DIDURL(subject, ofId))
    }

    /// Check key if public key or not.
    /// - Parameter forId: An identifier of public key.
    /// - Returns: True if has public key, or false.
    public func containsPublicKey(forId: DIDURL) -> Bool {
        return publicKey(ofId: forId) != nil
    }

    /// Check key if public key or not.
    /// - Parameter forId: An identifier of public key.
    /// - Throws: If an error occurred, throw error
    /// - Returns: True if has public key, or false.
    public func containsPublicKey(forId: String) throws -> Bool {
        return try publicKey(ofId: forId) != nil
    }

    /// Check key if private key or not.
    /// - Parameter forId: An identifier of private key.
    /// - Returns: True if has public key, or false.
    public func containsPrivateKey(forId: DIDURL) -> Bool {
        guard containsPublicKey(forId: forId) else {
            return false
        }
        guard let store = getMetadata().store else {
            return false
        }

        return store.containsPrivateKey(for: self.subject, id: forId)
    }

    /// Check key if private key or not.
    /// - Parameter forId: An identifier of private key.
    /// - Returns: True if has public key, or false.
    public func containsPrivateKey(forId: String) -> Bool {
        do {
            return containsPrivateKey(forId: try DIDURL(self.subject, forId))
        } catch {
            return false
        }
    }

    private func getDefaultPublicKey() -> DIDURL? {
        for key in publicKeys() {
            if subject != key.controller {
                continue
            }

            let address = HDKey.toAddress(key.publicKeyBytes)
            if  address == subject.methodSpecificId {
                return key.getId()
            }
        }
        return nil
    }

    /// Get primary public key, which is for creating method specific string.
    public var defaultPublicKey: DIDURL {
        return getDefaultPublicKey()!
    }

    func keyPair_PublicKey(ofId: DIDURL) throws -> Data {
        guard containsPublicKey(forId: ofId) else {
            throw DIDError.illegalArgument("Key no exist")
        }
        guard getMetadata().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store.")
        }
        guard getMetadata().store!.containsPrivateKey(for: subject, id: ofId) else {
            throw DIDError.illegalArgument("Don't have private key")
        }
        let pubKey = publicKey(ofId: ofId)
        let pubs = pubKey!.publicKeyBytes
        let pubData = Data(bytes: pubs, count: pubs.count)
        let publicKeyData = try HDKey.PEM_ReadPublicKey(pubData)

        return publicKeyData.data(using: .utf8)!
    }

    func keyPair_PrivateKey(ofId: DIDURL, using storePassword: String) throws -> Data {
        guard containsPublicKey(forId: ofId) else {
            throw DIDError.illegalArgument("Key no exist")
        }
        guard getMetadata().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store.")
        }
        guard getMetadata().store!.containsPrivateKey(for: subject, id: ofId) else {
            throw DIDError.illegalArgument("Don't have private key")
        }

        let pubKey = publicKey(ofId: ofId)
        let pubs = pubKey!.publicKeyBytes
        let pubData = Data(bytes: pubs, count: pubs.count)

        let privKey = try getMetadata().store?.loadPrivateKey(subject, ofId, storePassword)
        let privateKeyData = try HDKey.PEM_ReadPrivateKey(pubData, privKey!)

        return privateKeyData.data(using: .utf8)!
    }

    // The result is extended private key format, the real private key is
    // 32 bytes long start from position 46.
    public func derive(index: Int, storePassword: String) throws -> String {

        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument("storePassword is empty.")
        }
        guard getMetadata().attachedStore else {
            throw DIDError.didStoreError("Not attached with a DID store.")
        }

        let key = HDKey.deserialize((try getMetadata().store?.loadPrivateKey(subject, getDefaultPublicKey()!, storePassword))!)
        return key.derive(index).serializeBase58()
    }

    /// Create a JwtBuilder instance.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtBuilder instance.
    public func jwtBuilder() throws -> JwtBuilder {

        let build = JwtBuilder(issuer: subject.toString(), publicKey: { (id) -> Data in

            var _id: DIDURL
            if id == nil {
                _id = self.getDefaultPublicKey()!
            } else {
                _id = try DIDURL(self.subject, id!)
            }
            return try self.keyPair_PublicKey(ofId: _id)

        }) { (id, storePassword) -> Data in
            var _id: DIDURL

            if id == nil {
                _id = self.getDefaultPublicKey()!
            } else {
                _id = try DIDURL(self.subject, id!)
            }
            return try self.keyPair_PrivateKey(ofId: _id, using: storePassword)
        }
        return build.setIssuer(iss: subject.description)
    }

    /// Create a JwtParserBuilder instance.
    /// - Returns: JwtParserBuilder instance.
    public func jwtParserBuilder() -> JwtParserBuilder {

        let builder: JwtParserBuilder = JwtParserBuilder()
        builder.getPublicKey = { (id) in

            var _id: DIDURL
            if id == nil {
                _id = self.getDefaultPublicKey()!
            } else {
                _id = try DIDURL(self.subject, id!)
            }
            return try self.keyPair_PublicKey(ofId: _id)
        }
        builder.getPrivateKey = {(id, storePassword) in

            var _id: DIDURL
            if id == nil {
                _id = self.getDefaultPublicKey()!
            } else {
                _id = try DIDURL(self.subject, id!)
            }
            return try self.keyPair_PrivateKey(ofId: _id, using: storePassword!)
        }
        return builder
    }

    /// Create a JwtParser instance.
    /// - Throws: If error occurs, throw error.
    /// - Returns: JwtParser instance.
    public func build() throws -> JwtParser {
        return JwtParser()
    }

    func appendPublicKey(_ publicKey: PublicKey) -> Bool {
        for key in publicKeys() {
            if  key.getId() == publicKey.getId() ||
                key.publicKeyBase58 == publicKey.publicKeyBase58 {
                return false
            }
        }

        publicKeyMap.append(publicKey)
        return true
    }

    func removePublicKey(_ id: DIDURL, _ force: Bool) throws -> Bool {
        let key = publicKey(ofId: id)
        guard let _ = key else {
            return false
        }

        // Can not remove default public key.
        guard self.getDefaultPublicKey() != id else {
            return false
        }

        if !force && (key!.isAuthenticationKey || key!.isAthorizationKey) {
            return  false
        }

        _ = publicKeyMap.remove(id)
        _ = getMetadata().store?.deletePrivateKey(for: subject, id: id)
        return true
    }

    /// Get the count of authentication keys.
    public var authenticationKeyCount: Int {
        return publicKeyMap.count() { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    /// Get the array of authentication keys.
    /// - Returns: The array of authentication keys.
    public func authenticationKeys() -> Array<PublicKey> {
        return publicKeyMap.values() { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    /// Get authentication key conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of authentication key to be selected.
    ///   - andType: The type of authentication key to be selected.
    /// - Returns: The array of authentication keys.
    public func selectAuthenticationKeys(byId: DIDURL, andType: String?) -> Array<PublicKey> {
        return publicKeyMap.select(byId, andType) { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    /// Get authentication key conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of authentication key to be selected.
    ///   - andType: The type of authentication key to be selected.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: The array of authentication keys.
    public func selectAuthenticationKeys(byId: String, andType: String?) throws -> Array<PublicKey> {
        let id = try DIDURL(subject, byId)
        return selectAuthenticationKeys(byId: id, andType: andType)
    }

    /// Get authentication key conforming to type or identifier of key.
    /// - Parameter byType: The type of authentication key to be selected.
    /// - Returns: The array of authentication keys.
    public func selectAuthenticationKeys(byType: String) -> Array<PublicKey> {
        return publicKeyMap.select(nil, byType) { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    /// Get authentication key according to identifier of authentication key.
    /// A DID Document must include a authentication property.
    /// - Parameter ofId: An identifier of authentication key.
    /// - Returns: The handle to public key.
    public func authenticationKey(ofId: DIDURL) -> PublicKey? {
        return publicKeyMap.get(forKey: ofId) { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    /// Get authentication key according to identifier of authentication key.
    /// A DID Document must include a authentication property.
    /// - Parameter ofId: An identifier of authentication key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: The handle to public key.
    public func authenticationKey(ofId: String) throws -> PublicKey?  {
        return authenticationKey(ofId: try DIDURL(subject, ofId))
    }

    /// Check key if authentiacation key or not.
    /// - Parameter forId: An identifier of authentication key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: true if has authentication key, or false.
    public func containsAuthenticationKey(forId: String) throws -> Bool {
        return try authenticationKey(ofId: forId) != nil
    }

    /// Check key if authentiacation key or not.
    /// - Parameter forId: An identifier of authentication key.
    /// - Returns: true if has authentication key, or false.
    public func containsAuthenticationKey(forId: DIDURL) -> Bool {
        return authenticationKey(ofId: forId) != nil
    }

    /// Add public key to Authenticate.
    ///  Authentication is the mechanism by which the controller(s) of a DID can cryptographically prove that they are associated with that DID.
    ///  A DID Document must include an authentication property.
    /// - Parameter id: An identifier of public key.
    /// - Returns: true if append authentication key success, or false.
    func appendAuthenticationKey(_ id: DIDURL) -> Bool {
        let key = publicKey(ofId: id)
        guard let _ = key else {
            return false
        }

        // Make sure that controller should be current DID subject.
        guard key!.controller == self.subject else {
            return false
        }

        key!.setAuthenticationKey(true)
        return true
    }

    func removeAuthenticationKey(_ id: DIDURL) -> Bool {
        let key = publicKey(ofId: id)
        guard let _ = key else {
            return false
        }

        // Can not remove default publicKey.
        guard getDefaultPublicKey() != id else {
            return false
        }

        key!.setAuthenticationKey(false)
        return true
    }

    /// Get the count of authorization keys.
    public var authorizationKeyCount: Int {
        return publicKeyMap.count() { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    /// Get the array of authorization keys.
    /// - Returns: The array of authentication keys.
    public func authorizationKeys() -> Array<PublicKey> {
        return publicKeyMap.values() { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    /// Get authorization key conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of authorization key to be selected.
    ///   - andType: The type of authorization key to be selected.
    /// - Returns: Array of authorization keys selected.
    public func selectAuthorizationKeys(byId: DIDURL, andType: String?) -> Array<PublicKey> {
        return publicKeyMap.select(byId, andType) { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    /// Get authorization key conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of authorization key to be selected.
    ///   - andType: The type of authorization key to be selected.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: Array of authorization keys selected.
    public func selectAuthorizationKeys(byId: String, andType: String?) throws -> Array<PublicKey> {
        let id = try DIDURL(subject, byId)
        return selectAuthorizationKeys(byId: id, andType: andType)
    }

    /// Get authorization key conforming to type or identifier of key.
    /// - Parameter byType: An identifier of authorization key to be selected.
    /// - Returns: Array of authorization keys selected.
    public func selectAuthorizationKeys(byType: String) -> Array<PublicKey> {
        return publicKeyMap.select(nil, byType) { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    /// Get authorization key according to identifier of key.
    /// - Parameter ofId: An identifier of authorization key.
    /// - Returns: If has authorization key, return the handle to public key,Otherwise, return nil.
    public func authorizationKey(ofId: DIDURL) -> PublicKey? {
        return publicKeyMap.get(forKey: ofId) { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    /// Get authorization key according to identifier of key.
    /// - Parameter ofId: An identifier of authorization key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: If has authorization key, return the handle to public key,Otherwise, return nil.
    public func authorizationKey(ofId: String) throws -> PublicKey?  {
        return authorizationKey(ofId: try DIDURL(subject, ofId))
    }

    /// Check key if authorization key or not.
    /// - Parameter forId: An identifier of authorization key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: true if has authorization key, or false.
    public func containsAuthorizationKey(forId: String) throws -> Bool {
        return try authorizationKey(ofId: forId) != nil
    }

    /// Check key if authorization key or not.
    /// - Parameter forId: An identifier of authorization key.
    /// - Returns: true if has authorization key, or false.
    public func containsAuthorizationKey(forId: DIDURL) -> Bool {
        return authorizationKey(ofId: forId) != nil
    }

    func appendAuthorizationKey(_ id: DIDURL) -> Bool {
        let key = publicKey(ofId: id)
        guard let _ = key else {
            return false
        }

        // Make sure that controller should be current DID subject.
        guard key!.controller != self.subject else {
            return false
        }

        key!.setAthorizationKey(true)
        return true
    }

    func removeAuthorizationKey(_ id: DIDURL) -> Bool {
        let key = publicKey(ofId: id)
        guard let _ = key else {
            return false
        }

        // Can not remove default publicKey.
        guard getDefaultPublicKey() != id else {
            return false
        }

        key!.setAthorizationKey(false)
        return true
    }

    /// Get the count of credentials.
    public var credentialCount: Int {
        return credentialMap.count() { value -> Bool in return true }
    }

    /// Get the array of credentials.
    /// - Returns: Array of authorization keys.
    public func credentials() -> Array<VerifiableCredential> {
        return credentialMap.values() { value -> Bool in return true }
    }

    /// Get credential key conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of credential key to be selected.
    ///   - andType: The type of credential key to be selected.
    /// - Returns: Array of credential keys selected.
    public func selectCredentials(byId: DIDURL, andType: String?) -> Array<VerifiableCredential>  {
        return credentialMap.select(byId, andType) { value -> Bool in return true }
    }

    /// Get credential key conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of credential key to be selected.
    ///   - andType: The type of credential key to be selected.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: Array of credential keys selected.
    public func selectCredentials(byId: String, andType: String?) throws -> Array<VerifiableCredential>  {
        let id = try DIDURL(subject, byId)
        return selectCredentials(byId: id, andType: andType)
    }

    /// Get credential key conforming to type or identifier of key.
    /// - Parameter byType: The type of credential key to be selected.
    /// - Returns: Array of credential keys selected.
    public func selectCredentials(byType: String) -> Array<VerifiableCredential>  {
        return credentialMap.select(nil, byType) { value -> Bool in return true }
    }

    /// Get credential according to identifier of credential.
    /// - Parameter ofId: An identifier of Credential.
    /// - Returns: If has the credential, return the handle to Credential. Otherwise, return nil
    public func credential(ofId: DIDURL) -> VerifiableCredential? {
        return credentialMap.get(forKey: ofId) { value -> Bool in return true }
    }

    /// Get credential according to identifier of credential.
    /// - Parameter ofId: An identifier of Credential.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: If has the credential, return the handle to Credential. Otherwise, return nil
    public func credential(ofId: String) throws -> VerifiableCredential? {
        return credential(ofId: try DIDURL(subject, ofId))
    }

    func appendCredential(_ vc: VerifiableCredential) -> Bool {
        guard vc.subject.did == subject else {
            return false
        }
        let _vc = credential(ofId: vc.getId())
        guard _vc == nil else {
            // TODO: Throw ERROR
            Log.e(DIDDocument.TAG, "Credential \(vc.getId()) already exist.")
            return false
        }
        credentialMap.append(vc)
        return true
    }

    func removeCredential(_ id: DIDURL) -> Bool {
        let vc = credential(ofId: id)
        guard let _ = vc else {
            return false
        }

        return credentialMap.remove(id)
    }

    /// Get the count of service keys.
    public var serviceCount: Int {
        return serviceMap.count() { value -> Bool in return true }
    }

    /// Get the array of services.
    /// - Returns: Array of services keys.
    public func services() -> Array<Service> {
        return serviceMap.values() { value -> Bool in return true }
    }

    /// Get Service conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of Service to be selected.
    ///   - andType: The type of Service.
    /// - Returns: Array of Service keys selected.
    public func selectServices(byId: DIDURL, andType: String?) -> Array<Service>  {
        return serviceMap.select(byId, andType) { value -> Bool in return true }
    }

    /// Get Service conforming to type or identifier of key.
    /// - Parameters:
    ///   - byId: An identifier of Service to be selected.
    ///   - andType: The type of Service.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: Array of Service keys selected.
    public func selectServices(byId: String, andType: String?) throws -> Array<Service>  {
        let id = try DIDURL(subject, byId)
        return selectServices(byId: id, andType: andType)
    }

    /// Get Service conforming to type or identifier of key.
    /// - Parameter byType: The type of Service.
    /// - Returns: Array of Service keys selected.
    public func selectServices(byType: String) -> Array<Service>  {
        return serviceMap.select(nil, byType) { value -> Bool in return true }
    }

    /// Get service according to identifier of credential.
    /// - Parameter ofId: An identifier of service key.
    /// - Returns: If has service, return the handle to service,Otherwise, return nil.
    public func service(ofId: DIDURL) -> Service? {
        return serviceMap.get(forKey: ofId) { value -> Bool in return true }
    }

    /// Get service according to identifier of credential.
    /// - Parameter ofId: An identifier of service key.
    /// - Throws: if an error occurred, throw error.
    /// - Returns: If has service, return the handle to service,Otherwise, return nil.
    public func service(ofId: String) throws -> Service? {
        return service(ofId: try DIDURL(subject, ofId))
    }

    func appendService(_ service: Service) -> Bool {
        serviceMap.append(service)
        return true
    }

    func removeService(_ id: DIDURL) -> Bool {
        return serviceMap.remove(id)
    }

    /// Get expire time about DID Document.
    public var expirationDate: Date? {
        return self._expirationDate
    }

    func setExpirationDate(_ expirationDate: Date) {
        self._expirationDate = expirationDate
    }

    /// Get proof of DIDDocument.
    public var proof: DIDDocumentProof {
        // Guaranteed that this field would not be nil because the object
        // was generated by "builder".
        return _proof!
    }

    // This type of getXXXX function would specifically be provided for
    // sdk internal when we can't be sure about it's validity/integrity.
    func getProof() -> DIDDocumentProof? {
        return self._proof
    }

    func setProof(_ proof: DIDDocumentProof) {
        self._proof = proof
    }

    func setMetadata(_ metadata: DIDMeta) {
        self._metadata = metadata
        subject.setMetadata(metadata)
    }

    /// Get DIDMetaData from DID.
    /// - Returns: Return the handle to DIDMetadata
    public func getMetadata() -> DIDMeta {
        if _metadata == nil {
            _metadata = DIDMeta()
            subject.setMetadata(_metadata!)
        }

        return _metadata!
    }

    /// Save DIDMetaData of document.
    /// - Throws: If no error occurs, throw error.
    public func saveMetadata() throws {
        if _metadata != nil && _metadata!.attachedStore {
            try  _metadata!.store?.storeDidMetadata(subject, _metadata!)
        }
    }

    /// Check that document is deactivated or not.
    /// true if document is deactivated, otherwise false.
    public var isDeactivated: Bool {
        return getMetadata().isDeactivated
    }

    /// Check that document is expired or not.
    /// true if document is expired, otherwise false.
    public var isExpired: Bool {
        return DateHelper.isExipired(self.expirationDate!)
    }

    /// Check that document is genuine or not.
    /// true if document is genuine, otherwise false.
    public var isGenuine: Bool {
        // Document should be signed (only) by default public key.
        guard proof.creator == defaultPublicKey else {
            return false
        }
        // Unsupported public key type;
        guard proof.type == Constants.DEFAULT_PUBLICKEY_TYPE else {
            return false
        }

        do {
            let data: Data = try toJson(true, true)
            return try verify(proof.creator, proof.signature, [data])
        } catch {
            return false
        }
    }

    /// Check that document is valid or not.
    /// true if document is valid, otherwise false.
    public var isValid: Bool {
        return !isDeactivated && !isExpired && isGenuine
    }

    /// Get DIDDocument Builder to modify document.
    /// - Returns: DIDDocumentBuilder instance.
    public func editing() -> DIDDocumentBuilder {
        return DIDDocumentBuilder(self)
    }

    /// Sign data by DID.
    /// SDK will get default key from DID
    /// - Parameters:
    ///   - storePassword: Pass word to sign.
    ///   - data: To sign of data list.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: The  string of signature data.
    public func sign(using storePassword: String, for data: Data...) throws -> String {
        return try sign(self.defaultPublicKey, storePassword, data)
    }

    /// Sign data by DID.
    /// - Parameters:
    ///   - withId: Public key to sign.
    ///   - storePassword: Pass word to sign.
    ///   - data: To sign of data list.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: The  string of signature data.
    public func sign(withId: DIDURL, using storePassword: String, for data: Data...) throws -> String {
        return try sign(withId, storePassword, data)
    }

    /// Sign data by DID.
    /// - Parameters:
    ///   - withId: Public key to sign.
    ///   - storePassword: Pass word to sign.
    ///   - data: To sign of data list.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: The  string of signature data.
    public func sign(withId: String, using storePassword: String, for data: Data...) throws -> String {
        return try sign(try DIDURL(self.subject, withId), storePassword, data)
    }

    func sign(_ id: DIDURL, _ storePassword: String, _ data: [Data]) throws -> String {
        guard data.count > 0 else {
            throw DIDError.illegalArgument()
        }
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }
        return try signDigest(withId: id, using: storePassword, for: sha256Digest(data))
    }

    private func sha256Digest(_ data: [Data]) -> Data {
        var cinputs: [CVarArg] = []
        var capacity: Int = 0
        data.forEach { data in
            let json = String(data: data, encoding: .utf8)
            if json != "" {
                let cjson = json!.toUnsafePointerInt8()!
                cinputs.append(cjson)
                cinputs.append(json!.count)
                capacity += json!.count * 3
            }
        }

        let c_inputs = getVaList(cinputs)
        let count = cinputs.count / 2
        _capacity = capacity
        // digest
        let cdigest = UnsafeMutablePointer<UInt8>.allocate(capacity: capacity)
        let size = sha256v_digest(cdigest, Int32(count), c_inputs)
        let cdigestPointerToArry: UnsafeBufferPointer<UInt8> = UnsafeBufferPointer(start: cdigest, count: size)

        return Data(buffer: cdigestPointerToArry)
    }

    /// Sign digest by DID.
    /// SDK will get default key from DID
    /// - Parameters:
    ///   - storePassword: Pass word to sign.
    ///   - digest: The digest to sign.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: The  string of signature data.
    public func signDigest(using storePassword: String, for digest: Data) throws -> String {
        return try signDigest(withId: self.defaultPublicKey, using: storePassword, for: digest)
    }

    public func signDigest(withId: DIDURL, using storePassword: String, for digest: Data) throws -> String {
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard getMetadata().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store")
        }

        return try getMetadata().store!.sign(subject, withId, storePassword, digest, _capacity)
    }

    /// Sign digest by DID.
    /// - Parameters:
    ///   - withId: Public key to sign
    ///   - storePassword: Pass word to sign.
    ///   - digest: The digest to sign.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: The  string of signature data.
    public func signDigest(withId: String, using storePassword: String, for digest: Data) throws -> String {
        let _id = try DIDURL(subject, withId)

        return try signDigest(withId: _id, using: storePassword, for: digest)
    }

    /// verify data.
    /// SDK will get default key from DID
    /// - Parameters:
    ///   - signature: Signature data.
    ///   - data: To verify of data list
    /// - Throws: If no error occurs, throw error.
    /// - Returns: True on success, or false.
    public func verify(signature: String, onto data: Data...) throws -> Bool {
        return try verify(self.defaultPublicKey, signature, data)
    }

    /// verify data.
    /// - Parameters:
    ///   - withId: Public key to sign
    ///   - signature: Signature data.
    ///   - data: To verify of data list
    /// - Throws: If no error occurs, throw error.
    /// - Returns: True on success, or false.
    public func verify(withId: DIDURL, using signature: String, onto data: Data...) throws -> Bool {
        return try verify(withId, signature, data)
    }

    /// verify data.
    /// - Parameters:
    ///   - withId: Public key to sign
    ///   - signature: Signature data.
    ///   - data: To verify of data list
    /// - Throws: If no error occurs, throw error.
    /// - Returns: True on success, or false.
    public func verify(withId: String, using signature: String, onto data: Data...) throws -> Bool {
        return try verify(DIDURL(self.subject, withId), signature, data)
    }

    func verify(_ id: DIDURL, _ sigature: String, _ data: [Data]) throws -> Bool {
        guard data.count > 0 else {
            throw DIDError.illegalArgument()
        }
        guard !sigature.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let pubKey = publicKey(ofId: id)
        guard let _ = pubKey else {
            throw DIDError.illegalArgument()
        }
        let digest = sha256Digest(data)

        return try verifyDigest(withId: id, using: sigature, for: digest)
    }

    /// verify digest.
    /// SDK will get default key from DID
    /// - Parameters:
    ///   - signature: Signature data.
    ///   - digest: The digest to sign.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: True on success, or false.
    public func verifyDigest(signature: String, for digest: Data) throws -> Bool {

        return try verifyDigest(withId: self.defaultPublicKey, using: signature, for: digest)
    }

    /// verify digest.
    /// - Parameters:
    ///   - withId: Public key to sign.
    ///   - signature: Signature data.
    ///   - digest: The digest to sign.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: True on success, or false.
    public func verifyDigest(withId: DIDURL, using signature: String, for digest: Data) throws -> Bool {
        guard !signature.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let pubKey = publicKey(ofId: withId)
        guard let _ = pubKey else {
            throw DIDError.illegalArgument()
        }

        let pks = pubKey!.publicKeyBytes
        var pkData = Data(bytes: pks, count: pks.count)
        let cpk = pkData.withUnsafeMutableBytes { (pk: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8> in
            return pk
        }
        let cdigest = digest.toPointer()
        let size: Int = digest.count
        let csignature = signature.toUnsafeMutablePointerInt8()
        let re = ecdsa_verify_base64(csignature, cpk, UnsafeMutablePointer(mutating: cdigest), size)

        return re == 0 ? true : false
    }

    /// verify digest.
    /// - Parameters:
    ///   - id: Public key to sign.
    ///   - signature: Signature data.
    ///   - digest: The digest to sign.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: True on success, or false.
    public func verifyDigest(withId id: String, using signature: String, for digest: Data) throws -> Bool {
        let _id = try DIDURL(subject, id)

        return try verifyDigest(withId: _id, using: signature, for: digest)
    }

    private func parse(_ doc: JsonNode) throws {
        let serializer = JsonSerializer(doc)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withHint("document subject")
        let did = try serializer.getDID(Constants.ID, options)
        setSubject(did)

        var node: JsonNode?

        node = doc.get(forKey: Constants.PUBLICKEY)
        guard let _ = node else {
            throw DIDError.malformedDocument("missing publicKey")
        }
        try parsePublicKeys(node!)

        node = doc.get(forKey: Constants.AUTHENTICATION)
        if let _ = node {
            try parseAuthenticationKeys(node!)
        }

        // Add default public key to authentication keys if need.
        let defaultKey = self.getDefaultPublicKey()
        guard let _ = defaultKey else {
            throw DIDError.malformedDocument("missing default public key")
        }

        if !containsAuthenticationKey(forId: defaultKey!) {
            _ = appendAuthenticationKey(defaultKey!)
        }

        node = doc.get(forKey: Constants.AUTHORIZATION)
        if let _ = node {
            try parseAuthorizationKeys(node!)
        }

        node = doc.get(forKey: Constants.VERIFIABLE_CREDENTIAL)
        if let _ = node {
            try parseCredential(node!)
        }

        node = doc.get(forKey: Constants.SERVICE)
        if let _ = node {
            try parseService(node!)
        }

        options = JsonSerializer.Options()
                                .withOptional()
                                .withHint("document expires")
        let expirationDate = try serializer.getDate(Constants.EXPIRES, options)
        self.setExpirationDate(expirationDate)

        node = doc.get(forKey: Constants.PROOF)
        guard let _ = node else {
            throw DIDError.malformedDocument("missing document proof")
        }

        setProof(try DIDDocumentProof.fromJson(node!, defaultKey!))
    }

    private func parsePublicKeys(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()

        guard array?.count ?? 0 > 0 else {
            throw DIDError.malformedDocument("invalid publicKeys, should not be empty.")
        }

        for node in array! {
            do {
                _ = appendPublicKey(try PublicKey.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseAuthenticationKeys(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()
        guard array?.count ?? 0 > 0 else {
            return
        }

        for node in array! {
            do {
                var pk: PublicKey
                if let _ = node.asDictionary() {
                    pk =  try PublicKey.fromJson(node, self.subject)
                }
                else {
                    let serializer = JsonSerializer(node)
                    var options: JsonSerializer.Options
                    options = JsonSerializer.Options()
                                            .withRef(subject)
                    let didUrl = try serializer.getDIDURL(options)
                    pk = publicKey(ofId: didUrl!)!
                }
                _ = appendAuthenticationKey(pk.getId())
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseAuthorizationKeys(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()
        guard array?.count ?? 0 > 0 else {
            return
        }

        for node in array! {
            do {
                var pk: PublicKey
                if let _ = node.asDictionary() {
                    pk =  try PublicKey.fromJson(node, self.subject)
                }
                else {
                    let serializer = JsonSerializer(node)
                    var options: JsonSerializer.Options
                    options = JsonSerializer.Options()
                                            .withRef(subject)
                    let didUrl = try serializer.getDIDURL(options)
                    pk = publicKey(ofId: didUrl!)!
                }
                _ = appendAuthorizationKey(pk.getId())
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseCredential(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()
        guard array?.count ?? 0 > 0 else {
            return
        }

        for node in array! {
            do {
                _ = appendCredential(try VerifiableCredential.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseService(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()
        guard array?.count ?? 0 > 0 else {
            return
        }

        for node in array! {
            do {
                _ = appendService(try Service.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    class func convertToDIDDocument(fromJson: JsonNode) throws -> DIDDocument {
        let doc = DIDDocument()
        try doc.parse(fromJson)

        return doc
    }

    /// Get DID Document from Data context.
    /// - Parameter data: Context of did conforming to Data informat.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: DIDDocument instance.
    public class func convertToDIDDocument(fromData data: Data) throws -> DIDDocument {
        guard !data.isEmpty else {
            throw DIDError.illegalArgument()
        }

        let node: Dictionary<String, Any>?
        do {
            node = try JSONSerialization.jsonObject(with: data, options: []) as? Dictionary<String, Any>
        } catch {
            throw DIDError.malformedDocument()
        }

        let doc = DIDDocument()
        try doc.parse(JsonNode(node!))

        return doc
    }

    /// Get DID Document from json string context.
    /// - Parameter fromJson: Context of did conforming to string informat.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: DIDDocument instance.
    public class func convertToDIDDocument(fromJson: String) throws -> DIDDocument {
        return try  convertToDIDDocument(fromData: fromJson.data(using: .utf8)!)
    }

    /// Get DID Document from data path.
    /// - Parameter fromFileAtPath: The data path with string.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: DIDDocument instance.
    public class func convertToDIDDocument(fromFileAtPath : String) throws -> DIDDocument {
        return try convertToDIDDocument(fromJson: String(contentsOfFile: fromFileAtPath, encoding: .utf8))
    }

    /// Get DID Document from data path.
    /// - Parameter url: The data path with URL.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: DIDDocument instance.
    public class func convertToDIDDocument(fromUrl url: URL) throws -> DIDDocument {
        return try convertToDIDDocument(fromJson: String(contentsOf: url, encoding: .utf8))
    }

    /*
     * Normalized serialization order:
     *
     * - id
     * + publickey
     *   + public keys array ordered by id(case insensitive/ascending)
     *     - id
     *     - type
     *     - controller
     *     - publicKeyBase58
     * + authentication
     *   - ordered by public key' ids(case insensitive/ascending)
     * + authorization
     *   - ordered by public key' ids(case insensitive/ascending)
     * + verifiableCredential
     *   - credentials array ordered by id(case insensitive/ascending)
     * + service
     *   + services array ordered by id(case insensitive/ascending)
     *     - id
     *     - type
     *     - endpoint
     * - expires
     * + proof
     *   - type
     *   - created
     *   - creator
     *   - signatureValue
     */
    private func toJson(_ generator: JsonGenerator, _ normalized: Bool, _ forSign: Bool) throws {
        generator.writeStartObject()

        // subject
        generator.writeFieldName(Constants.ID)
        generator.writeString(self.subject.toString())

        // publicKey
        generator.writeFieldName(Constants.PUBLICKEY)
        generator.writeStartArray()
        for pubKey in publicKeys() {
            pubKey.toJson(generator, self.subject, normalized)
        }
        generator.writeEndArray()

        // authentication
        generator.writeFieldName(Constants.AUTHENTICATION)
        generator.writeStartArray()
        for pubKey in authenticationKeys() {
            var value: String
            if normalized || pubKey.getId().did != self.subject {
                value = pubKey.getId().toString()
            } else {
                value = "#" + pubKey.getId().fragment!
            }
            generator.writeString(value)
        }
        generator.writeEndArray()

        if self.authorizationKeyCount > 0 {
            generator.writeFieldName(Constants.AUTHORIZATION)
            generator.writeStartArray()

            for pubKey in authorizationKeys() {
                var value: String
                if normalized || pubKey.getId().did != self.subject {
                    value = pubKey.getId().toString()
                } else {
                    value = "#" + pubKey.getId().fragment!
                }
                generator.writeString(value)
            }
            generator.writeEndArray()
        }

        // credential
        if self.credentialCount > 0 {
            generator.writeFieldName(Constants.VERIFIABLE_CREDENTIAL)
            generator.writeStartArray()
            for credential in credentials() {
                credential.toJson(generator, self.subject, normalized)
            }
            generator.writeEndArray()
        }

        if self.serviceCount > 0 {
            generator.writeFieldName(Constants.SERVICE)
            generator.writeStartArray()
            for service in services() {
                service.toJson(generator, self.subject, normalized)
            }
            generator.writeEndArray()
        }

        if let _ = self.expirationDate {
            generator.writeFieldName(Constants.EXPIRES)
            generator.writeString(DateFormatter.convertToUTCStringFromDate(self.expirationDate!))
        }

        if getProof() != nil && !forSign {
            generator.writeFieldName(Constants.PROOF)
            self.proof.toJson(generator, normalized)
        }

        generator.writeEndObject()
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) throws {
        return try toJson(generator, normalized, false)
    }

    func toJson(_ normalized: Bool, _ forSign: Bool) throws -> String {
        let generator = JsonGenerator()
        try toJson(generator, normalized, forSign)
        return generator.toString()
    }

    func toJson(_ normalized: Bool, _ forSign: Bool) throws -> Data {
        return try toJson(normalized, forSign).data(using: .utf8)!
    }

    /// Get json context from DID Document.
    /// - Parameter normalized: Json context is normalized or not.
    /// true represents normalized, false represents not compact.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: json context.
    public func convertFromDIDDocument(_ normalized: Bool) throws -> String {
        return try toJson(normalized, false)
    }

    /// Get json context from DID Document.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: json context.
    public func convertFromDIDDocument() throws -> String {
        return try toJson(false, false)
    }

    /// Get json context from DID Document.
    /// - Parameter normalized: Json context is normalized or not.
    /// true represents normalized, false represents not compact.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: json context.
    public func convertFromDIDDocument(_ normalized: Bool) throws -> Data {
        return try toJson(normalized, false)
    }

    /// Get data context from DID Document.
    /// - Throws: If no error occurs, throw error.
    /// - Returns: data context.
    public func convertFromDIDDocument() throws -> Data {
        return try toJson(false, false)
    }

    /// Write DIDDocument in path
    /// - Parameters:
    ///   - normalized: Json context is normalized or not.
    /// true represents normalized, false represents not compact.
    ///   - asFileAtPath: the path to write of string format.
    /// - Throws: If no error occurs, throw error.
    public func convertFromDIDDocument(_ normalized: Bool, asFileAtPath: String) throws {
        return try convertFromDIDDocument(normalized, asURL: URL.init(fileURLWithPath: asFileAtPath))
    }

    ///  Write DIDDocument in path.
    ///  Default  represents not compact.
    /// - Parameter asFileAtPath: the path to write of string format.
    /// - Throws: If no error occurs, throw error.
    public func convertFromDIDDocument(asFileAtPath: String) throws {
        return try convertFromDIDDocument(false, asFileAtPath: asFileAtPath)
    }

    /// Write DIDDocument in path.
    /// - Parameters:
    ///   - normalized: Json context is normalized or not.
    /// true represents normalized, false represents not compact.
    ///   - asURL: the path to write of URL format.
    /// - Throws: If no error occurs, throw error.
    public func convertFromDIDDocument(_ normalized: Bool, asURL: URL) throws {
        let data: Data = try convertFromDIDDocument(normalized)
        let fileManager = FileManager.default
        if !fileManager.fileExists(atPath: asURL.absoluteString) {
            let dirPath = PathExtracter(asURL.absoluteString).dirname()
            if !FileManager.default.fileExists(atPath: dirPath) {
                try fileManager.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
            }
            fileManager.createFile(atPath: asURL.absoluteString, contents: nil, attributes: nil)
        }
        let handle = try FileHandle(forWritingTo: asURL)
        handle.write(data)
    }

    /// Write DIDDocument in path.
    /// - Parameter asURL:  the path to write of URL format.
    /// - Throws: If no error occurs, throw error.
    public func convertFromDIDDocument(asURL: URL) throws {
        return try convertFromDIDDocument(false, asURL: asURL)
    }
}

extension DIDDocument: CustomStringConvertible {

    /// Get DID Document string from DIDDocument.
    /// Default  represents not compact.
    /// - Returns: DIDDocument string
    public func toString() -> String {
        return (try? toJson(false, false)) ?? ""
    }

    /// Get DID Document string from DIDDocument.
    /// Default  represents not compact.
    /// - Parameter force: Json context is normalized or not.
    /// true represents normalized, false represents not compact.
    /// - Returns: DIDDocument string
    public func toString(_ force: Bool) -> String {
        return (try? toJson(force, false)) ?? ""
    }

    /// Get DID Document string from DIDDocument.
    /// - Parameters:
    ///   - force: Json context is normalized or not.
    /// true represents normalized, false represents not compact.
    ///   - forSign: is sign or not
    /// - Returns: DIDDocument string
    public func toString(_ force: Bool, forSign: Bool) -> String {
        return (try? toJson(force, forSign)) ?? ""
    }

    /// DIDDocument string.
    /// Default  represents not compact.
    public var description: String {
        return toString()
    }
}
