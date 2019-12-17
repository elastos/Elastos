
import XCTest
import ElastosDIDSDK

class DIDDoucumentTests: XCTestCase {
    
    var store: DIDStore!
    
    var compactPath: String!
    var documentPath: String!
    var normalizedPath: String!
    
    override func setUp() {
        super.setUp()
        
        let bundle = Bundle(for: type(of: self))
        compactPath = bundle.path(forResource: "compact", ofType: "json")!
        documentPath = bundle.path(forResource: "testdiddoc", ofType: "json")!
        normalizedPath = bundle.path(forResource: "normalized", ofType: "json")!
    }
    
    override func tearDown() {
        compactPath = nil
        documentPath = nil
        normalizedPath = nil
        super.tearDown()
    }
    
    func loadTestDocument() throws -> DIDDocument {
        let bundle = Bundle(for: type(of: self))
        let path = bundle.path(forResource: "testdiddoc", ofType: "json")!
        let doc = try DIDDocument.fromJson(path)
        return doc
    }
    
    func updateForTesting(_ doc: DIDDocument) throws -> DIDDocument {
        var id: DIDURL
        var success: Bool
        
        for i in 0...5 {
            id = try DIDURL(doc.subject!, "test-pk-\(i)")
            let data = Data()
            let udata = [UInt8](data)
            let keyBase58: String = Base58.base58FromBytes(udata)
            success = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertTrue(success)
        }
        
        for i in 0...5 {
            id = try DIDURL(doc.subject!, "test-auth-\(i)")
            let data = Data()
            let udata = [UInt8](data)
            let keyBase58: String = Base58.base58FromBytes(udata)
            success = try doc.addAuthenticationKey(id, keyBase58)
            XCTAssertTrue(success)
        }
        
        let controller: DID = try DID("did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh")
        for i in 0...5 {
            id = try DIDURL(doc.subject!, "test-autho-\(i)")
            let data = Data()
            let udata = [UInt8](data)
            let keyBase58: String = Base58.base58FromBytes(udata)
            success = try doc.addAuthorizationKey(id, controller, keyBase58)
            XCTAssertTrue(success)
        }
        return doc
    }
    
    
    func testGetPublicKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertTrue(try doc.isValid())
            
            // Count and list.
            XCTAssertEqual(4, doc.getPublicKeyCount())
            
            var pks:Array<DIDPublicKey> = doc.getPublicKeys()
            XCTAssertEqual(4, pks.count)
            
            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)
                
                if (pk.id.fragment == "recovery") {
                    XCTAssertNotEqual(doc.subject, pk.controller)
                }
                else {
                    XCTAssertEqual(doc.subject, pk.controller)
                }
                
                let re = pk.id.fragment == "primary" || pk.id.fragment == "key2" || pk.id.fragment == "key3" || pk.id.fragment == "recovery"
                XCTAssertTrue(re)
            }
            var pk = try doc.getPublicKey("primary")
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), pk!.id)
            
            var id: DIDURL = try DIDURL(doc.subject!, "key2")
            pk = try doc.getPublicKey(id)
            XCTAssertEqual(id, pk!.id)
            
            id = doc.getDefaultPublicKey()
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), id)
            
            // Key not exist, should fail.
            pk = try doc.getPublicKey("notExist")
            XCTAssertNil(pk)
            id = try DIDURL(doc.subject!, "notExist")
            pk = try doc.getPublicKey(id)
            XCTAssertNil(pk)
            
            // Selector
            id = doc.getDefaultPublicKey()
            pks = try doc.selectPublicKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count);
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), pks[0].id)
            
            pks = try doc.selectPublicKeys(id, nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), pks[0].id)
            
            pks = try doc.selectPublicKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEqual(4, pks.count)
            
            pks = try doc.selectPublicKeys("key2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
            
            pks = try doc.selectPublicKeys("key3", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key3"), pks[0].id)
        } catch {
            print(error)
        }
    }
    
    func testAddPublicKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Add 2 public keys
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey = try TestData.generateKeypair()
            var success: Bool = try doc.addPublicKey(id, doc.subject!, key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try doc.addPublicKey("test2", doc.subject!.description,
                                           try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk: DIDPublicKey = try doc.getPublicKey("test1")!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk.id)
            
            pk = try doc.getPublicKey("test2")!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test2"), pk.id)
            
            // Check the final count.
            XCTAssertEqual(6, doc.getPublicKeyCount())
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
            
        } catch  {
            print(error)
        }
    }
    
    func testRemovePublicKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // recovery used by authorization, should failed.
            let id: DIDURL = try DIDURL(doc.subject!, "recovery")
            var success: Bool = try doc.removePublicKey(id)
            XCTAssertFalse(success)
            
            // force remove public key, should success
            success = try doc.removePublicKey(id, true)
            XCTAssertTrue(success)
            
            success = try doc.removePublicKey("key2", true)
            XCTAssertTrue(success)
            // Key not exist, should fail.
            success = try doc.removePublicKey("notExistKey", true)
            XCTAssertFalse(success)
            
            // Can not remove default publickey, should fail.
            success = try doc.removePublicKey(doc.getDefaultPublicKey(), true)
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk: DIDPublicKey = try doc.getPublicKey("recovery")!
            XCTAssertNil(pk)
            
            pk = try doc.getPublicKey("key2")!
            XCTAssertNil(pk)
            
            // Check the final count.
            XCTAssertEqual(2, doc.getPublicKeyCount())
            XCTAssertEqual(2, doc.getAuthenticationKeyCount())
            XCTAssertEqual(0, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
        
    }
    
    func testGetAuthenticationKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Count and list.
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            
            var pks: Array<DIDPublicKey> = doc.getAuthenticationKeys()
            XCTAssertEqual(3, pks.count)
            
            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)
                XCTAssertEqual(doc.subject, pk.controller)
                let re = pk.id.fragment == "primary" || pk.id.fragment == "key2" || pk.id.fragment == "key3"
                XCTAssertTrue(re)
            }
            
            // AuthenticationKey getter
            var pk: DIDPublicKey = try doc.getAuthenticationKey("primary")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), pk.id)
            
            var id: DIDURL = try DIDURL(doc.subject!, "key3")
            pk = try doc.getAuthenticationKey(id)
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk.id)
            
            // Key not exist, should fail.
            pk = try doc.getAuthenticationKey("notExist")
            XCTAssertNil(pk)
            id = try DIDURL(doc.subject!, "notExist")
            pk = try doc.getAuthenticationKey(id)
            XCTAssertNil(pk)
            
            // selector
            id = try DIDURL(doc.subject!, "key3")
            pks = try doc.selectAuthenticationKeys(id, type: Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            pks = try doc.selectAuthenticationKeys(id, type: nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            
            pks = try doc.selectAuthenticationKeys(nil, type: Constants.defaultPublicKeyType)
            XCTAssertEqual(3, pks.count)
            
            pks = try doc.selectAuthenticationKeys("key2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
            
            pks = try doc.selectAuthenticationKeys("key2", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
        } catch {
            print(error)
        }
        
    }
    func testAddAuthenticationKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            // Add 2 public keys for test.
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey  = try TestData.generateKeypair()
            var success: Bool = try doc.addPublicKey(id, doc.subject!,
                                                     try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            key = try TestData.generateKeypair()
            success = try doc.addPublicKey("test2", doc.subject!.description,
                                           try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Add by reference
            success = try doc.addAuthenticationKey(DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try doc.addAuthenticationKey("test2")
            XCTAssertTrue(success)
            
            // Add new keys
            key = try TestData.generateKeypair()
            success = try doc.addAuthenticationKey(DIDURL(doc.subject!, "test3"),
                                                   try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try doc.addAuthenticationKey("test4", key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Try to add a non existing key, should fail.
            success = try doc.addAuthenticationKey("notExistKey")
            XCTAssertFalse(success)
            
            // Try to add a key not owned by self, should fail.
            success = try doc.addAuthenticationKey("recovery")
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk: DIDPublicKey = try doc.getAuthenticationKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk.id)
            
            pk = try doc.getAuthenticationKey("test2")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test2"), pk.id)
            
            pk = try doc.getAuthenticationKey("test3")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test3"), pk.id)
            
            pk = try doc.getAuthenticationKey("test4")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test4"), pk.id)
            
            // Check the final count.
            XCTAssertEqual(8, doc.getPublicKeyCount())
            XCTAssertEqual(7, doc.getAuthenticationKeyCount())
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testRemoveAuthenticationKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Add 2 public keys for test
            var key: DerivedKey  = try TestData.generateKeypair()
            var success: Bool = try doc.addAuthenticationKey(
                try DIDURL(doc.subject!, "test1"), key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try doc.addAuthenticationKey("test2", key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Remote keys
            success = doc.removeAuthenticationKey(try DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try doc.removeAuthenticationKey("test2")
            XCTAssertTrue(success)
            
            success = try doc.removeAuthenticationKey("key2")
            XCTAssertTrue(success)
            
            // Key not exist, should fail.
            success = try doc.removeAuthenticationKey("notExistKey")
            XCTAssertFalse(success)
            
            
            // Default publickey, can not remove, should fail.
            success = doc.removeAuthenticationKey(doc.getDefaultPublicKey())
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk: DIDPublicKey = try doc.getAuthenticationKey("test1")
            XCTAssertNil(pk)
            
            pk = try doc.getAuthenticationKey("test2")
            XCTAssertNil(pk)
            
            pk = try doc.getAuthenticationKey("key2")
            XCTAssertNil(pk)
            
            // Check the final count.
            XCTAssertEqual(6, doc.getPublicKeyCount())
            XCTAssertEqual(2, doc.getAuthenticationKeyCount())
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testGetAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Count and list.
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
            
            var pks: Array<DIDPublicKey> = doc.getAuthorizationKeys()
            XCTAssertEqual(1, pks.count)
            
            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)
                
                XCTAssertNotEqual(doc.subject, pk.controller)
                XCTAssertTrue(pk.id.fragment == "recovery")
            }
            
            // AuthorizationKey getter
            var pk: DIDPublicKey = try doc.getAuthorizationKey("recovery")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "recovery"), pk.id)
            
            var id: DIDURL = try DIDURL(doc.subject!, "recovery")
            pk = try doc.getAuthorizationKey(id)!
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk.id)
            
            // Key not exist, should fail.
            pk = try doc.getAuthorizationKey("notExistKey")
            XCTAssertNil(pk)
            id = try DIDURL(doc.subject!, "notExistKey")
            pk = try doc.getAuthorizationKey(id)!
            XCTAssertNil(pk)
            
            // Selector
            id = try DIDURL(doc.subject!, "recovery")
            pks = try doc.selectAuthorizationKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            
            pks = try doc.selectAuthorizationKeys(id, nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            pks = try doc.selectAuthorizationKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
        } catch {
            print(error)
        }
    }
    
    func testAddAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Add 2 public keys for test.
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey = try TestData.generateKeypair()
            var did = DID(DID.METHOD, DerivedKey.getIdString(try key.getPublicKeyBytes()))
            var success: Bool = try doc.addPublicKey(id, did, try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair();
            success = try doc.addPublicKey("test2", did.description,
                                           try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Add by reference
            success = try doc.addAuthorizationKey(DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try doc.addAuthorizationKey("test2")
            XCTAssertTrue(success)
            
            // Add new keys
            key = try TestData.generateKeypair()
            success = try doc.addAuthorizationKey(try DIDURL(doc.subject!, "test3"),
                                                  did, try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try doc.addAuthorizationKey("test4", did.description,
                                                  try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Try to add a non existing key, should fail.
            success = try doc.addAuthorizationKey("notExistKey")
            XCTAssertFalse(success)
            
            // Try to add key owned by self, should fail.
            success = try doc.addAuthorizationKey("key2")
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            var pk: DIDPublicKey = try doc.getAuthorizationKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk.id)
            pk = try doc.getAuthorizationKey("test2")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test2"), pk.id)
            pk = try doc.getAuthorizationKey("test3")
            
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test3"), pk.id)
            
            pk = try doc.getAuthorizationKey("test4")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test4"), pk.id)
            
            // Check the final key count.
            XCTAssertEqual(8, doc.getPublicKeyCount())
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            XCTAssertEqual(5, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testRemoveAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Add 2 keys for test.
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey  = try TestData.generateKeypair()
            let did = DID(DID.METHOD, DerivedKey.getIdString(try key.getPublicKeyBytes()))
            var success: Bool = try doc.addAuthorizationKey(id,
                                                            did, try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try doc.addAuthorizationKey("test2",
                                             did.description,
                                             try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Remove keys.
            success = doc.removeAuthorizationKey(try DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try doc.removeAuthorizationKey("recovery")
            XCTAssertTrue(success)
            
            // Key not exist, should fail.
            success = try doc.removeAuthorizationKey("notExistKey")
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk: DIDPublicKey = try doc.getAuthorizationKey("test1")
            XCTAssertNil(pk)
            
            pk = try doc.getAuthorizationKey("test2")
            XCTAssertNotNil(pk)
            
            pk = try doc.getAuthorizationKey("recovery")
            XCTAssertNil(pk)
            
            // Check the final count.
            XCTAssertEqual(6, doc.getPublicKeyCount())
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testGetCredential() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Count and list.
            XCTAssertEqual(2, doc.getCredentialCount())
            var vcs: Array<VerifiableCredential> = doc.getCredentials()
            XCTAssertEqual(2, vcs.count)
            
            for vc in vcs {
                XCTAssertEqual(doc.subject, vc.id.did)
                XCTAssertEqual(doc.subject, vc.subject.id)
                let re = vc.id.fragment == "profile" || vc.id.fragment == "email"
                XCTAssertTrue(re)
            }
            // Credential getter.
            var vc: VerifiableCredential = try doc.getCredential("profile")
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vc.id)
            
            vc = try doc.getCredential(DIDURL(doc.subject!, "email"))
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "email"), vc.id)
            
            // Credential not exist.
            vc = try doc.getCredential("notExistVc")
            XCTAssertNil(vc)
            
            // Credential selector.
            vcs = try doc.selectCredentials(DIDURL(doc.subject!, "profile"),
                                        "SelfProclaimedCredential")
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vcs[0].id)
            
            vcs = try doc.selectCredentials(DIDURL(doc.subject!, "profile"), nil)
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vcs[0].id)
            
            vcs = try doc.selectCredentials(nil, "SelfProclaimedCredential")
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vcs[0].id)
            
            vcs = try doc.selectCredentials(nil, "TestingCredential")
            XCTAssertEqual(0, vcs.count)
        } catch {
            print(error)
        }
    }
    
    func testAddCredential() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            // Add credentials.
            var vc: VerifiableCredential = try testData.loadPassportCredential()
            var success: Bool = doc.addCredential(vc)
            XCTAssertTrue(success)
            
            vc = try testData.loadTwitterCredential()
            success = doc.addCredential(vc)
            XCTAssertTrue(success)
            
            // Credential already exist, should fail.
            success = doc.addCredential(vc)
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check new added credential.
            vc = try doc.getCredential("passport")
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "passport"), vc.id)
            
            let id: DIDURL = try DIDURL(doc.subject!, "twitter")
            vc = try doc.getCredential(id)
            XCTAssertNotNil(vc)
            XCTAssertEqual(id, vc.id)
            
            // Should contains 3 credentials.
            XCTAssertEqual(4, doc.getCredentialCount())
        } catch {
            print(error)
        }
    }
    
    func testRemoveCredential() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Add test credentials.
            var vc: VerifiableCredential = try testData.loadPassportCredential()
            var success: Bool = doc.addCredential(vc)
            XCTAssertTrue(success)
            
            vc = try testData.loadTwitterCredential()
            success = doc.addCredential(vc)
            XCTAssertTrue(success)
            // Remove credentials
            success = try doc.removeCredential("profile")
            XCTAssertTrue(success)
            
            success = doc.removeCredential(try DIDURL(doc.subject!, "twitter"))
            XCTAssertTrue(success)
            
            // Credential not exist, should fail.
            success = try doc.removeCredential("notExistCredential")
            XCTAssertFalse(success)
            success = doc.removeCredential(try DIDURL(doc.subject!,
                                                  "notExistCredential"))
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            vc = try doc.getCredential("profile")
            XCTAssertNil(vc)
            vc = try doc.getCredential(try DIDURL(doc.subject!, "twitter"))
            XCTAssertNil(vc)
            
            // Check the final count.
            XCTAssertEqual(2, doc.getCredentialCount())
        } catch {
            print(error)
        }
    }
    
    func testGetService() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Count and list
            XCTAssertEqual(3, doc.getServiceCount())
            var svcs: Array<Service> = doc.getServices()
            XCTAssertEqual(3, svcs.count)
            
            for svc in svcs {
                XCTAssertEqual(doc.subject, svc.id.did)
                let re = svc.id.fragment == "openid" || svc.id.fragment == "vcr" || svc.id.fragment == "carrier"
                XCTAssertTrue(re)
            }
            
            // Service getter, should success.
            var svc: Service = try doc.getService("openid")!
            XCTAssertNotNil(svc)
            XCTAssertEqual(try DIDURL(doc.subject!, "openid"), svc.id)
            XCTAssertEqual("OpenIdConnectVersion1.0Service", svc.type)
            XCTAssertEqual("https://openid.example.com/", svc.endpoint)
            
            svc = try doc.getService(DIDURL(doc.subject!, "vcr"))!
            XCTAssertNotNil(svc)
            XCTAssertEqual(try DIDURL(doc.subject!, "vcr"), svc.id)
            
            // Service not exist, should fail.
            svc = try doc.getService("notExistService")!
            XCTAssertNil(svc)
            
            // Service selector.
            svcs = try doc.selectServices("vcr", "CredentialRepositoryService")
            XCTAssertEqual(1, svcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "vcr"), svcs[0].id)
            
            svcs = try doc.selectServices(DIDURL(doc.subject!, "openid"), nil)
            XCTAssertEqual(1, svcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "openid"), svcs[0].id)
            
            svcs = try doc.selectServices(nil, "CarrierAddress")
            XCTAssertEqual(1, svcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "carrier"), svcs[0].id)
            
            // Service not exist, should return a empty list.
            svcs = try doc.selectServices("notExistService", "CredentialRepositoryService")
            XCTAssertEqual(0, svcs.count)
            
            svcs = try doc.selectServices(nil, "notExistType")
            XCTAssertEqual(0, svcs.count)
        } catch {
            print(error)
        }
    }
    
    func testAddService() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Add services
            var success: Bool = try doc.addService("test-svc-1",
                                              "Service.Testing", "https://www.elastos.org/testing1")
            XCTAssertTrue(success)
            
            success = try doc.addService(DIDURL(doc.subject!, "test-svc-2"),
                                     "Service.Testing", "https://www.elastos.org/testing2")
            XCTAssertTrue(success)
            
            // Service id already exist, should failed.
            success = try doc.addService("vcr", "test", "https://www.elastos.org/test")
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check the final count
            XCTAssertEqual(5, doc.getServiceCount())
            
            // Try to select new added 2 services
            let svcs: Array<Service> = try doc.selectServices(nil, "Service.Testing")
            XCTAssertEqual(2, svcs.count)
            XCTAssertEqual("Service.Testing", svcs[0].type)
            XCTAssertEqual("Service.Testing", svcs[0].type)
        } catch {
            print(error)
        }
    }
    
    func testRemoveService() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // remove services
            var success: Bool = try doc.removeService("openid")
            XCTAssertTrue(success)
            
            success = doc.removeService(try DIDURL(doc.subject!, "vcr"))
            XCTAssertTrue(success)
            
            // Service not exist, should fail.
            success = try doc.removeService("notExistService")
            XCTAssertFalse(success)
            
            doc = try doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            var svc: Service = try doc.getService("openid")!
            XCTAssertNil(svc)
            
            svc = try doc.getService(DIDURL(doc.subject!, "vcr"))!
            XCTAssertNil(svc)
            
            // Check the final count
            XCTAssertEqual(1, doc.getServiceCount())
        } catch {
            print(error)
        }
    }
    
    func testParseAndSerializeDocument() {
        do {
            let testData: TestData = try TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            let compact: DIDDocument = try DIDDocument.fromJson(try testData.loadTestCompactJson())
            XCTAssertNotNil(compact)
            XCTAssertTrue(try compact.isValid())
            
            XCTAssertEqual(4, compact.getPublicKeyCount())
            
            XCTAssertEqual(3, compact.getAuthenticationKeyCount())
            XCTAssertEqual(1, compact.getAuthorizationKeyCount())
            XCTAssertEqual(2, compact.getCredentialCount())
            XCTAssertEqual(3, compact.getServiceCount())
            
            let normalized: DIDDocument = try DIDDocument.fromJson(try testData.loadTestCompactJson())
            XCTAssertNotNil(normalized);
            XCTAssertTrue(try normalized.isValid())
            
            XCTAssertEqual(4, normalized.getPublicKeyCount())
            
            XCTAssertEqual(3, normalized.getAuthenticationKeyCount())
            XCTAssertEqual(1, normalized.getAuthorizationKeyCount())
            XCTAssertEqual(2, normalized.getCredentialCount())
            XCTAssertEqual(3, normalized.getServiceCount())
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            XCTAssertEqual(try testData.loadTestNormalizedJson(), try compact.description(true))
            XCTAssertEqual(try testData.loadTestNormalizedJson(), try normalized.description(true))
            XCTAssertEqual(try testData.loadTestNormalizedJson(), try doc.description(true))
            
            XCTAssertEqual(try testData.loadTestCompactJson(), try compact.description(false))
            XCTAssertEqual(try testData.loadTestCompactJson(), try normalized.description(false))
            XCTAssertEqual(try testData.loadTestCompactJson(), try doc.description(false))
        } catch {
        }
    }
    
    func test31SignAndVerify() {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(true)
            try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            var json = try doc.description(false)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            var data = Data()
            let pkid: DIDURL = try DIDURL(doc.subject!, "primary")
            
            for i in 0..<10 {
                var inputs: [CVarArg] = [json, json.count]
                var count: Int = inputs.count / 2
                var sig: String = try doc.sign(pkid, storePass, count, inputs)
                
                var result: Bool = try doc.verify(pkid, sig, count, inputs)
                XCTAssertTrue(result)
                
                json = String(json.suffix(json.count - 1))
                inputs = [json, json.count]
                count = inputs.count / 2
                result = try doc.verify(pkid, sig, count, inputs)
                XCTAssertFalse(result)
                
                sig = try doc.sign(storePass, count, inputs)
                result = try doc.verify(sig, count, inputs)
                XCTAssertTrue(result)
                
                json = String(json.suffix(json.count - 1))
                inputs = [json, json.count]
                count = inputs.count / 2
                result = try doc.verify(sig, count, inputs)
                XCTAssertFalse(result)
            }
        } catch {
            print(error)
        }
    }

    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
