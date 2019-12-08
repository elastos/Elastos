
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
       _ = doc.modify()
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
        doc.readonly = true
        return doc
    }

    
    func testGetPublicKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)
            // Count and list.
            XCTAssertEqual(4, doc.getPublicKeyCount())

            var pks = doc.getPublicKeys()
            XCTAssertEqual(4, pks.count)

            pks.forEach { pk in
                XCTAssertEqual(doc.subject!, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)
                if (pk.id.fragment == "recovery") {
                    XCTAssertNotEqual(doc.subject!, pk.controller!)
                }
                else
                {
                    XCTAssertEqual(doc.subject!, pk.controller)
                }
                let re = pk.id.fragment == "default" || pk.id.fragment == "key2" || pk.id.fragment == "key3" || pk.id.fragment == "recovery"
                XCTAssertTrue(re)
            }
            
             // PublicKey getter.
            var pk = try doc.getPublicKey("default")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"),  pk!.id)
            
            var id: DIDURL = try DIDURL(doc.subject!, "key3")
            pk = try doc.getPublicKey(id)!
            XCTAssertNotNil(pk)
            XCTAssertEqual(id,  pk!.id)
            
            id = doc.getDefaultPublicKey()!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), id)
            
            // Key not exist, should fail.
            pk = try doc.getPublicKey("notExist")
            XCTAssertNil(pk)
            
            id = try DIDURL(doc.subject!, "notExist")
            pk = try doc.getPublicKey(id)
            XCTAssertNil(pk)
            
            // Selector
            id = doc.getDefaultPublicKey()!
            pks = try doc.selectPublicKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), pks[0].id)
            
            pks = try doc.selectPublicKeys(id, nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), pks[0].id)

            pks = try doc.selectPublicKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEqual(4, pks.count)
            
            pks = try doc.selectPublicKeys("key2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)

            pks = try doc.selectPublicKeys("key2", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "key2"), pks[0].id)
        } catch {
            print(error)
        }
    }
    
    func testAddPublicKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)
            
            let data = Data()
            let udata = [UInt8](data)
            var keyBase58: String = Base58.base58FromBytes(udata)
            
            // Read only mode, should fail.
            var id: DIDURL = try DIDURL(doc.subject!, "test0")
            var success: Bool = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertFalse(success)
            
            success = try doc.addPublicKey("test0", doc.subject!.description, keyBase58)
            XCTAssertFalse(success)
            
            _ = doc.modify()

            // Modification mode, should success.
            id = try DIDURL(doc.subject!, "test0")
            success = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertTrue(success)
            
            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addPublicKey("test1", doc.subject!.description, keyBase58)
            XCTAssertTrue(success)
            
            var pk: DIDPublicKey = try doc.getPublicKey("test0")!
            XCTAssertNotNil(pk);
            XCTAssertEqual(try DIDURL(doc.subject!, "test0"), pk.id)

            pk = try doc.getPublicKey("test1")!
            XCTAssertNotNil(pk);
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk.id)

            // Check the final count.
            XCTAssertEqual(6, doc.getPublicKeyCount())
            
        } catch  {
        print(error)
        }
    }
    
    func testRemovePublicKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)

            try _ = updateForTesting(doc)

            // Read only mode, should fail.
            var id: DIDURL = try DIDURL(doc.subject!, "test-pk-0")
            var success: Bool = try doc.removePublicKey(id, true)
            XCTAssertFalse(success)

            success = try doc.removePublicKey("test-pk-1", true)
            XCTAssertFalse(success)
            _ = doc.modify()
            
            // Modification mode, should success.
            id = try DIDURL(doc.subject!, "test-auth-0")
            success = try doc.removePublicKey(id, true)
            XCTAssertTrue(success)

            success = try doc.removePublicKey("test-pk-0")
            XCTAssertTrue(success)

            success = try doc.removePublicKey("test-autho-0", true)
            XCTAssertTrue(success)

            var pk = try doc.getPublicKey("test-auth-0")
            XCTAssertNil(pk)
            
            pk = try doc.getPublicKey("test-pk-0")
            XCTAssertNil(pk)

            pk = try doc.getPublicKey("test-autho-0")
            XCTAssertNil(pk)
            
            // PublicKey used by authentication, can not remove directly, should fail.
            id = try DIDURL(doc.subject!, "test-auth-0")
            success = try doc.removePublicKey(id)
            XCTAssertFalse(success)

            // Key not exist, should fail.
            success = try doc.removePublicKey("notExistKey", true)
            XCTAssertFalse(success)
            
            // Can not remove default publickey, should fail.
            success = try doc.removePublicKey(doc.getDefaultPublicKey()!, true)
            XCTAssertFalse(success)

            // Check the final count.
            XCTAssertEqual(16, doc.getPublicKeyCount())
            XCTAssertEqual(7, doc.getAuthenticationKeyCount())
            XCTAssertEqual(5, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
        
    }
    
    func testGetAuthenticationKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Count and list.
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())

            var pks: Array<DIDPublicKey> = doc.getAuthenticationKeys()
            XCTAssertEqual(3, pks.count)
            
            pks.forEach { pk in
                XCTAssertEqual(doc.subject!, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)

                XCTAssertEqual(doc.subject!, pk.controller!)
                let re = pk.id.fragment == "default" || pk.id.fragment == "key2" || pk.id.fragment == "key3"
                XCTAssertTrue(re)
            }

            // AuthenticationKey getter
            var pk = try doc.getAuthenticationKey("default")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "default"), pk!.id)

            var id: DIDURL = try DIDURL(doc.subject!, "key3")
            pk = doc.getAuthenticationKey(id)!
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk!.id)
            
            // Key not exist, should fail.
            pk = try doc.getAuthenticationKey("notExist")
            XCTAssertNil(pk)

            id = try DIDURL(doc.subject!, "notExist")
            pk = doc.getAuthenticationKey(id)
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
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Add the keys for testing.
           _ = doc.modify()

            var id: DIDURL = try DIDURL(doc.subject!, "test1")
            let data = Data()
            let udata = [UInt8](data)
            var keyBase58: String = Base58.base58FromBytes(udata)
            
            var success: Bool = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertTrue(success)
            
            id = try DIDURL(doc.subject!, "test2")
            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addPublicKey(id, doc.subject!, keyBase58)
            XCTAssertTrue(success)

            doc.readonly = true
            // Read only mode, shoud fail.
            id = try DIDURL(doc.subject!, "test1")
            success = try doc.addAuthenticationKey(id)
            XCTAssertFalse(success)

            success = try doc.addAuthenticationKey("test2")
            XCTAssertFalse(success)

            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addAuthenticationKey("test3", keyBase58)
            XCTAssertFalse(success)
            
            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addAuthenticationKey(DIDURL(doc.subject!, "test4"), keyBase58)
            XCTAssertFalse(success)

            _ = doc.modify();

            // Modification mode, should success.
            success = try doc.addAuthenticationKey(DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)

            success = try doc.addAuthenticationKey("test2")
            XCTAssertTrue(success)

            keyBase58 = Base58.base58FromBytes(udata)

            success = try doc.addAuthenticationKey(DIDURL(doc.subject!, "test3"), keyBase58)
            XCTAssertTrue(success)
            
            keyBase58 = Base58.base58FromBytes(udata)

            success = try doc.addAuthenticationKey("test4", keyBase58)
            XCTAssertTrue(success)

            var pk: DIDPublicKey = try doc.getAuthenticationKey("test1")!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk.id)

            pk = try doc.getAuthenticationKey("test2")!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test2"), pk.id)

            pk = try doc.getAuthenticationKey("test3")!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test3"), pk.id)

            pk = try doc.getAuthenticationKey("test4")!
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test4"), pk.id)

            // Try to add a non existing key, should fail.
            success = try doc.addAuthenticationKey("test0")
            XCTAssertFalse(success)
            
            // Try to add a key not owned by self, should fail.
            success = try doc.addAuthenticationKey("recovery")
            XCTAssertFalse(success)

            // Check the final count.
            XCTAssertEqual(8, doc.getPublicKeyCount())
            XCTAssertEqual(7, doc.getAuthenticationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testRemoveAuthenticationKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)

            try _ = updateForTesting(doc)

            // Read only mode, should fail.
            var success: Bool = doc.removeAuthenticationKey(try DIDURL(doc.subject!, "test-auth-0"))
            XCTAssertFalse(success)

            success = try doc.removeAuthenticationKey("test-auth-1")
            XCTAssertFalse(success)

            _ = doc.modify()

            // Modification mode, should success.
            success = doc.removeAuthenticationKey(try DIDURL(doc.subject!, "test-auth-0"))
            XCTAssertTrue(success)

            success = try doc.removeAuthenticationKey("test-auth-1")
            XCTAssertTrue(success)

            success = try doc.removeAuthenticationKey("key2")
            XCTAssertTrue(success)

            var pk = try doc.getAuthenticationKey("test-auth-0")
            XCTAssertNil(pk)

            pk = try doc.getAuthenticationKey("test-auth-1")
            XCTAssertNil(pk)

            pk = try doc.getAuthenticationKey("key2")
            XCTAssertNil(pk)

            // Key not exist, should fail.
            success = try doc.removeAuthenticationKey("test-auth-10")
            XCTAssertFalse(success)

            // Default publickey, can not remove, should fail.
            success = doc.removeAuthenticationKey(doc.getDefaultPublicKey()!)
            XCTAssertFalse(success)

            // Check the final count.
            XCTAssertEqual(19, doc.getPublicKeyCount())
            XCTAssertEqual(5, doc.getAuthenticationKeyCount())
            XCTAssertEqual(6, doc.getAuthorizationKeyCount())

            XCTAssertEqual(19, doc.getPublicKeyCount())
            XCTAssertEqual(5, doc.getAuthenticationKeyCount())
            XCTAssertEqual(6, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testGetAuthorizationKey() {
        do {
            let doc: DIDDocument = try loadTestDocument()
            XCTAssertNotNil(doc)

            try _ = updateForTesting(doc)

            // Count and list.
            XCTAssertEqual(6, doc.getAuthorizationKeyCount())

            var pks = doc.getAuthorizationKeys()
            XCTAssertEqual(6, pks.count)

            pks.forEach { pk in
                XCTAssertEqual(doc.subject!, pk.id.did)
                XCTAssertEqual(Constants.defaultPublicKeyType, pk.type)

                XCTAssertNotEqual(doc.subject!, pk.controller!)
                let re = pk.id.fragment == "recovery" || pk.id.fragment.hasPrefix("test-autho-")
                XCTAssertTrue(re)
            }

            // AuthorizationKey getter
            var pk = try doc.getAuthorizationKey("recovery")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "recovery"), pk!.id)


            var id: DIDURL = try DIDURL(doc.subject!, "test-autho-0")
            pk = doc.getAuthorizationKey(id)
            XCTAssertNotNil(pk)
            XCTAssertEqual(id, pk!.id)

            // Key not exist, should fail.
            pk = try doc.getAuthorizationKey("notExist")
            XCTAssertNil(pk)

            id = try DIDURL(doc.subject!, "notExist")
            pk = doc.getAuthorizationKey(id)
            XCTAssertNil(pk)

            // Selector
            id = try DIDURL(doc.subject!, "test-autho-1")
            pks = try doc.selectAuthorizationKeys(id, Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)

            pks = try doc.selectAuthorizationKeys(id, nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(id, pks[0].id)

            pks = try doc.selectAuthorizationKeys(nil, Constants.defaultPublicKeyType)
            XCTAssertEqual(6, pks.count)

            pks = try doc.selectAuthorizationKeys("test-autho-2", Constants.defaultPublicKeyType)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "test-autho-2"), pks[0].id)

            pks = try doc.selectAuthorizationKeys("test-autho-2", nil)
            XCTAssertEqual(1, pks.count)
            XCTAssertEqual(try DIDURL(doc.subject!, "test-autho-2"), pks[0].id)
        } catch {
            print(error)
        }
    }
    
    func testAddAuthorizationKey() {
        do {
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)

            // Add the testing keys
            _ = doc.modify()

            let controller: DID = try DID("did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh")

            var id: DIDURL = try DIDURL(doc.subject!, "test1")
            let data = Data()
            let udata = [UInt8](data)
            var keyBase58: String = Base58.base58FromBytes(udata)
            var success: Bool = try doc.addPublicKey(id, controller, keyBase58)
            XCTAssertTrue(success)

            id = try DIDURL(doc.subject!, "test2")
            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addPublicKey(id, controller, keyBase58)
            XCTAssertTrue(success)

            doc.readonly = true

            // Read only mode, should fail.
            id = try DIDURL(doc.subject!, "test1")
            success = try doc.addAuthorizationKey(id)
            XCTAssertFalse(success)

            success = try doc.addAuthorizationKey("test2")
            XCTAssertFalse(success)

            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addAuthorizationKey("test3", controller.description, keyBase58)
            XCTAssertFalse(success)

            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addAuthorizationKey(DIDURL(doc.subject!, "test4"), controller, keyBase58)
            XCTAssertFalse(success)

            _ = doc.modify()

            // Modification mode, should success.
            success = try doc.addAuthorizationKey(DIDURL(doc.subject!, "test1"))
            XCTAssertTrue(success)

            success = try doc.addAuthorizationKey("test2")
            XCTAssertTrue(success)

            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addAuthorizationKey(DIDURL(doc.subject!, "test3"), controller, keyBase58)
            XCTAssertTrue(success)

            keyBase58 = Base58.base58FromBytes(udata)
            success = try doc.addAuthorizationKey("test4", controller.description, keyBase58)
            XCTAssertTrue(success)

            var pk = try doc.getAuthorizationKey("test1")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test1"), pk?.id)

            pk = try doc.getAuthorizationKey("test2")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test2"), pk?.id)

            pk = try doc.getAuthorizationKey("test3")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test3"), pk?.id)

            pk = try doc.getAuthorizationKey("test4")
            XCTAssertNotNil(pk)
            XCTAssertEqual(try DIDURL(doc.subject!, "test4"), pk?.id)

            // Try to add a non existing key, should fail.
            success = try doc.addAuthorizationKey("test0")
            XCTAssertFalse(success)

            // Try to add key owned by self, should fail.
            success = try doc.addAuthorizationKey("key2")
            XCTAssertFalse(success)

            // Check the final key count.
            XCTAssertEqual(8, doc.getPublicKeyCount())
            XCTAssertEqual(5, doc.getAuthorizationKeyCount())
        } catch {
            print(error)
        }
    }
    
    func testRemoveAuthorizationKey() {
        do {
            let doc = try loadTestDocument()
            XCTAssertNotNil(doc)
            
            try _ = updateForTesting(doc)

            // Read only mode, should fail.
            var success: Bool = doc.removeAuthorizationKey(try DIDURL(doc.subject!, "test-autho-0"))
            XCTAssertFalse(success)

            success = try doc.removeAuthorizationKey("test-autho-1")
            XCTAssertFalse(success)

            _ = doc.modify()

            // Modification mode, should success.
            success = doc.removeAuthorizationKey(try DIDURL(doc.subject!, "test-autho-0"))
            XCTAssertTrue(success)

            success = try doc.removeAuthorizationKey("test-autho-1")
            XCTAssertTrue(success)

            success = try doc.removeAuthorizationKey("recovery")
            XCTAssertTrue(success)

            var pk = try doc.getAuthorizationKey("test-autho-0")
            XCTAssertNil(pk)

            pk = try doc.getAuthorizationKey("test-autho-1")
            XCTAssertNil(pk)

            pk = try doc.getAuthorizationKey("recovery")
            XCTAssertNil(pk)

            // Key not exist, should fail.
            success = try doc.removeAuthorizationKey("test-autho-10")
            XCTAssertFalse(success)

            // Check the final count.
            XCTAssertEqual(19, doc.getPublicKeyCount())
            XCTAssertEqual(8, doc.getAuthenticationKeyCount())
            XCTAssertEqual(3, doc.getAuthorizationKeyCount())
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
