
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
            testData.setupStore(true)
            
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertTrue(doc.isValid())
            
            // Count and list.
            XCTAssertEqual(4, doc.getPublicKeyCount())
            
            let pks:Array<DIDPublicKey> = doc.getPublicKeys()
            XCTAssertEqual(4, pks.count)
            
            for item in pks {
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
            let pk = doc.getPublicKey("primary")
            XCTAssertEqual(DIDURL(doc.subject, "primary"), pk.id)
            
            var id: DIDURL = DIDURL(doc.subject, "key2")
            pk = doc.getPublicKey(id)
            XCTAssertEqual(id, pk.id)
            
            id = doc.getDefaultPublicKey()
            XCTAssertEqual(DIDURL(doc.subject, "primary"), id)
            
            // Key not exist, should fail.
            pk = doc.getPublicKey("notExist")
            XCTAssertNil(pk)
            id = DIDURL(doc.subject, "notExist")
            pk = doc.getPublicKey(id)
            XCTAssertNil(pk)
            
            // Selector
            id = doc.getDefaultPublicKey()
            pks = doc.selectPublicKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count);
            XCTAssertEqual(DIDURL(doc.subject, "primary"), pks[0].id)
            
            pks = doc.selectPublicKeys(id, nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(DIDURL(doc.subject, "primary"), pks[0].id)
            
            pks = doc.selectPublicKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEqual(4, pks.count)
            
            pks = doc.selectPublicKeys("key2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(DIDURL(doc.subject, "key2"), pks[0].id)
            
            pks = doc.selectPublicKeys("key3", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(DIDURL(doc.subject, "key3"), pks[0].id)
        } catch {
            print(error)
        }
    }
    
    func testAddPublicKey() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            testData.initIdentity()
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Add 2 public keys
            let id: DIDURL = DIDURL(db.subject, "test1")
            let key: DerivedKey = TestData.generateKeypair()
            let success: Bool = doc.addPublicKey(id, doc.subject, key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = TestData.generateKeypair()
            success = doc.addPublicKey("test2", doc.subject.descriton,
                    key.getPublicKeyBase58())
            XCTAssertTrue(success)

            doc = doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Check existence
            let pk: PublicKey = doc.getPublicKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "test1"), pk.id)

            pk = doc.getPublicKey("test2")
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "test2"), pk.id)

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
            testData.setupStore(true)
            testData.initIdentity()
            
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())

            // recovery used by authorization, should failed.
            let id: DIDURL = DIDURL(doc.subject, "recovery")
            let success: Bool = doc.removePublicKey(id)
            XCTAssertFalse(success)
            
            // force remove public key, should success
            success = doc.removePublicKey(id, true)
            XCTAssertTrue(success)
            
            success = doc.removePublicKey("key2", true)
            XCTAssertTrue(success)
            // Key not exist, should fail.
            success = db.removePublicKey("notExistKey", true)
            XCTAssertFalse(success)
            
            // Can not remove default publickey, should fail.
            success = doc.removePublicKey(doc.getDefaultPublicKey(), true)
            XCTAssertFalse(success)

            doc = doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Check existence
            let pk: PublicKey = doc.getPublicKey("recovery")
            XCTAssertNil(pk)

            pk = doc.getPublicKey("key2")
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
            testData.setupStore(true)
            testData.initIdentity()
            
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Count and list.
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())

            let pks: Array<DIDPublicKey> = doc.getAuthenticationKeys()
            XCTAssertEqual(3, pks.count)

            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)
                XCTAssertEqual(doc.subject, pk.controller)
                let re = pk.id.fragment == "primary" || pk.id.fragment == "key2" || pk.id.fragment == "key3"
                XCTAssertTrue(re)
            }

            // AuthenticationKey getter
            let pk: DIDPublicKey = doc.getAuthenticationKey("primary")
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "primary"), pk.id)

            let id: DIDURL = DIDURL(doc.getSubject(), "key3")
            pk = doc.getAuthenticationKey(id)
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk.id)

            // Key not exist, should fail.
            pk = doc.getAuthenticationKey("notExist")
            XCTAssertNil(pk)
            id = DIDURL(doc.subject, "notExist")
            pk = doc.getAuthenticationKey(id)
            XCTAssertNil(pk)
            
            // selector
            id = DIDURL(doc.subject, "key3")
            pks = doc.selectAuthenticationKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            pks = doc.selectAuthenticationKeys(id, nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)

            pks = doc.selectAuthenticationKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEqual(3, pks.count)
            
            pks = doc.selectAuthenticationKeys("key2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(DIDURL(doc.subject, "key2"), pks[0].id)

            pks = doc.selectAuthenticationKeys("key2", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(DIDURL(doc.subject, "key2"), pks[0].id)
        } catch {
            print(error)
        }
        
    }
    func testAddAuthenticationKey() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            testData.initIdentity()
            
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
           XCTAssertTrue(doc.isValid())
            // Add 2 public keys for test.
            let id: DIDURL = DIDURL(db.subject, "test1")
            let key: DerivedKey  = TestData.generateKeypair()
            let success: Bool = doc.addPublicKey(id, db.subject,
                    key.getPublicKeyBase58())
            XCTAssertTrue(success)
            key = TestData.generateKeypair()
            success = doc.addPublicKey("test2", doc.subject.describtion,
                    key.getPublicKeyBase58())
            XCTAssertTrue(success)

            // Add by reference
            success = doc.addAuthenticationKey(DIDURL(doc.subject, "test1"))
            XCTAssertTrue(success)

            success = doc.addAuthenticationKey("test2")
            XCTAssertTrue(success)
            
            // Add new keys
            key = TestData.generateKeypair()
            success = doc.addAuthenticationKey(DIDURL(doc.subject, "test3"),
                    key.getPublicKeyBase58())
            XCTAssertTrue(success)

            key = TestData.generateKeypair()
            success = db.addAuthenticationKey("test4", key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Try to add a non existing key, should fail.
            success = doc.addAuthenticationKey("notExistKey")
            XCTAssertFalse(success)

            // Try to add a key not owned by self, should fail.
            success = db.addAuthenticationKey("recovery")
            XCTAssertFalse(success)

            doc = doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Check existence
            let pk: DIPublicKey = doc.getAuthenticationKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "test1"), pk.id)

            pk = doc.getAuthenticationKey("test2")
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "test2"), pk.id)

            pk = doc.getAuthenticationKey("test3")
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "test3"), pk.id)
            
            pk = doc.getAuthenticationKey("test4")
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "test4"), pk.id)

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
            testData.setupStore(true)
            testData.initIdentity()
            

            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Add 2 public keys for test
            let key: DerivedKey  = TestData.generateKeypair()
            let success: Bool = doc.addAuthenticationKey(
                    DIDURL(doc.subject, "test1"), key.getPublicKeyBase58())
            XCTAssertTrue(success)

            key = TestData.generateKeypair()
            success = db.addAuthenticationKey("test2", key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Remote keys
            success = doc.removeAuthenticationKey(DIDURL(doc.subject, "test1"))
            XCTAssertTrue(success)

            success = doc.removeAuthenticationKey("test2")
            XCTAssertTrue(success)

            success = doc.removeAuthenticationKey("key2")
            XCTAssertTrue(success)

            // Key not exist, should fail.
            success = doc.removeAuthenticationKey("notExistKey")
            XCTAssertFalse(success)
            

            // Default publickey, can not remove, should fail.
            success = doc.removeAuthenticationKey(doc.getDefaultPublicKey())
            XCTAssertFalse(success)

            doc = doc.seal(TestConfig.storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())

            // Check existence
            let pk: DIDPublicKey = doc.getAuthenticationKey("test1")
            XCTAssertNil(pk)

            pk = doc.getAuthenticationKey("test2")
            XCTAssertNil(pk)

            pk = doc.getAuthenticationKey("key2")
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
            testData.setupStore(true)
            testData.initIdentity()
            
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Count and list.
            XCTAssertEquals(1, doc.getAuthorizationKeyCount())

            let pks: Array<DIDPublicKey> = doc.getAuthorizationKeys()
            XCTAssertEquals(1, pks.count)
            
            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEquals(Constants.defaultPublicKeyType, pk.type)

                XCTAssertNotEqual(doc.subject, pk.controller)
                XCTAssertTrue(pk.id.fragment == "recovery")
            }
            
            // AuthorizationKey getter
            let pk: DIDPublicKey = doc.getAuthorizationKey("recovery")
            XCTAssertNotNil(pk)
            assertEquals(DIDURL(doc.subject, "recovery"), pk.id)

            let id: DIDURL = DIDURL(doc.subject, "recovery")
            pk = doc.getAuthorizationKey(id)
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk.id)

            // Key not exist, should fail.
            pk = doc.getAuthorizationKey("notExistKey")
            XCTAssertNil(pk)
            id = DIDURL(doc.subject, "notExistKey")
            pk = doc.getAuthorizationKey(id)
            XCTAssertNil(pk)

            // Selector
            id = DIDURL(doc.subject, "recovery")
            pks = doc.selectAuthorizationKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)

            pks = doc.selectAuthorizationKeys(id, nil)
            XCTAssertEquals(1, pks.count)
            XCTAssertEquals(id, pks[0].id)
            pks = doc.selectAuthorizationKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEquals(1, pks.count)
        } catch {
            print(error)
        }
    }
    
    func testAddAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            testData.initIdentity()
            
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())

            // Add 2 public keys for test.
            let id: DIDURL = DIDURL(db.subject, "test1")
            let key: DerivedKey = TestData.generateKeypair()
            let success: Bool = doc.addPublicKey(id,
                    DID(DID.METHOD, key.getAddress()), key.getPublicKeyBase58())
            assertTrue(success);

            key = TestData.generateKeypair();
            success = doc.addPublicKey("test2", DID(DID.METHOD, key.getAddress()).description,
                    key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Add by reference
            success = doc.addAuthorizationKey(DIDURL(doc.subject, "test1"))
            XCTAssertTrue(success)

            success = doc.addAuthorizationKey("test2")
            XCTAssertTrue(success)

            // Add new keys
            key = TestData.generateKeypair()
            success = doc.addAuthorizationKey(DIDURL(doc.subject, "test3"),
                    DID(DID.METHOD, key.getAddress()), key.getPublicKeyBase58())
            XCTAssertTrue(success)

            key = TestData.generateKeypair()
            success = doc.addAuthorizationKey("test4", DID(DID.METHOD, key.getAddress()).toString(),
                    key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Try to add a non existing key, should fail.
            success = doc.addAuthorizationKey("notExistKey")
            XCTAssertFalse(success)

            // Try to add key owned by self, should fail.
            success = dOC.addAuthorizationKey("key2")
            XCTAssertFalse(success)

            doc = doc.seal(storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())

            let pk: DIDPublicKey = doc.getAuthorizationKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEquals(DIDURL(doc.subject, "test1"), pk.ID)
            pk = doc.getAuthorizationKey("test2")
            XCTAssertNotNil(pk)
            assertEquals(DIDURL(doc.subject, "test2"), pk.id)
            pk = doc.getAuthorizationKey("test3")
            
            XCTAssertNotNil(pk)
            XCTAssertEqual(DIDURL(doc.subject, "test3"), pk.ID)

            pk = doc.getAuthorizationKey("test4")
            XCTAssertNotNil(pk)
            XCTAssertEquals(DIDURL(doc.subject, "test4"), pk.id)

            // Check the final key count.
            XCTAssertEquals(8, doc.getPublicKeyCount())
            XCTAssertEquals(3, doc.getAuthenticationKeyCount())
            XCTAssertEquals(5, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testRemoveAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            testData.setupStore(true)
            testData.initIdentity()
           
            let doc: DIDDocument = testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())

            // Add 2 keys for test.
            let id: DIDURL = DIDURL(db.subject, "test1")
            let key: DerivedKey  = TestData.generateKeypair()
            let success: Bool = doc.addAuthorizationKey(id,
                    DID(DID.METHOD, key.getAddress()), key.getPublicKeyBase58())
            XCTAssertTrue(success)

            key = TestData.generateKeypair()
            success = doc.addAuthorizationKey("test2",
                    DID(DID.METHOD, key.getAddress()).toString(),
                    key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Remove keys.
            success = doc.removeAuthorizationKey(new DIDURL(doc.subject, "test1"))
            XCTAssertTrue(success)

            success = doc.removeAuthorizationKey("recovery")
            XCTAssertTrue(success)

            // Key not exist, should fail.
            success = doc.removeAuthorizationKey("notExistKey")
            XCTAssertFalse(success)

            doc = doc.seal(TestConfig.storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(doc.isValid())
            
            // Check existence
            let pk: DIDPublicKey = doc.getAuthorizationKey("test1")
            XCTAssertNil(pk)

            pk = doc.getAuthorizationKey("test2")
            XTAssertNotNIl(pk)

            pk = doc.getAuthorizationKey("recovery")
            XCTAssertNIl(pk)

            // Check the final count.
            XCTAssertEqual(6, doc.getPublicKeyCount())
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            XCTAssertEquals(1, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testGetCredential() {
        do {
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Count and list.
            XCTAssertEqual(2, doc.getCredentialCount())
            var vcs = doc.getCredentials()
            XCTAssertEqual(2, vcs.count)

            vcs.forEach { vc in
                XCTAssertEqual(doc.subject!, vc.id.did)
                XCTAssertEqual(doc.subject!, vc.subject.id)
                let re: Bool = vc.id.fragment.hasPrefix("credential-")
                XCTAssertTrue(re)
            }

            // Credential getter.
            var vc = try doc.getCredential("credential-1")
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "credential-1"), vc!.id)

            vc = try doc.getCredential(DIDURL(doc.subject!, "credential-2"))
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "credential-2"), vc!.id)

            // Credential not exist.
            vc = try doc.getCredential("credential-3")
            XCTAssertNil(vc)

            // TODO: Credential selector.
            vcs = try doc.selectCredentials(try DIDURL(doc.subject!, "credential-1"), "SelfProclaimedCredential")
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "credential-1"), vcs[0].id)

            vcs = try doc.selectCredentials(try DIDURL(doc.subject!, "credential-1"), nil)
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "credential-1"), vcs[0].id)

            vcs = try doc.selectCredentials(nil, "SelfProclaimedCredential")
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "credential-1"), vcs[0].id)

            vcs = try doc.selectCredentials(nil, "TestingCredential")
            XCTAssertEqual(0, vcs.count)
        } catch {
            print(error)
        }
    }
    
    func testAddCredential() {
        do {
            let json: String = "{\"id\":\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#test-cred\",\"type\":[\"SelfProclaimedCredential\",\"BasicProfileCredential\"],\"issuanceDate\":\"2019-01-01T19:20:18Z\",\"credentialSubject\":{\"id\":\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\",\"purpose\": \"Testing\"},\"proof\":{\"type\":\"ECDSAsecp256r1\",\"verificationMethod\":\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#master-key\",\"signature\":\"pYw8XNi1..Cky6Ed=\"}}"

            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Read only mode, should fail.
            var vc = try VerifiableCredential.fromJson(json)
            var success: Bool = doc.addCredential(vc)
            XCTAssertFalse(success)

            _ = doc.modify()

            // Modification mode, should success.
            success = doc.addCredential(vc)
            XCTAssertTrue(success)

            // Credential already exist, should fail.
            success = doc.addCredential(vc)
            XCTAssertFalse(success)

            // Check new added credential.
            vc = try doc.getCredential("test-cred")!
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "test-cred"), vc.id)

            // Should contains 3 credentials.
            XCTAssertEqual(3, doc.getCredentialCount())
        } catch {
            print(error)
        }
    }
    
    func testRemoveCredential() {
        do {
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Read only mode, should fail.
            var success: Bool = try doc.removeCredential("credential-1")
            XCTAssertFalse(success)

            success = doc.removeCredential(try DIDURL(doc.subject!, "credential-2"))
            XCTAssertFalse(success)

            _ = doc.modify()

            // Modification mode, should success.
            success = try doc.removeCredential("credential-1")
            XCTAssertTrue(success)

            success = doc.removeCredential(try DIDURL(doc.subject!, "credential-2"))
            XCTAssertTrue(success)

            var vc = try doc.getCredential("credential-1")
            XCTAssertNil(vc)

            vc = try doc.getCredential(DIDURL(doc.subject!, "credential-2"))
            XCTAssertNil(vc)

            // Credential not exist, should fail.
            success = try doc.removeCredential("notExistCredential-1")
            XCTAssertFalse(success)

            success = doc.removeCredential(try DIDURL(doc.subject!, "notExistCredential-2"))
            XCTAssertFalse(success)

            // Should no credentials in the document.
            XCTAssertEqual(0, doc.getCredentialCount())
        } catch {
            print(error)
        }
    }
    
    func testGetService() {
        do {
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Count and list
            XCTAssertEqual(3, doc.getServiceCount())
            var svcs = doc.getServices()
            XCTAssertEqual(3, svcs.count)

            svcs.forEach { svc in
                XCTAssertEqual(doc.subject!, svc.id.did)
                let re = svc.id.fragment == "openid"
                    || svc.id.fragment == "vcr"
                    || svc.id.fragment == "carrier"
                XCTAssertTrue(re )
            }

            // Service getter, should success.
            var svc = try doc.getService("openid")
            XCTAssertNotNil(svc)
            XCTAssertEqual(try DIDURL(doc.subject!, "openid"), svc!.id)
            XCTAssertEqual("OpenIdConnectVersion1.0Service", svc!.type)
            XCTAssertEqual("https://openid.example.com/", svc!.endpoint)

            svc = doc.getService(try DIDURL(doc.subject!, "vcr"))
            XCTAssertNotNil(svc)
            XCTAssertEqual(try DIDURL(doc.subject!, "vcr"), svc!.id)

            // Service not exist, should fail.
            svc = try doc.getService("notExistService")
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
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Read only mode. should fail.
            var success: Bool = try doc.addService("test-svc-0",
                    "Service.Testing", "https://www.elastos.org/testing0")
            XCTAssertFalse(success)

            success = try doc.addService(DIDURL(doc.subject!, "test-svc-1"),
                    "Service.Testing", "https://www.elastos.org/testing1")
            XCTAssertFalse(success)

            _ = doc.modify()

            // Modification mode. should success.
            success = try doc.addService("test-svc-0",
                    "Service.Testing", "https://www.elastos.org/testing0")
            XCTAssertTrue(success)

            success = try doc.addService(DIDURL(doc.subject!, "test-svc-1"),
                    "Service.Testing", "https://www.elastos.org/testing1")
            XCTAssertTrue(success)

            // Service id already exist, should failed.
            success = try doc.addService("vcr", "test", "https://www.elastos.org/test");
            XCTAssertFalse(success)
            
            XCTAssertEqual(5, doc.getServiceCount())

            // Try to select new added 2 services
            let svcs = try doc.selectServices(nil, "Service.Testing")
            XCTAssertEqual(2, svcs.count)
            XCTAssertEqual("Service.Testing", svcs[0].type)
        } catch {
            print(error)
        }
    }
    
    func testRemoveService() {
        do {
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Read only mode, should fail.
            var success: Bool = try doc.removeService("openid")
            XCTAssertFalse(success)

            success = doc.removeService(try DIDURL(doc.subject!, "vcr"))
            XCTAssertFalse(success)

            _ = doc.modify()

            // Modification mode, should success.
            success = try doc.removeService("openid")
            XCTAssertTrue(success)

            success = doc.removeService(try DIDURL(doc.subject!, "vcr"))
            XCTAssertTrue(success)

            var svc = try doc.getService("openid")
            XCTAssertNil(svc)

            svc = doc.getService(try DIDURL(doc.subject!, "vcr"))
            XCTAssertNil(svc)

            // Service not exist, should fail.
            success = try doc.removeService("notExistService")
            XCTAssertFalse(success)
            XCTAssertEqual(1, doc.getServiceCount())
        } catch {
            print(error)
        }
    }

    func testParseDocument() {
        do {
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)
            
            XCTAssertEqual(4, doc.getPublicKeyCount())
            
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
            XCTAssertEqual(2, doc.getCredentialCount())
            XCTAssertEqual(3, doc.getServiceCount())
        } catch {
        }
    }
    
    func testCompactJson() {
        
        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        let jsonString: String = try! document.toExternalForm(true)
        let url = URL(fileURLWithPath:compactPath)
        let jsonStr = try! String(contentsOf: url)
        XCTAssertEqual(jsonString, jsonStr)
    }
    
    func  testNormalizedJson() {
        
        let document: DIDDocument = try! DIDDocument.fromJson(documentPath)
        let str: String = try! document.toExternalForm(false)
        let url = URL(fileURLWithPath: normalizedPath)
        let jsonStr = try! String(contentsOf: url)
        XCTAssertEqual(str, jsonStr)
    }
    
    func test31SignAndVerify() {
        do {
            // add ids
            try DIDStore.creatInstance("filesystem", storePath, FakeConsoleAdaptor())
            store = try DIDStore.shareInstance()
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)

            var ids: Dictionary<DID, String> = [: ]
            for i in 0..<100 {
                let hint: String = "my did \(i)"
                let doc: DIDDocument = try! store.newDid(storePass, hint)
                let id: String = doc.subject!.methodSpecificId!
                let path: String = storePath + "/" + "ids" + "/" + id + "/" + "document"
                XCTAssertTrue(TestUtils.existsFile(path))
                
                let path2: String = storePath + "/" + "ids" + "/" + "." + id + ".meta"
                XCTAssertTrue(TestUtils.existsFile(path2))
                ids[doc.subject!] = hint
            }
        
            try ids.keys.forEach { did in
                let doc: DIDDocument = try store.loadDid(did)!
                var json: String = try doc.toExternalForm(false)
                let pkid: DIDURL = try DIDURL(did, "primary")
                var inputs: [CVarArg] = [json, json.count]
                var count: Int = inputs.count / 2
                var sig: String = try doc.sign(pkid, storePass, count, inputs)
                var re: Bool = try doc.verify(pkid, sig, count, inputs)
                XCTAssertTrue(re)
                
                sig = try doc.sign(storePass, count,inputs)
                re = try doc.verify(sig, count,inputs)
                XCTAssertTrue(re)
                
                json = String(json.suffix(json.count - 1))
                inputs = [json, json.count]
                count = inputs.count / 2
                re = try doc.verify(pkid, sig, count, inputs)
                XCTAssertFalse(re)
                re = try doc.verify(sig, count,inputs)
                XCTAssertFalse(re)
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
