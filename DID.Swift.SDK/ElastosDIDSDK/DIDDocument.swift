import Foundation

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
        for (id, publicKey) in doc._publicKeys! {
            if  self._publicKeys == nil  {
                self._publicKeys = Dictionary<DIDURL, PublicKey>()
            }
            self._publicKeys![id] = publicKey
        }
        for (id, publicKey) in doc._authenticationKeys! {
            if  self._authenticationKeys == nil {
                self._authenticationKeys = Dictionary<DIDURL, PublicKey>()
            }
            self._authenticationKeys![id] = publicKey
        }
        doc._authorizationKeys?.forEach { (id, publicKey) in
            if  self._authorizationKeys == nil {
                self._authorizationKeys = Dictionary<DIDURL, PublicKey>()
            }
            self._authorizationKeys![id] = publicKey
        }
        doc._credentials?.forEach { (did, credential) in
            if  self._credentials == nil {
                self._credentials = Dictionary<DIDURL, VerifiableCredential>()
            }
            self._credentials![did] = credential
        }
        doc._services?.forEach { (did, service) in
            if  self._services == nil {
                self._services = Dictionary<DIDURL, Service>()
            }
            self._services![did] = service
        }

        self._subject = doc.subject
        self._expirationDate = doc.expirationDate
        self._proof = doc.proof
        self._meta = doc.getMeta()
    }

    private func getEntry<T: DIDObject>(_ entry: Dictionary<DIDURL, T>?, _ id: DIDURL) -> T? {
        return entry?[id] ?? nil
    }

    private func getEntryCount<T: DIDObject>(_ entry: Dictionary<DIDURL, T>?) -> Int {
        return entry?.count ?? 0
    }

    private func getEntries<T: DIDObject>(_ entries: Dictionary<DIDURL, T>?) -> Array<T> {
        var result = Array<T>()
        entries?.values.forEach { entry in
            result.append(entry)
        }
        return result
    }

    private func selectEntries<T: DIDObject>(_ entries: Dictionary<DIDURL, T>?,
                                             _ id: DIDURL?,
                                             _ type: String?) throws -> Array<T> {
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
                    if !credential.getTypes()!.contains(type!) {
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

    public var subject: DID {
        return self._subject!
    }

    private func setSubject(_ subject: DID) {
        self._subject = subject
    }

    public var publicKeyCount: Int {
        return getEntryCount(self._publicKeys)
    }

    public var publicKeys: Array<PublicKey>? {
        return getEntries(self._publicKeys)
    }

    public func selectPublicKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectEntries(self._publicKeys, DIDURL(subject, byId), andType)
    }

    public func selectPublicKeys(byId: DIDURL, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectEntries(self._publicKeys, byId, andType)
    }

    public func publicKey(ofId: String) throws -> PublicKey? {
        return getEntry(self._publicKeys, try DIDURL(subject, ofId))
    }

    public func publicKey(ofId: DIDURL) -> PublicKey? {
        return getEntry(self._publicKeys, ofId)
    }

    public func containsPublicKey(forId: String) throws -> Bool {
        return try publicKey(ofId: forId) != nil
    }

    public func containsPublicKey(forId: DIDURL) -> Bool {
        return publicKey(ofId: forId) != nil
    }

    public func containsPrivateKey(forId: String) throws -> Bool {
        return try containsPrivateKey(forId: try DIDURL(self.subject, forId))
    }

    public func containsPrivateKey(forId: DIDURL) throws -> Bool {
        guard containsPublicKey(forId: forId) else {
            return false
        }
        guard getMeta().attachedStore else {
            return false
        }
        return try getMeta().store!.containsPrivateKey(self.subject, forId)
    }

    private func getDefaultPublicKey() -> DIDURL? {
        for publicKey in self._publicKeys!.values {
            if self.subject != publicKey.controller {
                continue
            }

            let address = DerivedKey.getAddress(publicKey.publicKeyBytes)
            guard address == self.subject.methodSpecificId else {
                return publicKey.getId()
            }
        }
        return nil
    }

    public var defaultPublicKey: DIDURL {
        return getDefaultPublicKey()!
    }

    func appendPublicKey(_ publicKey: PublicKey) -> Bool {
        if self._publicKeys == nil {
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
        guard self.defaultPublicKey != id else {
            return false
        }

        if force {
            _ = removeAuthenticationKey(id)
            _ = removeAuthorizationKey(id)
        } else {
            if  containsAuthenticationKey(forId: id) ||
                containsAuthorizationKey(forId: id) {
                return false
            }
        }

        let removedKey = self._publicKeys?.removeValue(forKey: id)
        if  removedKey != nil {
            _ = try? getMeta().store?.deletePrivateKey(self.subject, id)
        }

        return removedKey != nil
    }

    public var authenticationKeyCount: Int {
        return getEntryCount(self._authenticationKeys)
    }

    public func authenticationKeys() -> Array<PublicKey> {
        return getEntries(self._authenticationKeys)
    }

    public func selectAuthenticationKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectEntries(self._authenticationKeys, DIDURL(subject, byId), andType)
    }

    public func selectAuthenticationKeys(byId: DIDURL, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectEntries(self._authenticationKeys, byId, andType)
    }

    public func authenticationKey(ofId: String) throws -> PublicKey?  {
        return getEntry(self._authenticationKeys, try DIDURL(subject, ofId))
    }

    public func authenticationKey(ofId: DIDURL) -> PublicKey? {
        return getEntry(self._authenticationKeys, ofId)
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
        guard defaultPublicKey != key else {
            return false
        }

        return self._authenticationKeys?.removeValue(forKey: key) != nil
    }

    public var authorizationKeyCount: Int {
        return getEntryCount(self._authorizationKeys)
    }

    public func authorizationKeys() -> Array<PublicKey> {
        return getEntries(self._authorizationKeys)
    }

    public func selectAuthorizationKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectEntries(self._authorizationKeys, DIDURL(subject, byId), andType)
    }

    public func selectAuthorizationKeys(byId: DIDURL, andType: String? = nil) throws -> Array<PublicKey> {
        return try selectEntries(self._authenticationKeys, byId, andType)
    }

    public func authorizationKey(ofId: String) throws -> PublicKey?  {
        return getEntry(self._authorizationKeys, try DIDURL(subject, ofId))
    }

    public func authorizationKey(ofId: DIDURL) -> PublicKey? {
        return getEntry(self._authorizationKeys, ofId)
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
        guard defaultPublicKey != key else {
            return false
        }

        return self._authorizationKeys!.removeValue(forKey: key) != nil
    }

    public var credentialCount: Int {
        return getEntryCount(self._credentials)
    }

    public var credentials: Array<VerifiableCredential> {
        return getEntries(self._credentials)
    }

    public func selectCredentials(byId: String, andType: String? = nil) throws -> Array<VerifiableCredential>  {
        return try selectEntries(self._credentials, DIDURL(self.subject, byId), andType)
    }

    public func selectCredentials(byId: DIDURL, andType: String? = nil) throws -> Array<VerifiableCredential>  {
        return try selectEntries(self._credentials, byId, andType)
    }

    public func credential(ofId: String) throws -> VerifiableCredential? {
        return getEntry(self._credentials, try DIDURL(self.subject, ofId))
    }

    public func credential(ofId: DIDURL) -> VerifiableCredential? {
        return getEntry(self._credentials, ofId)
    }

    func appendCredential(_ credential: VerifiableCredential) -> Bool {
        // Make sure the verifiable credential must belong to current DID.
        guard credential.subject.did != self.subject else {
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
        return getEntryCount(self._services)
    }

    public var services: Array<Service> {
        return getEntries(self._services)
    }

    public func selectServices(byId: String, andType: String? = nil) throws -> Array<Service>  {
        return try selectEntries(self._services, try DIDURL(self.subject, byId), andType)
    }

    public func selectServices(byId: DIDURL, andType: String? = nil) throws -> Array<Service>  {
        return try selectEntries(self._services, byId, andType)
    }

    public func service(ofId: String) throws -> Service? {
        return getEntry(self._services, try DIDURL(self.subject, ofId))
    }

    public func service(ofId: DIDURL) -> Service? {
        return getEntry(self._services, ofId)
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
        if getMeta().attachedStore {
            try getMeta().store!.storeDidMeta(getMeta(), for: self.subject)
        }
    }

    public func getExtra(forName: String) -> String? {
        return getMeta().getExtra(forName)
    }

    private func setAliasName(_ newValue: String?) throws {
        getMeta().setAlias(newValue)
        if getMeta().attachedStore {
            try getMeta().store!.storeDidMeta(getMeta(), for: self.subject)
        }
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
        guard self.proof.creator != self.defaultPublicKey else {
            return false
        }
        // Unsupported public key type;
        guard self.proof.type != Constants.DEFAULT_PUBLICKEY_TYPE else {
            return false
        }

        do {
            let json = try toJson(true, true)
            var inputs: [Data] = []
            inputs.append(json.data(using: .utf8)!)

            return try verifyEx(proof.creator, proof.signature, inputs)
        } catch {
            return false
        }
    }

    public var isValid: Bool {
        return !isDeactivated && !isExpired && isGenuine
    }

    public func edit() -> DIDDocumentBuilder {
        return DIDDocumentBuilder(self)
    }

    public func sign(using storePass: String, _ data: Data...) throws -> String {
        return try signEx(self.defaultPublicKey, storePass, data)
    }

    public func sign(using id: String, storePass: String, _ data: Data...) throws -> String {
        return try signEx(try DIDURL(self.subject, id), storePass, data)
    }

    public func sign(using id: DIDURL, storePass: String, _ data: Data...) throws -> String {
        return try signEx(id, storePass, data)
    }

    func signEx(_ id: DIDURL, _ storePass: String, _ data: [Data]) throws -> String {
        guard data.count > 0 else {
            throw DIDError.illegalArgument()
        }
        guard !storePass.isEmpty else {
            throw DIDError.illegalArgument()
        }
        guard getMeta().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store")
        }

        return try getMeta().store!.signEx(self.subject, id, storePass, data)
    }

    public func verify(using signature: String, _ data: Data...) throws -> Bool {
        return try verifyEx(self.defaultPublicKey, signature, data)
    }

    public func verify(using id: DIDURL, signature: String, _ data: Data...) throws -> Bool {
        return try verifyEx(id, signature, data)
    }

    public func verify(using id: String, signature: String, _ data: Data...) throws -> Bool {
        return try verifyEx(DIDURL(self.subject, id), signature, data)
    }

    func verifyEx(_ id: DIDURL, _ sigature: String, _ data: [Data]) throws -> Bool {
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

    private func parse(_ doc: JsonNode) throws {
        let serializer = JsonSerializer(doc)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withHint("document subject")
        let did = try serializer.getDID(Constants.ID, options)
        setSubject(did)

        var arrayNode: [JsonNode]?
        arrayNode = doc.getArrayNode(Constants.PUBLICKEY)
        guard let _ = arrayNode else {
            throw DIDError.malformedDocument("missing publicKey")
        }
        try parsePublicKeys(arrayNode!)

        arrayNode = doc.getArrayNode(Constants.AUTHENTICATION)
        if let _ = arrayNode {
            try parseAuthenticationKeys(arrayNode!)
        }

        // Add default public key to authentication keys if need.
        let defaultKey = self.defaultPublicKey
        if containsAuthenticationKey(forId: defaultKey) {
            _ = appendAuthenticationKey(publicKey(ofId: defaultKey)!)
        }

        arrayNode = doc.getArrayNode(Constants.AUTHORIZATION)
        if let _ = arrayNode {
            try parseAuthorizationKeys(arrayNode!)
        }

        arrayNode = doc.getArrayNode(Constants.VERIFIABLE_CREDENTIAL)
        if let _ = arrayNode {
            try parseCredential(arrayNode!)
        }

        arrayNode = doc.getArrayNode(Constants.SERVICE)
        if let _ = arrayNode {
            try parseService(arrayNode!)
        }

        options = JsonSerializer.Options()
                                .withOptional()
                                .withHint("document expires")
        let expirationDate = try serializer.getDate(Constants.EXPIRES, options)
        self.setExpirationDate(expirationDate)

        var node: JsonNode?
        node = doc.getNode(Constants.PROOF)
        guard let _ = node else {
            throw DIDError.malformedDocument("missing document proof")
        }

        setProof(try DIDDocumentProof.fromJson(node!, defaultKey))
    }

    private func parsePublicKeys(_ arrayNode: [JsonNode]) throws {
        guard arrayNode.count < 0 else {
            throw DIDError.malformedDocument("invalid publicKeys, should not be empty.")
        }

        for node in arrayNode {
            do {
                _ = appendPublicKey(try PublicKey.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseAuthenticationKeys(_ arrayNode: [JsonNode]) throws {
        for node in arrayNode {
            do {
                _ = appendAuthenticationKey(try PublicKey.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseAuthorizationKeys(_ arrayNode: [JsonNode]) throws {
        for node in arrayNode {
            do {
                _ = appendAuthorizationKey(try PublicKey.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseCredential(_ arrayNode: [JsonNode]) throws {
        for node in arrayNode {
            do {
                _ = appendCredential(try VerifiableCredential.fromJson(node, self.subject))
            } catch {
                throw DIDError.malformedDocument()
            }
        }
    }

    private func parseService(_ arrayNode: [JsonNode]) throws {
        for node in arrayNode {
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

        let doc = DIDDocument()
        let node: Dictionary<String, Any>?
        do {
            node = try JSONSerialization.jsonObject(with: data, options: []) as? Dictionary<String, Any>
        } catch {
            throw DIDError.malformedDocument()
        }
        try doc.parse(JsonNode(node!))

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

    func toJson(_ normalized: Bool) throws -> String {
        return try toJson(normalized, false)
    }

    public func convertFromDIDDocument(_ normalized: Bool) throws -> String {
        return try toJson(normalized)
    }

    public func convertFromDIDDocument(_ normalized: Bool) throws -> Data {
        return try toJson(normalized).data(using: .utf8)!
    }
}

extension DIDDocument: CustomStringConvertible {
    func toString() -> String {
        return (try? toJson(false)) ?? ""
    }

    public var description: String {
        return toString()
    }
}
