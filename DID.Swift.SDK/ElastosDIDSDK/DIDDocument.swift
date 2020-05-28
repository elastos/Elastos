import Foundation

public class DIDDocument {
    private static let TAG = "DIDDocument"
    private var _subject: DID?
    private var _expirationDate: Date?
    private var _proof: DIDDocumentProof?
    private var _meta: DIDMeta?

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
        self._meta = doc.getMeta()
    }

    public var subject: DID {
        return self._subject!
    }

    private func setSubject(_ subject: DID) {
        self._subject = subject
    }

    public var publicKeyCount: Int {
        return self.publicKeyMap.count() { value -> Bool in return true }
    }

    public func publicKeys() -> Array<PublicKey> {
        return self.publicKeyMap.values() { value -> Bool in return true }
    }

    public func selectPublicKeys(byId: DIDURL, andType: String?) -> Array<PublicKey> {
        return self.publicKeyMap.select(byId, andType) { value -> Bool in return true }
    }

    public func selectPublicKeys(byId: String, andType: String?) throws -> Array<PublicKey> {
        let id = try DIDURL(subject, byId)
        return selectPublicKeys(byId: id, andType: andType)
    }

    public func selectPublicKeys(byType: String) -> Array<PublicKey> {
        return self.publicKeyMap.select(nil, byType) { value -> Bool in return true }
    }

    public func publicKey(ofId: DIDURL) -> PublicKey? {
        return self.publicKeyMap.get(forKey: ofId) { value -> Bool in return true }
    }

    public func publicKey(ofId: String) throws -> PublicKey? {
        return publicKey(ofId: try DIDURL(subject, ofId))
    }

    public func containsPublicKey(forId: DIDURL) -> Bool {
        return publicKey(ofId: forId) != nil
    }

    public func containsPublicKey(forId: String) throws -> Bool {
        return try publicKey(ofId: forId) != nil
    }

    public func containsPrivateKey(forId: DIDURL) -> Bool {
        guard containsPublicKey(forId: forId) else {
            return false
        }
        guard let store = getMeta().store else {
            return false
        }

        return store.containsPrivateKey(for: self.subject, id: forId)
    }

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

            let address = HDKey.DerivedKey.getAddress(key.publicKeyBytes)
            if  address == subject.methodSpecificId {
                return key.getId()
            }
        }
        return nil
    }

    public var defaultPublicKey: DIDURL {
        return getDefaultPublicKey()!
    }

    func keyPair_PublicKey(ofId: DIDURL) throws -> Data {
        guard containsPublicKey(forId: ofId) else {
            throw DIDError.illegalArgument("Key no exist")
        }
        guard getMeta().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store.")
        }
        guard getMeta().store!.containsPrivateKey(for: subject, id: ofId) else {
            throw DIDError.illegalArgument("Don't have private key")
        }
        let pubKey = publicKey(ofId: ofId)
        let pubs = pubKey!.publicKeyBytes
        let pubData = Data(bytes: pubs, count: pubs.count)
        let publicKeyData = HDKey.DerivedKey.PEM_ReadPublicKey(pubData)

        return publicKeyData.data(using: .utf8)!
    }

    func keyPair_PrivateKey(ofId: DIDURL, using storePassword: String) throws -> Data {
        guard containsPublicKey(forId: ofId) else {
            throw DIDError.illegalArgument("Key no exist")
        }
        guard getMeta().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store.")
        }
        guard getMeta().store!.containsPrivateKey(for: subject, id: ofId) else {
            throw DIDError.illegalArgument("Don't have private key")
        }

        let pubKey = publicKey(ofId: ofId)
        let pubs = pubKey!.publicKeyBytes
        let pubData = Data(bytes: pubs, count: pubs.count)

        let privKey = try getMeta().store?.loadPrivateKey(for: subject, byId: ofId)
        let privKeyData = try DIDStore.decryptFromBase64(privKey!, storePassword)
        let privateKeyData = try HDKey.DerivedKey.PEM_ReadPrivateKey(pubData, privKeyData)

        return privateKeyData.data(using: .utf8)!
    }

    public func jwtBuilder() throws -> JwtBuilder {

        let build = JwtBuilder(issuer: subject.toString(), publicKey: { (id) -> Data in

            var _id: DIDURL
            if id == nil {
                _id = self.getDefaultPublicKey()!
            } else {
                _id = try DIDURL(self.subject, id!)
            }
            return try self.keyPair_PublicKey(ofId: _id)

        }) { (id, storepass) -> Data in
            var _id: DIDURL

            if id == nil {
                _id = self.getDefaultPublicKey()!
            } else {
                _id = try DIDURL(self.subject, id!)
            }
            return try self.keyPair_PrivateKey(ofId: _id, using: storepass)
        }
        return build.setIssuer(iss: subject.description)
    }

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
        builder.getPrivateKey = {(id, storepass) in

            var _id: DIDURL
            if id == nil {
                _id = self.getDefaultPublicKey()!
            } else {
                _id = try DIDURL(self.subject, id!)
            }
            return try self.keyPair_PrivateKey(ofId: _id, using: storepass!)
        }
        return builder
    }

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

        // Can not remove default public key
        guard self.getDefaultPublicKey() != id else {
            return false
        }

        if !force && (key!.isAuthenticationKey || key!.isAthorizationKey) {
            return  false
        }

        _ = publicKeyMap.remove(id)
        _ = getMeta().store?.deletePrivateKey(for: subject, id: id)
        return true
    }

    public var authenticationKeyCount: Int {
        return publicKeyMap.count() { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    public func authenticationKeys() -> Array<PublicKey> {
        return publicKeyMap.values() { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    public func selectAuthenticationKeys(byId: DIDURL, andType: String?) -> Array<PublicKey> {
        return publicKeyMap.select(byId, andType) { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    public func selectAuthenticationKeys(byId: String, andType: String?) throws -> Array<PublicKey> {
        let id = try DIDURL(subject, byId)
        return selectAuthenticationKeys(byId: id, andType: andType)
    }

    public func selectAuthenticationKeys(byType: String) -> Array<PublicKey> {
        return publicKeyMap.select(nil, byType) { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    public func authenticationKey(ofId: DIDURL) -> PublicKey? {
        return publicKeyMap.get(forKey: ofId) { value -> Bool in
            return (value as PublicKey).isAuthenticationKey
        }
    }

    public func authenticationKey(ofId: String) throws -> PublicKey?  {
        return authenticationKey(ofId: try DIDURL(subject, ofId))
    }

    public func containsAuthenticationKey(forId: String) throws -> Bool {
        return try authenticationKey(ofId: forId) != nil
    }

    public func containsAuthenticationKey(forId: DIDURL) -> Bool {
        return authenticationKey(ofId: forId) != nil
    }

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

    public var authorizationKeyCount: Int {
        return publicKeyMap.count() { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    public func authorizationKeys() -> Array<PublicKey> {
        return publicKeyMap.values() { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    public func selectAuthorizationKeys(byId: DIDURL, andType: String?) -> Array<PublicKey> {
        return publicKeyMap.select(byId, andType) { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    public func selectAuthorizationKeys(byId: String, andType: String?) throws -> Array<PublicKey> {
        let id = try DIDURL(subject, byId)
        return selectAuthorizationKeys(byId: id, andType: andType)
    }

    public func selectAuthorizationKeys(byType: String) -> Array<PublicKey> {
        return publicKeyMap.select(nil, byType) { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    public func authorizationKey(ofId: DIDURL) -> PublicKey? {
        return publicKeyMap.get(forKey: ofId) { value -> Bool in
            return (value as PublicKey).isAthorizationKey
        }
    }

    public func authorizationKey(ofId: String) throws -> PublicKey?  {
        return authorizationKey(ofId: try DIDURL(subject, ofId))
    }

    public func containsAuthorizationKey(forId: String) throws -> Bool {
        return try authorizationKey(ofId: forId) != nil
    }

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

    public var credentialCount: Int {
        return credentialMap.count() { value -> Bool in return true }
    }

    public func credentials() -> Array<VerifiableCredential> {
        return credentialMap.values() { value -> Bool in return true }
    }

    public func selectCredentials(byId: DIDURL, andType: String?) -> Array<VerifiableCredential>  {
        return credentialMap.select(byId, andType) { value -> Bool in return true }
    }

    public func selectCredentials(byId: String, andType: String?) throws -> Array<VerifiableCredential>  {
        let id = try DIDURL(subject, byId)
        return selectCredentials(byId: id, andType: andType)
    }

    public func selectCredentials(byType: String) -> Array<VerifiableCredential>  {
        return credentialMap.select(nil, byType) { value -> Bool in return true }
    }

    public func credential(ofId: DIDURL) -> VerifiableCredential? {
        return credentialMap.get(forKey: ofId) { value -> Bool in return true }
    }

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

    public var serviceCount: Int {
        return serviceMap.count() { value -> Bool in return true }
    }

    public func services() -> Array<Service> {
        return serviceMap.values() { value -> Bool in return true }
    }

    public func selectServices(byId: DIDURL, andType: String?) -> Array<Service>  {
        return serviceMap.select(byId, andType) { value -> Bool in return true }
    }

    public func selectServices(byId: String, andType: String?) throws -> Array<Service>  {
        let id = try DIDURL(subject, byId)
        return selectServices(byId: id, andType: andType)
    }

    public func selectServices(byType: String) -> Array<Service>  {
        return serviceMap.select(nil, byType) { value -> Bool in return true }
    }

    public func service(ofId: DIDURL) -> Service? {
        return serviceMap.get(forKey: ofId) { value -> Bool in return true }
    }

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

    public var isValid: Bool {
        return !isDeactivated && !isExpired && isGenuine
    }

    public func editing() -> DIDDocumentBuilder {
        return DIDDocumentBuilder(self)
    }

    public func sign(using storePassword: String, for data: Data...) throws -> String {
        return try sign(self.defaultPublicKey, storePassword, data)
    }

    public func sign(withId: DIDURL, using storePassword: String, for data: Data...) throws -> String {
        return try sign(withId, storePassword, data)
    }

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
        guard getMeta().attachedStore else {
            throw DIDError.didStoreError("Not attached with DID store")
        }

        return try getMeta().store!.sign(subject, id, storePassword, data)
    }

    public func verify(signature: String, onto data: Data...) throws -> Bool {
        return try verify(self.defaultPublicKey, signature, data)
    }

    public func verify(withId: DIDURL, using signature: String, onto data: Data...) throws -> Bool {
        return try verify(withId, signature, data)
    }

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
        let pks = pubKey!.publicKeyBytes
        var pkData = Data(bytes: pks, count: pks.count)
        let cpk = pkData.withUnsafeMutableBytes { (pk: UnsafeMutablePointer<UInt8>) -> UnsafeMutablePointer<UInt8> in
            return pk
        }
        let csignature = sigature.toUnsafeMutablePointerInt8()
        let c_inputs = getVaList(cinputs)
        let count = cinputs.count / 2

        // digest
        let cdigest = UnsafeMutablePointer<UInt8>.allocate(capacity: capacity)
        let size = sha256v_digest(cdigest, Int32(count), c_inputs)
        let re = ecdsa_verify_base64(csignature, cpk, cdigest, size)

        return re == 0 ? true : false
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
        try doc.fromJson(fromJson)

        return doc
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

    public class func convertToDIDDocument(fromJson: String) throws -> DIDDocument {
        return try  convertToDIDDocument(fromData: fromJson.data(using: .utf8)!)
    }

    public class func convertToDIDDocument(fromFileAtPath : String) throws -> DIDDocument {
        return try convertToDIDDocument(fromJson: String(contentsOfFile: fromFileAtPath, encoding: .utf8))
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

    public func convertFromDIDDocument(_ normalized: Bool) throws -> String {
        return try toJson(normalized, false)
    }

    public func convertFromDIDDocument() throws -> String {
        return try toJson(false, false)
    }

    public func convertFromDIDDocument(_ normalized: Bool) throws -> Data {
        return try toJson(normalized, false)
    }

    public func convertFromDIDDocument() throws -> Data {
        return try toJson(false, false)
    }

    public func convertFromDIDDocument(_ normalized: Bool, asFileAtPath: String) throws {
        return try convertFromDIDDocument(normalized, asURL: URL.init(fileURLWithPath: asFileAtPath))
    }

    public func convertFromDIDDocument(asFileAtPath: String) throws {
        return try convertFromDIDDocument(false, asFileAtPath: asFileAtPath)
    }

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

    public func convertFromDIDDocument(asURL: URL) throws {
        return try convertFromDIDDocument(false, asURL: asURL)
    }
}

extension DIDDocument: CustomStringConvertible {
    func toString() -> String {
        return (try? toJson(false, false)) ?? ""
    }
    
    func toString(_ force: Bool) -> String {
        return (try? toJson(force, false)) ?? ""
    }

    func toString(_ force: Bool, forSign: Bool) -> String {
        return (try? toJson(force, forSign)) ?? ""
    }

    public var description: String {
        return toString()
    }
}
