
import XCTest
import ElastosDIDSDK

class DIDStoreTests: XCTestCase {
    
    var store: DIDStore!
    static var ids: Dictionary<DID, String> = [: ]
    static var primaryDid: DID!
    var adapter: SPVAdaptor!
    
    func testCreateEmptyStore() {
        do {
            let testData: TestData = TestData()
            try _ = testData.setupStore(true)
            _ = testData.exists(storePath)
            
            let path = storePath + "/" + ".meta"
            _ = testData.existsFile(path)
        } catch {
            print("testCreateEmptyStore error: \(error)")
            XCTFail()
        }
    }
    
    func testCreateDidInEmptyStore()  {
        do {
            let testData: TestData = TestData()
            
            let store = try testData.setupStore(true)
            try store.newDid(storePass, "this will be fail")
        } catch {
            print(error)
            XCTAssertTrue(true)
        }
    }

    func testInitPrivateIdentity0() {
        do {
            let testData: TestData = TestData()
            var store: DIDStore = try testData.setupStore(true)
            
            XCTAssertFalse(try store.containsPrivateIdentity())
            
            _ = try testData.initIdentity()
            XCTAssertTrue(try store.containsPrivateIdentity())
                        
            var path = storePath + "/" + "private" + "/" + "key"
            XCTAssertTrue(testData.existsFile(path))
            path = storePath + "/" + "private" + "/" + "index"
            XCTAssertTrue(testData.existsFile(path))
            
           store = try DIDStore.open("filesystem", storePath)
            
            XCTAssertTrue(try store.containsPrivateIdentity())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testCreateDIDWithAlias() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let alias: String = "my first did"
            
            let doc: DIDDocument = try store.newDid(storePass, alias)
            XCTAssertTrue(try doc.isValid())
            
            var resolved = try doc.subject?.resolve()
            XCTAssertNil(resolved)
            
            _ = try store.publishDid(doc.subject!, storePass)
            var path = storePath
            
            path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/document"
            XCTAssertTrue(testData.existsFile(path))
            path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/.meta"
            XCTAssertTrue(testData.existsFile(path))
            
            resolved = try store.resolveDid(doc.subject!, true)!
            
            XCTAssertNotNil(resolved)
            XCTAssertEqual(alias, try resolved!.getAlias())
            XCTAssertEqual(doc.subject, resolved!.subject)
            XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
            
            XCTAssertTrue(try resolved!.isValid())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testCreateDIDWithoutAlias() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            try testData.initIdentity()
            
            let doc: DIDDocument = try store.newDid(storePass)
            XCTAssertTrue(try doc.isValid())
            
            var resolved = try doc.subject?.resolve(true)
            XCTAssertNil(resolved)
            
            try store.publishDid(doc.subject!, storePass)
            // todo change
//            try store.publishDid(doc.subject, storePass)
            
            
            var path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/document"
            XCTAssertTrue(testData.existsFile(path))
            // todo isFile

//            path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/.meta"
//            XCTAssertFalse(testData.existsFile(path))
            
            resolved = try doc.subject?.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.subject, resolved!.subject)
            XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
            XCTAssertTrue(try resolved!.isValid())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testUpdateDid() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc: DIDDocument = try store.newDid(storePass)
            XCTAssertTrue(try doc.isValid())
            _ = try store.publishDid(doc.subject!, storePass)
            
            var resolved = try! store.resolveDid(doc.subject!, true)
            try store.storeDid(resolved!)
            XCTAssertNotNil(resolved)
            
            // Update
            var key = try TestData.generateKeypair()
            _ = try resolved?.addAuthenticationKey("key1", try key.getPublicKeyBase58())
            var newDoc = try resolved!.seal(store, storePass)
            XCTAssertEqual(2, newDoc.getPublicKeyCount())
            XCTAssertEqual(2, newDoc.getAuthenticationKeyCount())
            
            print(newDoc.getTransactionId())
            _ = try store.publishDid(newDoc.subject!, storePass)
            try store.storeDid(newDoc)
            
            resolved = try newDoc.subject!.resolve()!

            resolved = try doc.subject!.resolve()!
            try store.storeDid(resolved!)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try newDoc.description(),try resolved?.description())
            
            // Update again
            key = try TestData.generateKeypair()
            try resolved?.addAuthenticationKey("key2", key.getPublicKeyBase58())
            newDoc = try! resolved!.seal(store, storePass)
            XCTAssertEqual(3, newDoc.getPublicKeyCount())
            XCTAssertEqual(3, newDoc.getAuthenticationKeyCount())
            try store.storeDid(newDoc)
            try store.publishDid(newDoc.subject!, storePass)
            
            resolved = try! doc.subject?.resolve(true)
            XCTAssertNotNil(resolved)
//            XCTAssertEqual(newDoc.description, resolved?.description)
        } catch {
            XCTFail()
        }
    }
    
    func testUpdateNonExistedDid() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            

            let doc = try store.newDid(storePass)
            XCTAssertTrue(try doc.isValid())
            // fake a txid
            let meta = DIDMeta()
            meta.store = store
            meta.transactionId = "12345678"
            try store.storeDidMeta(doc.subject!, meta)

            // Update will fail
            try store.publishDid(doc.subject!, storePass)
        } catch  {
            // todo:  Create ID transaction error.
            XCTAssertTrue(true)
        }
    }
    
    // TODO:
    func testDeactivateSelfAfterCreate() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc = try store.newDid(storePass)
            XCTAssertTrue(try doc.isValid())
            
            _ = try store.publishDid(doc.subject!, storePass)
            let resolved: DIDDocument = try doc.subject!.resolve(true)!
            XCTAssertNotNil(resolved)
            
            _ = try store.deactivateDid(doc.subject!, storePass)
            
            let resolvedNil: DIDDocument? = try doc.subject!.resolve(true)
            
            XCTAssertNil(resolvedNil)
        } catch  {
            XCTFail()
        }
    }
    
    // TODO:
    func testDeactivateSelfAfterUpdate() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc = try store.newDid(storePass)
            XCTAssertTrue(try doc.isValid())
            
            _ = try store.publishDid(doc.subject!, storePass)
            
            var resolved: DIDDocument! = try doc.subject!.resolve(true)
            XCTAssertNotNil(resolved)
            try store.storeDid(resolved!)
            
            let key = try TestData.generateKeypair()
            _ = try resolved?.addAuthenticationKey("key2", key.getPublicKeyBase58())
            let newDoc = try resolved!.seal(store, storePass)
            XCTAssertEqual(2, newDoc.getPublicKeyCount())
            XCTAssertEqual(2, newDoc.getAuthenticationKeyCount())
            try store.storeDid(newDoc)
            
            _ = try store.publishDid(newDoc.subject!, storePass)
            
            resolved = try doc.subject!.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try newDoc.toJson(nil, true, true),try resolved?.toJson(nil, true, true))
            try store.storeDid(resolved!)
            
            _ = try store.deactivateDid(newDoc.subject!, storePass)
            
            resolved = try doc.subject!.resolve(true)
            
            let resolvedNil: DIDDocument? = try doc.subject!.resolve(true)
            
            XCTAssertNil(resolvedNil)
        } catch  {
            XCTFail()
        }
    }
    
    // TODO:
    func testDeactivateWithAuthorization1() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc = try store.newDid(storePass)
            XCTAssertTrue(try doc.isValid())
            
            _ = try store.publishDid(doc.subject!, storePass)
            
            var resolved: DIDDocument! = try doc.subject!.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try doc.toJson(nil, true, true),try resolved?.toJson(nil, true, true))
            
            var target = try store.newDid(storePass)
            _ = try target.authorizationDid("recovery", doc.subject!.description)
            XCTAssertNotNil(target)
            XCTAssertEqual(1, target.getAuthorizationKeyCount())
            let controller = target.getAuthorizationKeys()[0].controller
            XCTAssertEqual(doc.subject, controller)
            target = try target.seal(store, storePass)
            
            _ = try store.publishDid(target.subject!, storePass)
            
            resolved = try target.subject!.resolve(true)!
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try target.toJson(nil, true, true),try resolved.toJson(nil, true, true))
            
            _ = try store.deactivateDid(target.subject!, doc.subject!, storePass)
            
            let resolvedNil: DIDDocument? = try target.subject!.resolve(true)
            
            XCTAssertNil(resolvedNil)
        } catch  {
            XCTFail()
        }
    }
    
    // TODO:
    func testDeactivateWithAuthorization2() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc = try store.newDid(storePass)
            let key = try TestData.generateKeypair()
            let id = try DIDURL(doc.subject!, "key-2")
            _ = try doc.addAuthenticationKey(id,try key.getPublicKeyBase58())
            try store.storePrivateKey(doc.subject!, id, key.getPrivateKeyData(), storePass)
            doc = try doc.seal(store, storePass)
            XCTAssertTrue(try doc.isValid())
            XCTAssertEqual(2, doc.getAuthorizationKeyCount())
            try store.storeDid(doc)
            
            _ = try store.publishDid(doc.subject!, storePass)
            
            var resolved: DIDDocument = try doc.subject!.resolve(true)!
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try doc.toJson(nil, true, true),try resolved.toJson(nil, true, true))
            
            var target: DIDDocument = try store.newDid(storePass)
            _ = try target.addAuthorizationKey("recovery", doc.subject!.description, key.getPrivateKeyBase58())
            target = try target.seal(store, storePass)
            XCTAssertNotNil(target)
            XCTAssertEqual(1, doc.getAuthorizationKeyCount())
            let controller = target.getAuthorizationKeys()[0].controller
            XCTAssertEqual(doc.subject, controller)
            try store.storeDid(target)
            
            _ = try store.publishDid(target.subject!, storePass)
            
            resolved = try target.subject!.resolve()!
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try target.toJson(nil, true, true),try resolved.toJson(nil, true, true))
            
            _ = try store.deactivateDid(target.subject!, doc.subject!, id, storePass)
            
            let resolvedNil: DIDDocument? = try target.subject!.resolve(true)
            
            XCTAssertNil(resolvedNil)
        } catch  {
            XCTFail()
        }
    }
    
    // TODO:
    func testDeactivateWithAuthorization3() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc = try store.newDid(storePass)
            let key = try TestData.generateKeypair()
            let id: DIDURL = try DIDURL(doc.subject!, "key-2")
            _ = try doc.addAuthenticationKey(id, key.getPrivateKeyBase58())
            
            try store.storePrivateKey(doc.subject!, id, key.getPrivateKeyData(), storePass)
            doc = try doc.seal(store, storePass)
            XCTAssertTrue(try doc.isValid())
            XCTAssertEqual(2, doc.getAuthenticationKeyCount())
            try store.storeDid(doc)
            
            _ = try store.publishDid(doc.subject!, storePass)
            
            var resolved: DIDDocument = try doc.subject!.resolve(true)!
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try doc.toJson(nil, true, true),try resolved.toJson(nil, true, true))
            
            var target = try store.newDid(storePass)
            _ = try target.addAuthorizationKey("recovery", doc.subject!.description, try key.getPublicKeyBase58())
            target = try target.seal(store, storePass)
            XCTAssertNotNil(target)
            XCTAssertEqual(1, target.getAuthorizationKeyCount())
            let controller = target.getAuthorizationKeys()[0].controller
            XCTAssertEqual(doc.subject, controller)
            try store.storeDid(target)
            
            _ = try store.publishDid(target.subject!, storePass)
            
            resolved = try target.subject!.resolve()!
            XCTAssertNotNil(resolved)
            XCTAssertEqual(try doc.toJson(nil, true, true),try resolved.toJson(nil, true, true))
            
            _ = try store.deactivateDid(target.subject!, doc.subject!, storePass)
            
            resolved = try target.subject!.resolve(true)!
            
            XCTAssertNil(resolved)
        } catch  {
            XCTFail()
        }
    }

    func testBulkCreate() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            for i in 0..<100 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(storePass, alias)
                XCTAssertTrue(try doc.isValid())
                
                var resolved = try store.resolveDid(doc.subject!, true)
                XCTAssertNil(resolved)
                
                try store.publishDid(doc.subject!, storePass)
                
                var path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/document"
                XCTAssertTrue(testData.existsFile(path))
                
                path = storePath + "/ids/" + doc.subject!.methodSpecificId + "/.meta"
                XCTAssertTrue(testData.existsFile(path))
                
                resolved = try doc.subject?.resolve(true)
                try store.storeDid(resolved!)
                XCTAssertNotNil(resolved)
                XCTAssertEqual(alias, try resolved!.getAlias())
                XCTAssertEqual(doc.subject, resolved!.subject)
                XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
                XCTAssertTrue(try resolved!.isValid())
            }
            var dids: Array<DID> = try store.listDids(DIDStore.DID_ALL)
            XCTAssertEqual(100, dids.count)
            
            dids = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(100, dids.count)
            
            dids = try store.listDids(DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, dids.count)
        } catch {
            XCTFail()
        }
    }
    
    func testDeleteDID() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            try testData.initIdentity()
            // Create test DIDs
            var dids: Array<DID> = []
            for i in 0..<100 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(storePass, alias)
                try store.publishDid(doc.subject!, storePass)
                dids.append(doc.subject!)
            }
            
            for i in 0..<100 {
                if (i % 5 != 0){
                    continue
                }
                
                let did: DID = dids[i]
                
                var deleted: Bool = try store.deleteDid(did)
                XCTAssertTrue(deleted)
                
                let path = storePath + "/ids/" + did.methodSpecificId
                XCTAssertFalse(testData.exists(path))
                
                deleted = try store.deleteDid(did)
                XCTAssertFalse(deleted)
            }
            var remains: Array<DID> = try store.listDids(DIDStore.DID_ALL)
            XCTAssertEqual(80, remains.count)
            
            remains = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(80, remains.count)
            
            remains = try store.listDids(DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, remains.count)
        } catch  {
            XCTFail()
        }
    }
    
    func testStoreAndLoadDID() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            let issuer: DIDDocument = try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
                        
            var doc: DIDDocument = try  store.loadDid(issuer.subject!)!
            XCTAssertEqual(issuer.subject, doc.subject)
            XCTAssertEqual(issuer.proof.signature, doc.proof.signature)
            XCTAssertTrue(try doc.isValid())
            
            doc = try store.loadDid(test.subject!.description)!
            XCTAssertEqual(test.subject, doc.subject)
            XCTAssertEqual(test.proof.signature, doc.proof.signature)
            XCTAssertTrue(try doc.isValid())
            
            var dids: Array<DID> = try store.listDids(DIDStore.DID_ALL)
            XCTAssertEqual(2, dids.count)
            
            dids = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(2, dids.count)
            
            dids = try store.listDids(DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, dids.count)
        }
        catch {
            XCTFail()
        }
    }
    
    func testLoadCredentials() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
                        
            var id: DIDURL = try DIDURL(test.subject!, "profile")
            vc = try store.loadCredential(test.subject!, id)
            XCTAssertNotNil(vc)
            XCTAssertEqual("MyProfile", try vc!.getAlias())
            XCTAssertEqual(test.subject, vc!.subject.id)
            XCTAssertEqual(id, vc!.id)
            XCTAssertTrue(try vc!.isValid())
            
            // try with full id string
            vc = try store.loadCredential(test.subject!.description, id.description)
            XCTAssertNotNil(vc)
            XCTAssertEqual("MyProfile", try vc!.getAlias())
            XCTAssertEqual(test.subject, vc!.subject.id)
            XCTAssertEqual(id, vc!.id)
            XCTAssertTrue(try vc!.isValid())
            
            id = try DIDURL(test.subject!, "twitter")
            vc = try store.loadCredential(test.subject!.description, "twitter")
            XCTAssertNotNil(vc)
            XCTAssertEqual("Twitter", try vc!.getAlias())
            XCTAssertEqual(test.subject, vc!.subject.id)
            XCTAssertEqual(id, vc!.id)
            XCTAssertTrue(try vc!.isValid())
            
            vc = try  store.loadCredential(test.subject!.description, "notExist")
            XCTAssertNil(vc)

            id = try DIDURL(test.subject!, "twitter")
            XCTAssertTrue(try store.containsCredential(test.subject!, id))
            XCTAssertTrue(try store.containsCredential(test.subject!.description, "twitter"))
            XCTAssertFalse(try store.containsCredential(test.subject!.description, "notExist"))
        }
        catch {
            XCTFail()
        }
    }
    
    func testListCredentials() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
            
            var vcs: Array<DIDURL> = try store.listCredentials(test.subject!)
            XCTAssertEqual(4, vcs.count)
            for id in vcs {
                var re = id.fragment == "profile" || id.fragment == "email" || id.fragment == "twitter" || id.fragment == "passport"
                XCTAssertTrue(re)
                
                re = id.getAlias() == "MyProfile" || id.getAlias() == "Email" || id.getAlias() == "Twitter" || id.getAlias() == "Passport"
                XCTAssertTrue(re)
            }
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testDeleteCredential() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            try testData.initIdentity()
            
            // Store test data into current store
            try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
            
            var path = storePath + "/ids/" + test.subject!.methodSpecificId + "/credentials/twitter/credential"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storePath + "/" + "ids" + "/" + test.subject!.methodSpecificId + "/" + "credentials" + "/" + "twitter" + "/" + ".meta"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storePath + "/" + "ids" + "/" + test.subject!.methodSpecificId + "/" + "credentials" + "/" + "passport" + "/" + "credential"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storePath + "/" + "ids" + "/" + test.subject!.methodSpecificId
                + "/" + "credentials" + "/" + "passport" + "/" + ".meta"
            XCTAssertTrue(testData.existsFile(path))
            
            var deleted: Bool = try store.deleteCredential(test.subject!, DIDURL(test.subject!, "twitter"))
            XCTAssertTrue(deleted)
            
            deleted = try store.deleteCredential(test.subject!.description, "passport")
            XCTAssertTrue(deleted)
            
            deleted = try store.deleteCredential(test.subject!.description, "notExist")
            XCTAssertFalse(deleted)
            
            path = storePath + "/" + "ids"
                + "/" + test.subject!.methodSpecificId
                + "/" + "credentials" + "/" + "twitter"
            XCTAssertFalse(testData.existsFile(path))
            
            path = storePath + "/" + "ids"
                + "/" + test.subject!.methodSpecificId
                + "/" + "credentials" + "/" + "passport"
            XCTAssertFalse(testData.existsFile(path))
            
            XCTAssertTrue(try store.containsCredential(test.subject!.description, "email"))
            XCTAssertTrue(try store.containsCredential(test.subject!.description, "profile"))
            
            XCTAssertFalse(try store.containsCredential(test.subject!.description, "twitter"))
            XCTAssertFalse(try store.containsCredential(test.subject!.description, "passport"))
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    // TODO:
    func testCompatibility() {
        let bundle = Bundle(for: type(of: self))
        let jsonPath: String = bundle.path(forResource: "teststore", ofType: "")!
        print(jsonPath)
        
        DIDBackend.creatInstance(DummyAdapter())
        let store = try! DIDStore.open("filesystem", jsonPath)
        
        let dids = try! store.listDids(DIDStore.DID_ALL)
        XCTAssertEqual(2, dids.count)
        
        for did in dids {
            if did.getAlias() == "Issuer" {
                let vcs: [DIDURL] = try! store.listCredentials(did)
                XCTAssertEqual(1, vcs.count)
                
                let id: DIDURL = vcs[0]
                XCTAssertEqual("Profile", id.getAlias())
                
                XCTAssertNotNil(try! store.loadCredential(did, id))
            } else if did.getAlias() == "Test" {
                let vcs: [DIDURL] = try! store.listCredentials(did)
                XCTAssertEqual(4, vcs.count)
                
                for id: DIDURL in vcs {
                    XCTAssertTrue(id.getAlias() == "Profile"
                    || id.getAlias() == "Email"
                    || id.getAlias() == "Passport"
                    || id.getAlias() == "Twitter")
                    
                    XCTAssertNotNil(try! store.loadCredential(did, id))
                }
            }
        }
    }
    
    // TODO:
    func testCompatibilityNewDIDWithWrongPass() {
        do {
            DIDBackend.creatInstance(DummyAdapter())
            let bundle = Bundle(for: type(of: self))
            let jsonPath = bundle.path(forResource: "teststore", ofType: "")
            let store = try! DIDStore.open("filesystem", jsonPath!)

            _ = try store.newDid("wrongpass");
        } catch {
            if error is DIDStoreError {
                let err = error as! DIDStoreError
                switch err {
                case  DIDStoreError.failue("decryptFromBase64 error."):
                    XCTAssertTrue(true)
                default:
                    XCTFail()
                }
            }
        }
    }
    
    // TODO:
    func testCompatibilityNewDID() {
        
        DIDBackend.creatInstance(DummyAdapter())
        let bundle = Bundle(for: type(of: self))
        let jsonPath = bundle.path(forResource: "teststore", ofType: "")
        let store = try! DIDStore.open("filesystem", jsonPath!)
        
        let doc: DIDDocument = try! store.newDid(storePass);
        XCTAssertNotNil(doc);
                
        _ = try! store.deleteDid(doc.subject!)
    }

    func createDataForPerformanceTest(_ store: DIDStore) {
        do {
            var props: OrderedDictionary<String, String> = OrderedDictionary()
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            for i in 0..<10 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(storePass, alias)
                
                let issuer: Issuer = try Issuer(doc)
                let vc: VerifiableCredential = try issuer.seal(for: doc.subject!, "cred-1", ["BasicProfileCredential", "SelfProclaimedCredential"], props, storePass)
                try store.storeCredential(vc)
            }
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testStorePerformance(_ cached: Bool) {
        do {
            let adapter: DIDAdapter = DummyAdapter()
            let testData: TestData = TestData()
            TestData.deleteFile(storePath)
            var store: DIDStore
            if (cached){
               store = try DIDStore.open("filesystem", storePath)
            }
            else {
               store = try DIDStore.open("filesystem", storePath, 0, 0)
            }
                        
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            
            createDataForPerformanceTest(store)
            let dids: Array<DID> = try store.listDids(DIDStore.DID_ALL)
            XCTAssertEqual(10, dids.count)
            // TODO: TimeMillis
            /*
             long start = System.currentTimeMillis()
             private void testStorePerformance(boolean cached) throws DIDException {
             
             for (int i = 0; i < 1000; i++) {
             for (DID did : dids) {
             DIDDocument doc = store.loadDid(did);
             assertEquals(did, doc.getSubject());
             
             DIDURL id = new DIDURL(did, "cred-1");
             VerifiableCredential vc = store.loadCredential(did, id);
             assertEquals(id, vc.getId());
             }
             }
             
             long end = System.currentTimeMillis();
             
             System.out.println("Store " + (cached ? "with " : "without ") +
             "cache took " + (end - start) + " milliseconds.");
             }
             */
            
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testStoreWithCache() {
        testStorePerformance(true)
    }
    
    func testStoreWithoutCache() {
        testStorePerformance(false)
    }
    
    func testMultipleStore() {
        
        do {
            
            var stores: Array = Array<DIDStore>()
            var docs: Array = Array<DIDDocument>()
            
            for i in 0..<10 {
                let path = storePath + String(i)
                TestData.deleteFile(path)
                let store: DIDStore = try DIDStore.open("filesystem", storePath + String(i))
                stores.append(store)
                let mnemonic: String = HDKey.generateMnemonic(0)
                try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            }
            
            for i in 0..<10 {
                let doc: DIDDocument = try stores[i].newDid(storePass)
                XCTAssertNotNil(doc)
                docs.append(doc)
            }
            
            for i in 0..<10 {
                let doc = try stores[i].loadDid(docs[i].subject!)
                XCTAssertNotNil(doc)
                XCTAssertEqual(try docs[i].toJson(nil, true, true),try doc!.toJson(nil, true, true))
            }
            
        } catch {
            print(error)
            XCTFail()
        }

    }
    
}

