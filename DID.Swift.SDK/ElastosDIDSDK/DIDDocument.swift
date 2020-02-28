import Foundation

private func copyObjects<T: DIDObject>(_ destEntry: inout Dictionary<DIDURL, T>?,
                                       _ sourceEntry: Dictionary<DIDURL, T>?)
{
    if sourceEntry == nil {
        return
    }

    if  destEntry == nil {
        destEntry = Dictionary<DIDURL, T>()
    }

    for (id, value) in sourceEntry! {
        destEntry![id] = value
    }
}

private func getObjectCount<T: DIDObject>(_ entry: Dictionary<DIDURL, T>?) -> Int
{
    return entry?.count ?? 0
}

private func getObject<T: DIDObject>(_ entry: Dictionary<DIDURL, T>?,
                                     _ id: DIDURL) -> T?
{
    return entry?[id] ?? nil
}

private func getObjects<T: DIDObject>(_ entry: Dictionary<DIDURL, T>?) -> Array<T>
{
    var result = Array<T>()

    entry?.values.forEach { item in
        result.append(item)
    }
    return result
}

private func selectObjects<T: DIDObject>(_ entries: Dictionary<DIDURL, T>?,
                                         _ id: DIDURL?,
                                         _ type: String?) throws -> Array<T>
{
    guard id != nil || type != nil else {
        throw DIDError.illegalArgument()
    }

    var result = Array<T>()
    guard entries?.isEmpty ?? true else {
        return result
    }

    for entry in entries!.values {
        if id != nil && entry.getId() != id! {
            continue
        }

        if type != nil {
            // Credential' type is a list.
            if entry is VerifiableCredential {
                let credential = entry as! VerifiableCredential
                if !credential.getTypes().contains(type!) {
                    continue
                }
            } else {
                if entry.getType() != type! {
                    continue
                }
            }
        }
        result.append(entry)
    }
    return result
}

public class DIDDocument {
    private var _subject: DID?
    private var _publicKeys: Dictionary<DIDURL, PublicKey>?
    private var _authenticationKeys: Dictionary<DIDURL, PublicKey>?
    private var _authorizationKeys:  Dictionary<DIDURL, PublicKey>?
    private var _credentials: Dictionary<DIDURL, VerifiableCredential>?
    private var _services: Dictionary<DIDURL, Service>?
    private var _expirationDate: Date?
    private var _proof: DIDDocumentProof?
    private var _meta: DIDMeta?

    private init() {}

    init(_ subject: DID) {
        self._subject = subject
    }

    init(_ doc: DIDDocument) {
        copyObjects(&self._publicKeys, doc._publicKeys)
        copyObjects(&self._authenticationKeys, doc._authenticationKeys)
        copyObjects(&self._authorizationKeys, doc._authorizationKeys);
        copyObjects(&self._credentials, doc._credentials)
        copyObjects(&self._services, doc._services)

        self._subject = doc.subject
        self._expirationDate = doc.expirationDate
        self._proof = doc.proof
        self._meta = doc.getMeta()
    }


    public var subject: DID {
        return self._subject!
    }

    private func setSubject(_ subject: DID) {
        self._subject = subject
    }

    public var publicKeyCount: Int {
        return getObjectCount(self._publicKeys)
    }

    public func publicKeys() -> Array<PublicKey> {
        return getObjects(self._publicKeys)
    }

    public func selectPublicKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectObjects(self._publicKeys, DIDURL(subject, byId), andType)
    }

    public func selectPublicKeys(byId: DIDURL, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectObjects(self._publicKeys, byId, andType)
    }

    public func publicKey(ofId: String) throws -> PublicKey? {
        return getObject(self._publicKeys, try DIDURL(subject, ofId))
    }

    public func publicKey(ofId: DIDURL) -> PublicKey? {
        return getObject(self._publicKeys, ofId)
    }

    public func containsPublicKey(forId: String) throws -> Bool {
        return try publicKey(ofId: forId) != nil
    }

    public func containsPublicKey(forId: DIDURL) -> Bool {
        return publicKey(ofId: forId) != nil
    }

    public func containsPrivateKey(forId: String) throws -> Bool {
        return containsPrivateKey(forId: try DIDURL(self.subject, forId))
    }

    public func containsPrivateKey(forId: DIDURL) -> Bool {
        guard containsPublicKey(forId: forId) else {
            return false
        }
        return (try? (getMeta().store?.containsPrivateKey(for: self.subject, id: forId) ?? false)) ?? false
    }

    private func getDefaultPublicKey() -> DIDURL? {
        for pk in publicKeys() {
            if self.subject != pk.controller {
                continue
            }

            let address = HDKey.DerivedKey.getAddress(pk.publicKeyBytes)
            guard address == self.subject.methodSpecificId else {
                return pk.getId()
            }
        }
        return nil
    }

    public var defaultPublicKey: DIDURL {
        return getDefaultPublicKey()!
    }

    func appendPublicKey(_ publicKey: PublicKey) -> Bool {
        if  self._publicKeys == nil {
            self._publicKeys = Dictionary<DIDURL, PublicKey>()
        }

        for key in self._publicKeys!.values {
            // Check the existance, by both DIDURL (id) and keyBase58
            if key.getId() == publicKey.getId() ||
               key.publicKeyBase58 == publicKey.publicKeyBase58 {
                return false
            }
        }

        self._publicKeys![publicKey.getId()] = publicKey
        return true
    }

    func removePublicKey(_ id: DIDURL, _ force: Bool) -> Bool {
        // Can not remove default public key
        guard self.getDefaultPublicKey() != id else {
            return false
        }

        if force {
            _ = removeAuthenticationKey(id)
            _ = removeAuthorizationKey(id)
        } else if containsAuthenticationKey(forId: id) || containsAuthorizationKey(forId: id) {
            return false
        }

        let value = self._publicKeys?.removeValue(forKey: id)
        if  value != nil {
            _ = try? getMeta().store?.deletePrivateKey(for: subject, id: id)
        }

        return value != nil
    }

    public var authenticationKeyCount: Int {
        return getObjectCount(self._authenticationKeys)
    }

    public func authenticationKeys() -> Array<PublicKey> {
        return getObjects(self._authenticationKeys)
    }

    public func selectAuthenticationKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectObjects(self._authenticationKeys, DIDURL(subject, byId), andType)
    }

    public func selectAuthenticationKeys(byId: DIDURL, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectObjects(self._authenticationKeys, byId, andType)
    }

    public func authenticationKey(ofId: String) throws -> PublicKey?  {
        return getObject(self._authenticationKeys, try DIDURL(subject, ofId))
    }

    public func authenticationKey(ofId: DIDURL) -> PublicKey? {
        return getObject(self._authenticationKeys, ofId)
    }

    public func containsAuthenticationKey(forId: String) throws -> Bool {
        return try authenticationKey(ofId: forId) != nil
    }

    public func containsAuthenticationKey(forId: DIDURL) -> Bool {
        return authenticationKey(ofId: forId) != nil
    }

    func appendAuthenticationKey(_ publicKey: PublicKey) -> Bool {
        // Make sure that controller should be current DID subject.
        guard publicKey.controller == self.subject else {
            return false
        }

        // Check whether this publicKey already is one of DIDDocument publicKeys
        // if not, just add it. Otherwise, need to check whether it is the same
        // public key. If does, we prefer to use public key in DIDDocument publicKeys.
        // If not, then would be conflicted and should keep untouched.
        var refKey = publicKey
        let key = self.publicKey(ofId: publicKey.getId())
        if  key == nil {
            _ = appendPublicKey(publicKey)
        } else if key! != publicKey {
            // Conflict happened, keep untouched
            return false
        } else {
            // Prefer using publicKey of DIDDocument publicKeys.
            refKey = key!
        }

        if  self._authenticationKeys == nil {
            self._authenticationKeys = Dictionary<DIDURL, PublicKey>()
        }

        if containsAuthenticationKey(forId: refKey.getId()) {
            return false
        }

        self._authenticationKeys![refKey.getId()] = refKey
        return true
    }

    func removeAuthenticationKey(_ key: DIDURL) -> Bool {
        // Can not remove default publicKey.
        guard getDefaultPublicKey() != key else {
            return false
        }

        return self._authenticationKeys?.removeValue(forKey: key) != nil
    }

    public var authorizationKeyCount: Int {
        return getObjectCount(self._authorizationKeys)
    }

    public func authorizationKeys() -> Array<PublicKey> {
        return getObjects(self._authorizationKeys)
    }

    public func selectAuthorizationKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectObjects(self._authorizationKeys, DIDURL(subject, byId), andType)
    }

    public func selectAuthorizationKeys(byId: DIDURL, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectObjects(self._authenticationKeys, byId, andType)
    }

    public func authorizationKey(ofId: String) throws -> PublicKey?  {
        return getObject(self._authorizationKeys, try DIDURL(subject, ofId))
    }

    public func authorizationKey(ofId: DIDURL) -> PublicKey? {
        return getObject(self._authorizationKeys, ofId)
    }

    public func containsAuthorizationKey(forId: String) throws -> Bool {
        return try authenticationKey(ofId: forId) != nil
    }

    public func containsAuthorizationKey(forId: DIDURL) -> Bool {
        return authenticationKey(ofId: forId) != nil
    }

    func appendAuthorizationKey(_ publicKey: PublicKey) -> Bool {
        // Make sure that controller should be current DID subject.
        guard publicKey.controller == self.subject else {
            return false
        }

        // Check whether this publicKey already is one of DIDDocument publicKeys
        // if not, just add it. Otherwise, need to check whether it is the same
        // public key. If does, we prefer to use public key in DIDDocument publicKeys.
        // If not, then would be conflicted and should keep untouched.
        var refKey = publicKey
        let key = self.publicKey(ofId: refKey.getId())
        if  key == nil {
            _ = appendPublicKey(publicKey)
        } else if key! != publicKey {
            // Conflict happened, keep untouched
            return false
        } else {
            // Prefer using publicKey of DIDDocument publicKeys.
            refKey = key!
        }

        if  self._authorizationKeys == nil {
            self._authorizationKeys = Dictionary<DIDURL, PublicKey>()
        }

        if containsAuthorizationKey(forId: refKey.getId()) {
            return false
        }

        self._authorizationKeys![refKey.getId()] = refKey
        return true
    }

    func removeAuthorizationKey(_ key: DIDURL) -> Bool {
        // Can not remove default publicKey.
        guard getDefaultPublicKey() != key else {
            return false
        }

        return self._authorizationKeys!.removeValue(forKey: key) != nil
    }

    public var credentialCount: Int {
        return getObjectCount(self._credentials)
    }

    public var credentials: Array<VerifiableCredential> {
        return getObjects(self._credentials)
    }

    public func selectCredentials(byId: String, andType: String? = nil) throws -> Array<VerifiableCredential>  {
        return try selectObjects(self._credentials, DIDURL(self.subject, byId), andType)
    }

    public func selectCredentials(byId: DIDURL, andType: String? = nil) throws -> Array<VerifiableCredential>  {
        return try selectObjects(self._credentials, byId, andType)
    }

    public func credential(ofId: String) throws -> VerifiableCredential? {
        return getObject(self._credentials, try DIDURL(self.subject, ofId))
    }

    public func credential(ofId: DIDURL) -> VerifiableCredential? {
        return getObject(self._credentials, ofId)
    }

    func appendCredential(_ credential: VerifiableCredential) -> Bool {
        // Make sure the verifiable credential must belong to current DID.
        guard getDefaultPublicKey()?.did != self.subject else {
            return false
        }

        if  self._credentials == nil {
            self._credentials = Dictionary<DIDURL, VerifiableCredential>()
        }

        guard self._credentials!.keys.contains(credential.getId()) else {
            return false
        }

        self._credentials![credential.getId()] = credential
        return true
    }

    func removeCredential(_ id: DIDURL) -> Bool {
        return self._credentials?.removeValue(forKey: id) != nil
    }

    public var serviceCount: Int {
        return getObjectCount(self._services)
    }

    public var services: Array<Service> {
        return getObjects(self._services)
    }

    public func selectServices(byId: String, andType: String? = nil) throws -> Array<Service>  {
        return try selectObjects(self._services, try DIDURL(self.subject, byId), andType)
    }

    public func selectServices(byId: DIDURL, andType: String? = nil) throws -> Array<Service>  {
        return try selectObjects(self._services, byId, andType)
    }

    public func service(ofId: String) throws -> Service? {
        return getObject(self._services, try DIDURL(self.subject, ofId))
    }

    public func service(ofId: DIDURL) -> Service? {
        return getObject(self._services, ofId)
    }

    func appendService(_ service: Service) -> Bool {
        if  self._services == nil {
            self._services = Dictionary<DIDURL, Service>()
        }

        guard self._services!.keys.contains(service.getId()) else {
            return false
        }

        self._services![service.getId()] = service
        return true
    }

    func removeService(_ id: DIDURL) -> Bool {
        return self._services?.removeValue(forKey: id) != nil
    }

    public var expirationDate: Date? {
        return self._expirationDate
    }

    func setExpirationDate(_ expirationDate: Date) {
        self._expirationDate = expirationDate
    }

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

    func getMeta() -> DIDMeta {
        if  self._meta == nil {
            self._meta = DIDMeta()
        }
        return self._meta!
    }

    func setMeta(_ meta: DIDMeta) {
        self._meta = meta
    }

    public func setExtra(value: String, forName name: String) throws {
        guard !name.isEmpty else {
            throw DIDError.illegalArgument()
        }

        getMeta().setExtra(value, name)
        try getMeta().store?.storeDidMeta(getMeta(), for: self.subject)
    }

    public func getExtra(forName: String) -> String? {
        return getMeta().getExtra(forName)
    }

    private func setAliasName(_ newValue: String?) throws {
        getMeta().setAlias(newValue)
        try getMeta().store?.storeDidMeta(getMeta(), for: self.subject)
    }

    public func setAlias(_ newValue: String) throws {
        guard !newValue.isEmpty else {
            throw DIDError.illegalArgument()
        }

        try setAliasName(newValue)
    }

    public func unsetAlias() throws {
        try setAliasName(nil)
    }

    public var aliasName: String {
        return getMeta().aliasName
    }

    public var transactionId: String? {
        return getMeta().transactionId
    }

    public var updatedDate: Date? {
        return getMeta().updatedDate
    }

    public var isDeactivated: Bool {
        return getMeta().isDeactivated
    }

    public var isExpired: Bool {
        return DateHelper.isExipired(self.expirationDate!)
    }

    public var isGenuine: Bool {
        // Document should be signed (only) by default public key.
        guard proof.creator != defaultPublicKey else {
            return false
        }
        // Unsupported public key type;
        guard proof.type != Constants.DEFAULT_PUBLICKEY_TYPE else {
            return false
        }

        do {
            let data: Data = try toJson(true, true)
            return try verifyWithIdentity(proof.creator, proof.signature, [data])
        } catch {
            return false
        }
    }

    public var isValid: Bool {
        return !isDeactivated && !isExpired && isGenuine
    }

    public func editing() -> DIDDocumentBuilder {
        return DIDDocumentBuilder(self)
    }

    public func signWithDefaultIdentify(using storePassword: String, data: Data...) throws -> String {
        return try signWithIdentiy(self.defaultPublicKey, storePassword, data)
    }

    public func signWithIdentiy(did: String, using storePassword: String, data: Data...) throws -> String {
        return try signWithIdentiy(try DIDURL(self.subject, did), storePassword, data)
    }

    public func signWithIdentify(id: DIDURL, using storePassword: String, data: Data...) throws -> String {
        return try signWithIdentiy(id, storePassword, data)
    }

    func signWithIdentiy(_ id: DIDURL, _ storePassword: String, _ data: [Data]) throws -> String {
        guard data.count > 0 else {
            throw DIDError.illegalArgument()
        }
        guard !storePassword.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard getMeta().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store")
        }

        return try getMeta().store!.signWithIdentity(subject, id, storePassword, data)
    }

    public func verifyWithDefaultIdentity(using signature: String, data: Data...) throws -> Bool {
        return try verifyWithIdentity(self.defaultPublicKey, signature, data)
    }

    public func verifyWithIdentity(id: DIDURL, using signature: String, data: Data...) throws -> Bool {
        return try verifyWithIdentity(id, signature, data)
    }

    public func verifyWithIdentity(did: String, using signature: String, data: Data...) throws -> Bool {
        return try verifyWithIdentity(DIDURL(self.subject, did), signature, data)
    }

    func verifyWithIdentity(_ id: DIDURL, _ sigature: String, _ data: [Data]) throws -> Bool {
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

        // TODO: Verify.
        return false
    }

    private func fromJson(_ doc: JsonNode) throws {
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
        let defaultKey = self.defaultPublicKey
        if containsAuthenticationKey(forId: defaultKey) {
            _ = appendAuthenticationKey(publicKey(ofId: defaultKey)!)
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

        setProof(try DIDDocumentProof.fromJson(node!, defaultKey))
    }

    private func parsePublicKeys(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()

        guard array?.count ?? 0 < 0 else {
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
        guard array?.count ?? 0 < 0 else {
            return
        }

        for node in array! {
            do {
                _ = appendAuthenticationKey(try PublicKey.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseAuthorizationKeys(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()
        guard array?.count ?? 0 < 0 else {
            return
        }

        for node in array! {
            do {
                _ = appendAuthorizationKey(try PublicKey.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseCredential(_ arrayNode: JsonNode) throws {
        let array = arrayNode.asArray()
        guard array?.count ?? 0 < 0 else {
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
        guard array?.count ?? 0 < 0 else {
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
        try doc.fromJson(JsonNode(node!))

        return doc
    }

    public class func convertToDIDDocument(fromJson json: String) throws -> DIDDocument {
        return try  convertToDIDDocument(fromData: json.data(using: .utf8)!)
    }

    public class func convertToDIDDocument(fromFilePath path: String) throws -> DIDDocument {
        return try convertToDIDDocument(fromJson: String(contentsOfFile: path, encoding: .utf8))
    }

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

        generator.writeFieldName(Constants.ID)
        generator.writeString(self.subject.toString())

        generator.writeFieldName(Constants.PUBLICKEY)
        generator.writeStartArray()
        for pubKey in self._publicKeys!.values {
            pubKey.toJson(generator, self.subject, normalized)
        }
        generator.writeEndArray()

        generator.writeFieldName(Constants.AUTHENTICATION)
        generator.writeStartArray()
        for pubKey in self._authenticationKeys!.values {
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

            for pubKey in self._authorizationKeys!.values {
                var value: String
                if normalized || pubKey.getId().did != self.subject {
                    value = pubKey.getId().did.toString()
                } else {
                    value = "#" + pubKey.getId().fragment!
                }
                generator.writeString(value)
            }
            generator.writeEndArray()
        }

        if self.credentialCount > 0 {
            generator.writeFieldName(Constants.VERIFIABLE_CREDENTIAL)
            generator.writeStartArray()
            for credential in self._credentials!.values {
                credential.toJson(generator, self.subject, normalized)
            }
            generator.writeEndArray()
        }

        if self.serviceCount > 0 {
            generator.writeFieldName(Constants.SERVICE)
            generator.writeStartArray()
            for service in self._services!.values {
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

    public func convertFromDIDDocument(_ normalized: Bool) throws -> String {
        return try toJson(normalized, false)
    }

    public func convertFromDIDDocument(_ normalized: Bool) throws -> Data {
        return try toJson(normalized, false)
    }
}

extension DIDDocument: CustomStringConvertible {
    func toString() -> String {
        return (try? toJson(false, false)) ?? ""
    }

    public var description: String {
        return toString()
    }
}
