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

    private func getEntry<T: DIDObject>(_ dict: Dictionary<DIDURL, T>?, _ id: DIDURL) -> T? {
        return dict?[id] ?? nil
    }

    private func getEntryCount<T: DIDObject>(_ dict: Dictionary<DIDURL, T>?) -> Int {
        return dict?.count ?? 0
    }

    private func getEntries<T: DIDObject>(_ dict: Dictionary<DIDURL, T>?) -> Array<T>? {
        var pkArray: Array<T>? = nil
        dict?.values.forEach { publicKey in
            if  pkArray == nil {
                pkArray = Array<T>()
            }
            pkArray!.append(publicKey)
        }
        return pkArray
    }

    private func selectEntries<T: DIDObject>(_ dict: Dictionary<DIDURL, T>?,
                                             _ id: DIDURL?,
                                             _ type: String?) -> Array<T>? {
        // TODO
        return nil
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

    public func selectPublicKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey>? {
        return selectEntries(self._publicKeys, try DIDURL(subject, byId), andType)
    }

    public func selectPublicKeys(byId: DIDURL, andType: String? = nil) -> Array<PublicKey>? {
        return selectEntries(self._publicKeys, byId, andType)
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
        guard getMeta().hasAttachedStore else {
            return false
        }
        return try getMeta().store!.containsPrivateKey(self.subject, forId)
    }

    public var defaultPublicKey: DIDURL {
        for publicKey in self._publicKeys!.values {
            if self.subject != publicKey.controller {
                continue
            }

            let address = DerivedKey.getAddress(publicKey.publicKeyBytes)
            guard address == self.subject.methodSpecificId else {
                return publicKey.getId()
            }
        }
    }

    func appendPublicKey(_ publicKey: PublicKey) {
        if self._publicKeys == nil {
            self._publicKeys = Dictionary<DIDURL, PublicKey>()
        }

        for key in self._publicKeys!.values {
            // Check the existance, by both DIDURL (id) and keyBase58
            if key.getId() == publicKey.getId() ||
               key.publicKeyBase58 == publicKey.publicKeyBase58 {
                return
            }
        }

        self._publicKeys![publicKey.getId()] = publicKey
    }

    func removePublicKey(atId id: DIDURL, _ force: Bool) throws {
        // Can not remove default public key
        guard self.defaultPublicKey != id else {
            throw DIDError.illegalArgument("Can be default public Key.")
        }

        if force {
            _ = try removeAuthenticationKey(atId: id)
            _ = try removeAuthorizationKey(atId: id)
        } else {
            if  containsAuthenticationKey(forId: id) ||
                containsAuthorizationKey(forId: id) {
                throw DIDError.illegalArgument("id is being used.")
            }
        }

        let removedKey = self._publicKeys?.removeValue(forKey: id)
        if  removedKey != nil && getMeta().hasAttachedStore {
            _ = try getMeta().store!.deletePrivateKey(self.subject, id)
        }
    }

    public var authenticationKeyCount: Int {
        return getEntryCount(self._authenticationKeys)
    }

    public func authenticationKeys() -> Array<PublicKey>? {
        return getEntries(self._authenticationKeys)
    }

    public func selectAuthenticationKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey>? {
        return selectEntries(self._authenticationKeys, try DIDURL(subject, byId), andType)
    }

    public func selectAuthenticationKeys(byId: DIDURL, andType: String? = nil) -> Array<PublicKey>? {
        return selectEntries(self._authenticationKeys, byId, andType)
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

    func appendAuthenticationKey(_ publicKey: PublicKey) throws {
        // Make sure that controller should be current DID subject.
        guard publicKey.controller == self.subject else {
            throw DIDError.illegalArgument("Controller is not document subject.")
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
            throw DIDError.illegalArgument("PublicKey conflicted.")
        } else {
            // Prefer using publicKey of DIDDocument publicKeys.
            refKey = key!
        }

        if  self._authenticationKeys == nil {
            self._authenticationKeys = Dictionary<DIDURL, PublicKey>()
        }

        if containsAuthenticationKey(forId: refKey.getId()) {
            throw DIDError.illegalArgument("Already exist authentication Key with same Id.")
        }

        self._authenticationKeys![refKey.getId()] = refKey
    }

    func removeAuthenticationKey(atId publicKey: DIDURL) throws {
        // Can not remove default publicKey.
        guard defaultPublicKey != publicKey else {
            throw DIDError.illegalArgument("Can not remove default publicKey.")
        }

        _ = self._authenticationKeys?.removeValue(forKey: publicKey)
    }

    public var authorizationKeyCount: Int {
        return getEntryCount(self._authorizationKeys)
    }

    public func authorizationKeys() -> Array<PublicKey>? {
        return getEntries(self._authorizationKeys)
    }

    public func selectAuthorizationKeys(byId: String, andType: String? = nil) throws -> Array<PublicKey>? {
        return selectEntries(self._authorizationKeys, try DIDURL(subject, byId), andType)
    }

    public func selectAuthorizationKeys(byId: DIDURL, andType: String? = nil) -> Array<PublicKey>? {
        return selectEntries(self._authenticationKeys, byId, andType)
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

    func appendAuthorizationKey(_ publicKey: PublicKey) throws {
        // Make sure that controller should be current DID subject.
        guard publicKey.controller == self.subject else {
            throw DIDError.illegalArgument("Controller is not document subject.")
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
            throw DIDError.illegalArgument("PublicKey conflicted.")
        } else {
            // Prefer using publicKey of DIDDocument publicKeys.
            refKey = key!
        }

        if  self._authorizationKeys == nil {
            self._authorizationKeys = Dictionary<DIDURL, PublicKey>()
        }

        if containsAuthorizationKey(forId: refKey.getId()) {
            throw DIDError.illegalArgument("Already exist authorization Key with same Id.")
        }

        self._authorizationKeys![refKey.getId()] = refKey
    }

    func removeAuthorizationKey(atId: DIDURL) throws {
        // Can not remove default publicKey.
        guard defaultPublicKey != atId else {
            throw DIDError.illegalArgument("Can not remove default publicKey.")
        }

        _ = self._authorizationKeys!.removeValue(forKey: atId)
    }

    public var credentialCount: Int {
        return getEntryCount(self._credentials)
    }

    public var credentials: Array<VerifiableCredential>? {
        return getEntries(self._credentials)
    }

    public func selectCredentials(byId: String, andType: String? = nil) throws -> Array<VerifiableCredential>?  {
        return selectEntries(self._credentials, try DIDURL(self.subject, byId), andType)
    }

    public func selectCredentials(byId: DIDURL, andType: String? = nil) -> Array<VerifiableCredential>?  {
        return selectEntries(self._credentials, byId, andType)
    }

    public func credential(ofId: String) throws -> VerifiableCredential? {
        return getEntry(self._credentials, try DIDURL(self.subject, ofId))
    }

    public func credential(ofId: DIDURL) -> VerifiableCredential? {
        return getEntry(self._credentials, ofId)
    }

    func appendCredential(_ credential: VerifiableCredential) throws {
        // Make sure the verifiable credential must belong to current DID.
        guard credential.subject.did != self.subject else {
            throw DIDError.illegalArgument("Credential subject shoud be document subject.")
        }

        if  self._credentials == nil {
            self._credentials = Dictionary<DIDURL, VerifiableCredential>()
        }

        guard self._credentials!.keys.contains(credential.getId()) else {
            throw DIDError.illegalArgument("Already exist credential with same Id.")
        }

        self._credentials![credential.getId()] = credential
    }

    func removeCredential(atId: DIDURL) {
        _ = self._credentials?.removeValue(forKey: atId)
    }

    public var serviceCount: Int {
        return getEntryCount(self._services)
    }

    public var services: Array<Service>? {
        return getEntries(self._services)
    }

    public func selectServices(byId: String, andType: String? = nil) throws -> Array<Service>?  {
        return selectEntries(self._services, try DIDURL(self.subject, byId), andType)
    }

    public func selectServices(byId: DIDURL, andType: String? = nil) -> Array<Service>?  {
        return selectEntries(self._services, byId, andType)
    }

    public func service(ofId: String) throws -> Service? {
        return getEntry(self._services, try DIDURL(self.subject, ofId))
    }

    public func service(ofId: DIDURL) -> Service? {
        return getEntry(self._services, ofId)
    }

    func appendService(_ service: Service) throws {
        if  self._services == nil {
            self._services = Dictionary<DIDURL, Service>()
        }

        guard self._services!.keys.contains(service.getId()) else { // TODO:
            throw DIDError.illegalArgument("Already exist service with same Id.")
        }

        self._services![service.getId()] = service
    }

    func removeService(atId: DIDURL) {
        _ = self._services?.removeValue(forKey: atId)
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
        if getMeta().hasAttachedStore {
            try getMeta().store!.storeDidMeta(getMeta(), for: self.subject)
        }
    }

    public func getExtra(forName: String) -> String? {
        return getMeta().getExtra(forName)
    }

    private func setAliasName(_ newValue: String?) throws {
        getMeta().setAlias(newValue)
        if getMeta().hasAttachedStore {
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
        // TODO:
        return false
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
        guard getMeta().hasAttachedStore else {
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

        // TODO:
        return false
    }

    private func parse(_ doc: Dictionary<String, Any>) throws {
        let jsonDict = JsonSerializer(doc)
        let did = try jsonDict.getDID(Constants.ID, JsonSerializer.Options<DID>()
                                    .withHint("subject"))
        setSubject(did!)

        var node = doc[Constants.PUBLICKEY] as? Dictionary<String, Any>
        guard let _ = node else {
            throw DIDError.malformedDocument("missing publicKey")
        }
        try parsePublicKeys(node!)

        node = doc[Constants.AUTHENTICATION] as? Dictionary<String, Any>
        if let _ = node {
            try parseAuthenticationKeys(node!)
        }

        let defaultKey = self.defaultPublicKey
        if containsAuthenticationKey(forId: defaultKey) {
            try appendAuthenticationKey(publicKey(ofId: defaultKey)!)  // TODO:
        }

        node = doc[Constants.AUTHORIZATION] as? Dictionary<String, Any>
        if let _ = node {
            try parseAuthorizationKeys(node!)
        }

        node = doc[Constants.VERIFIABLE_CREDENTIAL] as? Dictionary<String, Any>
        guard let _ = node else {
            try parseCredential(node!)
        }

        node = doc[Constants.SERVICE] as? Dictionary<String, Any>
        guard let _ = node else {
            try parseService(node!)
        }

        let expirationDate = try jsonDict.getDate(Constants.EXPIRES,
                                    JsonSerializer.Options<Date>()
                                        .withOptional()
                                        .withHint("expires"))
        self.setExpirationDate(expirationDate)

        node = doc[Constants.PROOF] as? Dictionary<String, Any>
        guard let _ = node else {
            throw DIDError.malformedDocument("Missing proof")
        }
        setProof(try DIDDocumentProof.fromJson(node!, defaultKey))
    }

    private func parsePublicKeys(_ node: Dictionary<String, Any>) throws {
        // TODO:
    }

    private func parseAuthenticationKeys(_ node: Dictionary<String, Any>) throws {
        // TODO:
    }

    private func parseAuthorizationKeys(_ node: Dictionary<String, Any>) throws {
        // TODO
    }

    private func parseCredential(_ node: Dictionary<String, Any>) throws {
        // TODO
    }

    private func parseService(_ node: Dictionary<String, Any>) throws {
        // TODO
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
        try doc.parse(node!)
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

        // subject/id
        generator.writeFieldName(Constants.ID)
        generator.writeString(self.subject.toString())

        // publicKey
        generator.writeFieldName(Constants.PUBLICKEY)
        generator.writeStartArray()
        for pubKey in self._publicKeys!.values {
            pubKey.toJson(generator, self.subject, normalized)
        }
        generator.writeEndArray()

        // authentication
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

        // authorization
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

        // verifiable credential
        if self.credentialCount > 0 {
            generator.writeFieldName(Constants.VERIFIABLE_CREDENTIAL)
            generator.writeStartArray()
            for credential in self._credentials!.values {
                credential.toJson(generator, self.subject, normalized)
            }
            generator.writeEndArray()
        }

        // service
        if self.serviceCount > 0 {
            generator.writeFieldName(Constants.SERVICE)
            generator.writeStartArray()
            for service in self._services!.values {
                service.toJson(generator, self.subject, normalized)
            }
            generator.writeEndArray()
        }

        // expires
        if let _ = self.expirationDate {
            generator.writeFieldName(Constants.EXPIRES)
            // TODO: try generator.writeString(JsonHelper.format)
        }

        // proof
        if !forSign { // TODO: check
            generator.writeFieldName(Constants.PROOF)
            proof.toJson(generator, normalized)
        }

        generator.writeEndObject()
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) throws {
        return try toJson(generator, normalized, false)
    }

    func toJson(_ normalized: Bool, _ forSign: Bool) throws -> String {
        // TODO
        return "TODO"
    }

    func toJson(_ normalized: Bool) throws -> String {
        return try toJson(normalized, false)
    }

    public func convertFromDIDDocumentToJsonString(_ normalized: Bool) throws -> String {
        // TODO:
        return "TODO"
    }

    public func convertFromDIDDocumentToData(_ normalized: Bool) throws -> Data {
        // TODO:
        return Data()
    }

    // TODO:
}

extension DIDDocument: CustomStringConvertible {
    func toString() -> String {
        return (try? toJson(false)) ?? ""
    }

    public var description: String {
        return toString()
    }
}
