
import XCTest
import ElastosDIDSDK

class DIDDoucumentTests: XCTestCase {
    
    var store: DIDStore!
    
    var compactPath: String!
    var documentPath: String!
    var normalizedPath: String!
    
    func testGetPublicKey() {
        do {
            let met: Metadata = Metadata()
            print(met)
            print(met)
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertTrue(try doc.isValid())
            
            // Count and list.
            XCTAssertEqual(4, doc.getPublicKeyCount())
            
            var pks:Array<DIDPublicKey> = doc.getPublicKeys()
            XCTAssertEqual(4, pks.count)
            
            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEqual("ECDSAsecp256r1", pk.type)
                
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
            pks = try doc.selectPublicKeys(id: id, type: "ECDSAsecp256r1")
            XCTAssertEqual(1, pks.count);
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), pks[0].id)
            
            pks = try doc.selectPublicKeys(id: id)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), pks[0].id)
            
            pks = try doc.selectPublicKeys(type: "ECDSAsecp256r1")
            XCTAssertEqual(4, pks.count)
            
            pks = try doc.selectPublicKeys("key2", type: "ECDSAsecp256r1")
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
            
            pks = try doc.selectPublicKeys("key3")
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key3"), pks[0].id)
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testAddPublicKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add 2 public keys
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey = try TestData.generateKeypair()
            var success: Bool = try db.addPublicKey(id, doc.subject!, key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try db.addPublicKey("test2", doc.subject!.description,
                                           try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            doc = try db.seal(storepass: storePass)
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
            XCTFail()
        }
    }
    
    func testRemovePublicKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            let db: DIDDocumentBuilder = doc.edit()
            
            // recovery used by authorization, should failed.
            let id: DIDURL = try DIDURL(doc.subject!, "recovery")
            var success: Bool = try db.removePublicKey(id)
            XCTAssertFalse(success)
            
            // force remove public key, should success
            success = try db.removePublicKey(id, true)
            XCTAssertTrue(success)
            
            success = try db.removePublicKey("key2", true)
            XCTAssertTrue(success)
            // Key not exist, should fail.
            success = try db.removePublicKey("notExistKey", true)
            XCTAssertFalse(success)
            
            // Can not remove default publickey, should fail.
            success = try db.removePublicKey(doc.getDefaultPublicKey(), true)
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk = try doc.getPublicKey("recovery")
            XCTAssertNil(pk)
            
            pk = try doc.getPublicKey("key2")
            XCTAssertNil(pk)
            
            // Check the final count.
            XCTAssertEqual(2, doc.getPublicKeyCount())
            XCTAssertEqual(2, doc.getAuthenticationKeyCount())
            XCTAssertEqual(0, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
            XCTFail()
        }
        
    }
    
    func testGetAuthenticationKey() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Count and list.
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            
            var pks: Array<DIDPublicKey> = doc.getAuthenticationKeys()
            XCTAssertEqual(3, pks.count)
            
            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEqual("ECDSAsecp256r1", pk.type)
                XCTAssertEqual(doc.subject, pk.controller)
                let re = pk.id.fragment == "primary" || pk.id.fragment == "key2" || pk.id.fragment == "key3"
                XCTAssertTrue(re)
            }
            
            // AuthenticationKey getter
            var pk = try doc.getAuthenticationKey("primary")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "primary"), pk!.id)
            
            var id: DIDURL = try DIDURL(doc.subject!, "key3")
            pk = try doc.getAuthenticationKey(id)
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk!.id)
            
            // Key not exist, should fail.
            pk = try doc.getAuthenticationKey("notExist")
            XCTAssertNil(pk)
            id = try DIDURL(doc.subject!, "notExist")
            pk = try doc.getAuthenticationKey(id)
            XCTAssertNil(pk)
            
            // selector
            id = try DIDURL(doc.subject!, "key3")
            pks = try doc.selectAuthenticationKeys(id: id, type: "ECDSAsecp256r1")
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            pks = try doc.selectAuthenticationKeys(id: id)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            
            pks = try doc.selectAuthenticationKeys(type: "ECDSAsecp256r1")
            XCTAssertEqual(3, pks.count)
            
            pks = try doc.selectAuthenticationKeys("key2", type: "ECDSAsecp256r1")
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
            
            pks = try doc.selectAuthenticationKeys("key2")
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
        } catch {
            print(error)
            XCTFail()
        }
        
    }
    
    func testAddAuthenticationKey() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add 2 public keys for test.
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey  = try TestData.generateKeypair()
            var success: Bool = try db.addPublicKey(id, doc.subject!,
                                                     try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            key = try TestData.generateKeypair()
            success = try db.addPublicKey("test2", doc.subject!.description,
                                           try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Add by reference
            success = try db.addAuthenticationKey(DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try db.addAuthenticationKey("test2")
            XCTAssertTrue(success)
            
            // Add new keys
            key = try TestData.generateKeypair()
            success = try db.addAuthenticationKey(DIDURL(doc.subject!, "test3"),
                                                   try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try db.addAuthenticationKey("test4", key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Try to add a non existing key, should fail.
            success = try db.addAuthenticationKey("notExistKey")
            XCTAssertFalse(success)
            
            // Try to add a key not owned by self, should fail.
            success = try db.addAuthenticationKey("recovery")
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk = try doc.getAuthenticationKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk!.id)
            
            pk = try doc.getAuthenticationKey("test2")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test2"), pk!.id)
            
            pk = try doc.getAuthenticationKey("test3")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test3"), pk!.id)
            
            pk = try doc.getAuthenticationKey("test4")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test4"), pk!.id)
            
            // Check the final count.
            XCTAssertEqual(8, doc.getPublicKeyCount())
            XCTAssertEqual(7, doc.getAuthenticationKeyCount())
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testRemoveAuthenticationKey() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
        
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add 2 public keys for test
            var key: DerivedKey  = try TestData.generateKeypair()
            var success: Bool = try db.addAuthenticationKey(
                try DIDURL(doc.subject!, "test1"), key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try db.addAuthenticationKey("test2", key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Remote keys
            success = db.removeAuthenticationKey(try DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try db.removeAuthenticationKey("test2")
            XCTAssertTrue(success)
            
            success = try db.removeAuthenticationKey("key2")
            XCTAssertTrue(success)
            
            // Key not exist, should fail.
            success = try db.removeAuthenticationKey("notExistKey")
            XCTAssertFalse(success)
            
            
            // Default publickey, can not remove, should fail.
            success = db.removeAuthenticationKey(doc.getDefaultPublicKey())
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk = try doc.getAuthenticationKey("test1")
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
            XCTFail()
        }
    }
    
    func testGetAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Count and list.
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
            
            var pks: Array<DIDPublicKey> = doc.getAuthorizationKeys()
            XCTAssertEqual(1, pks.count)
            
            for pk in pks {
                XCTAssertEqual(doc.subject, pk.id.did)
                XCTAssertEqual("ECDSAsecp256r1", pk.type)
                
                XCTAssertNotEqual(doc.subject, pk.controller)
                XCTAssertTrue(pk.id.fragment == "recovery")
            }
            
            // AuthorizationKey getter
            var pk = try doc.getAuthorizationKey("recovery")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "recovery"), pk!.id)
            
            var id: DIDURL = try DIDURL(doc.subject!, "recovery")
            pk = try doc.getAuthorizationKey(id)!
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk!.id)
            
            // Key not exist, should fail.
            pk = try doc.getAuthorizationKey("notExistKey")
            XCTAssertNil(pk)
            id = try DIDURL(doc.subject!, "notExistKey")
            pk = try doc.getAuthorizationKey(id)
            XCTAssertNil(pk)
            
            // Selector
            id = try DIDURL(doc.subject!, "recovery")
            pks = try doc.selectAuthorizationKeys(id: id, type: "ECDSAsecp256r1")
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            
            pks = try doc.selectAuthorizationKeys(id: id)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)
            pks = try doc.selectAuthorizationKeys(type: "ECDSAsecp256r1")
            XCTAssertEqual(1, pks.count)
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testAddAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add 2 public keys for test.
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey = try TestData.generateKeypair()
            let did = DID(DID.METHOD, DerivedKey.getIdString(try key.getPublicKeyBytes()))
            var success: Bool = try db.addPublicKey(id, did, try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair();
            success = try db.addPublicKey("test2", did.description,
                                           try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Add by reference
            success = try db.addAuthorizationKey(DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try db.addAuthorizationKey("test2")
            XCTAssertTrue(success)
            
            // Add new keys
            key = try TestData.generateKeypair()
            success = try db.addAuthorizationKey(try DIDURL(doc.subject!, "test3"),
                                                  did, try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try db.addAuthorizationKey("test4", did.description,
                                                  try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Try to add a non existing key, should fail.
            success = try db.addAuthorizationKey("notExistKey")
            XCTAssertFalse(success)
            
            // Try to add key owned by self, should fail.
            success = try db.addAuthorizationKey("key2")
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            var pk = try doc.getAuthorizationKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk!.id)
            pk = try doc.getAuthorizationKey("test2")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test2"), pk!.id)
            pk = try doc.getAuthorizationKey("test3")
            
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test3"), pk!.id)
            
            pk = try doc.getAuthorizationKey("test4")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test4"), pk!.id)
            
            // Check the final key count.
            XCTAssertEqual(8, doc.getPublicKeyCount())
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            XCTAssertEqual(5, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testRemoveAuthorizationKey() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add 2 keys for test.
            let id: DIDURL = try DIDURL(doc.subject!, "test1")
            var key: DerivedKey  = try TestData.generateKeypair()
            let did = DID(DID.METHOD, DerivedKey.getIdString(try key.getPublicKeyBytes()))
            var success: Bool = try db.addAuthorizationKey(id,
                                                            did, try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            key = try TestData.generateKeypair()
            success = try db.addAuthorizationKey("test2",
                                             did.description,
                                             try key.getPublicKeyBase58())
            XCTAssertTrue(success)
            
            // Remove keys.
            success = db.removeAuthorizationKey(try DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)
            
            success = try db.removeAuthorizationKey("recovery")
            XCTAssertTrue(success)
            
            // Key not exist, should fail.
            success = try db.removeAuthorizationKey("notExistKey")
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check existence
            var pk = try doc.getAuthorizationKey("test1")
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
            XCTFail()
        }
    }
    
    func testGetCredential() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
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
            var vc = try doc.getCredential("profile")
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vc!.id)
            
            vc = try doc.getCredential(DIDURL(doc.subject!, "email"))
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "email"), vc!.id)
            
            // Credential not exist.
            vc = try doc.getCredential("notExistVc")
            XCTAssertNil(vc)
            
            // Credential selector.
            vcs = try doc.selectCredentials(id: DIDURL(doc.subject!, "profile"),
                                            type: "SelfProclaimedCredential")
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vcs[0].id)
            
            vcs = try doc.selectCredentials(id: DIDURL(doc.subject!, "profile"))
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vcs[0].id)
            
            vcs = try doc.selectCredentials(type: "SelfProclaimedCredential")
            XCTAssertEqual(1, vcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "profile"), vcs[0].id)
            
            vcs = try doc.selectCredentials(type: "TestingCredential")
            XCTAssertEqual(0, vcs.count)
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testAddCredential() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add credentials.
            var vc = try testData.loadPassportCredential()
            var success: Bool = db.addCredential(vc!)
            XCTAssertTrue(success)
            
            vc = try testData.loadTwitterCredential()
            success = db.addCredential(vc!)
            XCTAssertTrue(success)
            
            // Credential already exist, should fail.
            success = db.addCredential(vc!)
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check new added credential.
            vc = try doc.getCredential("passport")
            XCTAssertNotNil(vc)
            XCTAssertEqual(try DIDURL(doc.subject!, "passport"), vc!.id)
            
            let id: DIDURL = try DIDURL(doc.subject!, "twitter")
            vc = try doc.getCredential(id)!
            XCTAssertNotNil(vc)
            XCTAssertEqual(id, vc!.id)
            
            // Should contains 3 credentials.
            XCTAssertEqual(4, doc.getCredentialCount())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testRemoveCredential() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add test credentials.
            var vc = try testData.loadPassportCredential()
            var success: Bool = db.addCredential(vc!)
            XCTAssertTrue(success)
            
            vc = try testData.loadTwitterCredential()
            success = db.addCredential(vc!)
            XCTAssertTrue(success)
            // Remove credentials
            success = try db.removeCredential("profile")
            XCTAssertTrue(success)
            
            success = db.removeCredential(try DIDURL(doc.subject!, "twitter"))
            XCTAssertTrue(success)
            
            // Credential not exist, should fail.
            success = try db.removeCredential("notExistCredential")
            XCTAssertFalse(success)
            success = db.removeCredential(try DIDURL(doc.subject!,
                                                  "notExistCredential"))
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
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
            XCTFail()
        }
    }
    
    func testGetService() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
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
            var svc = try doc.getService("openid")
            XCTAssertNotNil(svc)
            XCTAssertEqual(try DIDURL(doc.subject!, "openid"), svc!.id)
            XCTAssertEqual("OpenIdConnectVersion1.0Service", svc!.type)
            XCTAssertEqual("https://openid.example.com/", svc!.endpoint)
            
            svc = try doc.getService(DIDURL(doc.subject!, "vcr"))!
            XCTAssertNotNil(svc)
            XCTAssertEqual(try DIDURL(doc.subject!, "vcr"), svc!.id)
            
            // Service not exist, should fail.
            svc = try doc.getService("notExistService")
            XCTAssertNil(svc)
            
            // Service selector.
            svcs = try doc.selectServices("vcr", type: "CredentialRepositoryService")
            XCTAssertEqual(1, svcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "vcr"), svcs[0].id)
            
            svcs = try doc.selectServices(id: DIDURL(doc.subject!, "openid"))
            XCTAssertEqual(1, svcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "openid"), svcs[0].id)
            
            svcs = try doc.selectServices(type: "CarrierAddress")
            XCTAssertEqual(1, svcs.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "carrier"), svcs[0].id)
            
            // Service not exist, should return a empty list.
            svcs = try doc.selectServices("notExistService", type: "CredentialRepositoryService")
            XCTAssertEqual(0, svcs.count)
            
            svcs = try doc.selectServices(type: "notExistType")
            XCTAssertEqual(0, svcs.count)
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testAddService() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // Add services
            var success: Bool = try db.addService("test-svc-1",
                                              "Service.Testing", "https://www.elastos.org/testing1")
            XCTAssertTrue(success)
            
            success = try db.addService(DIDURL(doc.subject!, "test-svc-2"),
                                     "Service.Testing", "https://www.elastos.org/testing2")
            XCTAssertTrue(success)
            
            // Service id already exist, should failed.
            success = try db.addService("vcr", "test", "https://www.elastos.org/test")
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            // Check the final count
            XCTAssertEqual(5, doc.getServiceCount())
            
            // Try to select new added 2 services
            let svcs: Array<Service> = try doc.selectServices(type:"Service.Testing")
            XCTAssertEqual(2, svcs.count)
            XCTAssertEqual("Service.Testing", svcs[0].type)
            XCTAssertEqual("Service.Testing", svcs[0].type)
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testRemoveService() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let db: DIDDocumentBuilder = doc.edit()
            
            // remove services
            var success: Bool = try db.removeService("openid")
            XCTAssertTrue(success)
            
            success = db.removeService(try DIDURL(doc.subject!, "vcr"))
            XCTAssertTrue(success)
            
            // Service not exist, should fail.
            success = try db.removeService("notExistService")
            XCTAssertFalse(success)
            
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            var svc = try doc.getService("openid")
            XCTAssertNil(svc)
            
            svc = try doc.getService(DIDURL(doc.subject!, "vcr"))
            XCTAssertNil(svc)
            
            // Check the final count
            XCTAssertEqual(1, doc.getServiceCount())
        } catch {
            XCTFail()
        }
    }
    
    func testParseAndSerializeDocument() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
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
            
            XCTAssertEqual(try testData.loadTestNormalizedJson(), compact.description(true))
            XCTAssertEqual(try testData.loadTestNormalizedJson(), normalized.description(true))
            XCTAssertEqual(try testData.loadTestNormalizedJson(), doc.description(true))
            
            XCTAssertEqual(try testData.loadTestCompactJson(), compact.description(false))
            XCTAssertEqual(try testData.loadTestCompactJson(), normalized.description(false))
            XCTAssertEqual(try testData.loadTestCompactJson(), doc.description(false))
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func test31SignAndVerify() {
        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()

            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let pkid: DIDURL = try DIDURL(doc.subject!, "primary")
            
            for _ in 0..<10 {
                var json = doc.description(false)
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
            XCTFail()
        }
    }

    func testSignAndVerifyNew() {

        do {
            let testData: TestData = TestData()
            _ = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc: DIDDocument = try testData.loadTestDocument()
            XCTAssertNotNil(doc)
            XCTAssertTrue(try doc.isValid())
            
            let pkid: DIDURL = try DIDURL(doc.subject!, "primary")
            
            for i in 0..<10 {
                var data: [String]  = Array(repeating: String(i), count: 1024)
                var inputString = data.joined(separator: "")
                
                var inputs: [CVarArg] = [inputString, inputString.count]
                let count = inputs.count / 2
                
                var sig: String = try doc.sign(pkid, storePass, 1, inputs)
                var result: Bool = try doc.verify(sig, count, inputs)
                XCTAssertTrue(result)
                
                data[0] = String(i + 1)
                inputString = data.joined(separator: "")
                inputs = [inputString, inputString.count]
                result = try doc.verify(sig, count, inputs)
                XCTAssertFalse(result)

                sig = try doc.sign(storePass, count, inputs)
                result = try doc.verify(sig, count, inputs)
                XCTAssertTrue(result)
                
                data[0] = String(i + 2)
                inputString = data.joined(separator: "")
                inputs = [inputString, inputString.count]
                result = try doc.verify(sig, count, inputs)
                XCTAssertFalse(result)
            }
            
        } catch {
            print(error)
            XCTFail()
        }
        
    }

}
