
import XCTest
@testable import ElastosDIDSDK

class DIDStoreTests: XCTestCase {
    
    var store: DIDStore!
    static var ids: Dictionary<DID, String> = [: ]
    static var primaryDid: DID!
    var adapter: SPVAdaptor!
    
    func testCreateEmptyStore() {
        do {
            let testData: TestData = TestData()
            try _ = testData.setupStore(true)
            _ = testData.exists(storeRoot)
            
            let path = storeRoot + "/" + ".meta"
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
            _ = try store.newDid(withAlias: "this will be fail", using: storePass)
        } catch {
            print(error)
            XCTAssertTrue(true)
        }
    }

    func testInitPrivateIdentity0() {
        do {
            let testData: TestData = TestData()
            var store: DIDStore = try testData.setupStore(true)
            XCTAssertFalse(store.containsPrivateIdentity())
            
            _ = try testData.initIdentity()
            XCTAssertTrue(store.containsPrivateIdentity())
            var path = storeRoot + "/" + "private" + "/" + "key"
            XCTAssertTrue(testData.existsFile(path))
            path = storeRoot + "/" + "private" + "/" + "index"
            XCTAssertTrue(testData.existsFile(path))
            
            store = try DIDStore.open(atPath: storeRoot, withType: "filesystem", adapter: testData.adapter!)
            XCTAssertTrue(store.containsPrivateIdentity())
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testInitPrivateIdentityWithMnemonic() {
        do {
            let expectedIDString = "iY4Ghz9tCuWvB5rNwvn4ngWvthZMNzEA7U"
            let mnemonic = "cloth always junk crash fun exist stumble shift over benefit fun toe"

            let testData = TestData()
            var store = try testData.setupStore(true)
            XCTAssertFalse(store.containsPrivateIdentity())

            try store.initializePrivateIdentity(using: Mnemonic.ENGLISH, mnemonic: mnemonic, passPhrase: "", storePassword: storePass)
            XCTAssertTrue(store.containsPrivateIdentity())

            var path = storeRoot + "/" + "private" + "/" + "key"
            XCTAssertTrue(testData.existsFile(path))

            path = storeRoot + "/" + "private" + "/" + "index"
            XCTAssertTrue(testData.existsFile(path))

            path = storeRoot + "/" + "private" + "/" + "mnemonic"
            XCTAssertTrue(testData.existsFile(path))

            store = try DIDStore.open(atPath: storeRoot, withType: "filesystem", adapter: testData.adapter!)
            XCTAssertTrue(store.containsPrivateIdentity())
            let exportedMnemonic = try store.exportMnemonic(using: storePass)
            XCTAssertEqual(mnemonic, exportedMnemonic)

            let doc = try store.newDid(using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(expectedIDString, doc.subject.methodSpecificId)
        } catch {
            XCTFail()
        }
    }

    func testInitPrivateIdentityWithRootKey() {
        do {
            let expectedIDString = "iYbPqEA98rwvDyA5YT6a3mu8UZy87DLEMR";
            let rootKey = "xprv9s21ZrQH143K4biiQbUq8369meTb1R8KnstYFAKtfwk3vF8uvFd1EC2s49bMQsbdbmdJxUWRkuC48CXPutFfynYFVGnoeq8LJZhfd9QjvUt";

            let testData = TestData()
            var store = try testData.setupStore(true)
            XCTAssertFalse(store.containsPrivateIdentity())

            try store.initializePrivateIdentity(using: rootKey, storePassword: storePass)
            XCTAssertTrue(store.containsPrivateIdentity())

            var path = storeRoot + "/" + "private" + "/" + "key"
            XCTAssertTrue(testData.existsFile(path))

            path = storeRoot + "/" + "private" + "/" + "index"
            XCTAssertTrue(testData.existsFile(path))

            path = storeRoot + "/" + "private" + "/" + "mnemonic"
            XCTAssertFalse(testData.existsFile(path))

            store = try DIDStore.open(atPath: storeRoot, withType: "filesystem", adapter: testData.adapter!)
            XCTAssertTrue(store.containsPrivateIdentity())

            let doc = try store.newDid(using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(expectedIDString, doc.subject.methodSpecificId)
        } catch {
            XCTFail()
        }
    }

    func testCreateDIDWithAlias() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let alias: String = "my first did"
            
            let doc: DIDDocument = try store.newDid(withAlias: alias, using: storePass)
            XCTAssertTrue(doc.isValid)
            
            var resolved = try? doc.subject.resolve()
            XCTAssertNil(resolved)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            var path = ""
            
            path = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/document"
            XCTAssertTrue(testData.existsFile(path))
            path = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/.meta"
            XCTAssertTrue(testData.existsFile(path))
            
            resolved = try doc.subject.resolve(true)
            
            XCTAssertNotNil(resolved)

            try store.storeDid(using: resolved!)
            XCTAssertEqual(alias, resolved!.aliasName)
            XCTAssertEqual(doc.subject, resolved!.subject)
            XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
            
            XCTAssertTrue(resolved!.isValid)
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testCreateDIDWithoutAlias() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc: DIDDocument = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)
            
            var resolved = try? doc.subject.resolve(true)
            XCTAssertNil(resolved)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            let path = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/document"
            XCTAssertTrue(testData.existsFile(path))

            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.subject, resolved!.subject)
            XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
            XCTAssertTrue(resolved!.isValid)
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testCreateDIDByIndex() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            let alias = "my first did"
            let did = try store.getDid(byPrivateIdentityIndex: 0)
            var doc = try store.newDid(withPrivateIdentityIndex: 0, alias: alias, using: storePass)
            XCTAssertTrue(doc.isValid)
            XCTAssertEqual(did, doc.subject)

            XCTAssertThrowsError(try store.newDid(withAlias: alias, using: storePass)){ (error) in
                switch error {
                case DIDError.didStoreError("DID already exists."): break
                //everything is fine
                default:
                XCTFail("Unexpected error thrown")
                }
            }

            let success = store.deleteDid(did)
            XCTAssertTrue(success)
            doc = try store.newDid(withAlias: alias, using: storePass)
            XCTAssertTrue(doc.isValid)
            XCTAssertEqual(did, doc.subject)
        } catch {
            XCTFail()
        }
    }

    func testGetDid() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            for i in 0...100 {
                let alias = "did#\(i)"
                let doc = try store.newDid(withPrivateIdentityIndex: i, alias: alias, using: storePass)
                XCTAssertTrue(doc.isValid)
                let did = try store.getDid(byPrivateIdentityIndex: i)
                XCTAssertEqual(doc.subject, did)
            }
        } catch {
            XCTFail()
        }
    }

    func testUpdateDid() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc: DIDDocument = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)
            _ = try store.publishDid(for: doc.subject, using: storePass)
            
            var resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            try store.storeDid(using: resolved)
            
            // Update
            var db: DIDDocumentBuilder = resolved.editing()
            var key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            var newDoc = try db.sealed(using: storePass)
            XCTAssertEqual(2, newDoc.publicKeyCount)
            XCTAssertEqual(2, newDoc.authenticationKeyCount)
            try store.storeDid(using: newDoc)
            
            _ = try store.publishDid(for: newDoc.subject, using: storePass)
            
            resolved = try doc.subject.resolve(true)

            XCTAssertNotNil(resolved)
            XCTAssertEqual(newDoc.description, resolved.description)
            try store.storeDid(using: resolved)

            // Update again
            db = resolved.editing()
            key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key2", keyBase58: key.getPublicKeyBase58())
            newDoc = try db.sealed(using: storePass)
            XCTAssertEqual(3, newDoc.publicKeyCount)
            XCTAssertEqual(3, newDoc.authenticationKeyCount)
            try store.storeDid(using: newDoc)
            _ = try store.publishDid(for: newDoc.subject, using: storePass)
            
            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(newDoc.description, resolved.description)
        } catch {
            XCTFail()
        }
    }
    

    func testUpdateDidWithoutTxid() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            var resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setTransactionId(nil)
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            var db = doc.editing()
            var key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setTransactionId(nil)
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update again
            db = doc.editing()
            key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key2", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(3, doc.publicKeyCount)
            XCTAssertEqual(3, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            _ = try store.publishDid(for: doc.subject, using: storePass)
            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())
        } catch {
            XCTFail()
        }
    }

    func testUpdateDidWithoutSignature() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            var resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

//            doc.getMeta().setSignature(nil) // seeting Signature withour storepass
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            var db = doc.editing()
            var key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

//            doc.getMeta().setSignature(nil)
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update again
            db = doc.editing()
            key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key2", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(3, doc.publicKeyCount)
            XCTAssertEqual(3, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())
        } catch {
            XCTFail()
        }
    }

    func testUpdateDidWithoutTxidAndSignature() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            let resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setTransactionId(nil)
//            doc.getMeta().setSignature(nil)
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            let db = doc.editing()
            let key = try TestData.generateKeypair()
            _ =  try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)
            let did = doc.subject
            try store.publishDid(for: did, using: storePass)
            // java reference
//            XCTAssertThrowsError(try store.publishDid(for: did, using: storePass)) { error in
//                switch error as! DIDError{
//                case .didStoreError("DID document not up-to-date"):
//                    XCTAssertTrue(true)
//                default:
//                    XCTFail()
//                }
//            }
        } catch {
            XCTFail()
        }
    }

    func testForceUpdateDidWithoutTxidAndSignature() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            var resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setTransactionId(nil)
//            doc.getMeta().setSignature(nil)
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            let db = doc.editing()
            let key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            _ = try store.publishDid(for: doc.subject, waitForConfirms: 1, using: doc.defaultPublicKey, storePassword: storePass, true)

            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

        } catch {
            XCTFail()
        }
    }

    func testUpdateDidWithWrongTxid() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            let resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setTransactionId("1234567890")
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            let db = doc.editing()
            let key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            let did = doc.subject

            XCTAssertThrowsError(try store.publishDid(for: did, using: storePass)) { error in
                switch error as! DIDError {
                case .didStoreError("DID document not up-to-date"):
                    XCTAssertTrue(true)
                default:
                    XCTFail()
                }
            }
        } catch {
            XCTFail()
        }
    }

    func testUpdateDidWithWrongSignature() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            let resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setSignature("1234567890")
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            let db = doc.editing()
            let key = try TestData.generateKeypair()
             _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            let did = doc.subject
            XCTAssertThrowsError(try store.publishDid(for: did, using: storePass)) { error in
                switch error as! DIDError{
                case .didStoreError("DID document not up-to-date"):
                    XCTAssertTrue(true)
                default:
                    XCTFail()
                }
            }
        } catch {
            XCTFail()
        }
    }

    func testForceUpdateDidWithWrongTxid() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            var resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setTransactionId("1234567890")
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            let db = doc.editing()
            let key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            _ = try store.publishDid(for: doc.subject, waitForConfirms: 1, using: doc.defaultPublicKey, storePassword: storePass, true)
            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())
        } catch {
            XCTFail()
        }
    }

    func testForceUpdateDidWithWrongSignature() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()

            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)

            _ = try store.publishDid(for: doc.subject, using: storePass)

            var resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            doc.getMeta().setSignature("1234567890")
            try store.storeDidMeta(doc.getMeta(), for: doc.subject)

            // Update
            let db = doc.editing()
            let key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            _ = try store.publishDid(for: doc.subject, waitForConfirms: 1, using: doc.defaultPublicKey, storePassword: storePass, true)

            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())
        } catch {
            XCTFail()
        }
    }
    
    func testDeactivateSelfAfterCreate() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            let resolved: DIDDocument = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            
            _ = try store.deactivateDid(for: doc.subject, using: storePass)
            
            let resolvedNil = try doc.subject.resolve(true)
            
            XCTAssertNil(resolvedNil)
        } catch  {
            switch error as! DIDError{
            case .didDeactivated(nil):
                XCTAssertTrue(true)
            default:
                XCTFail()
            }
        }
    }
    
    func testDeactivateSelfAfterUpdate() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            
            var resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())

            // update
            let db = doc.editing()
            let key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            
            resolved = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())
            _ = try store.deactivateDid(for: doc.subject, using: storePass)
            let did = doc.subject

            XCTAssertThrowsError(try did.resolve(true)) { (error) in
                switch error {
                case DIDError.didDeactivated: break
                //everything is fine
                default:
                    XCTFail("Unexpected error thrown")
                }
            }
        } catch  {
            XCTFail()
        }
    }
    
    func testDeactivateWithAuthorization1() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            let doc = try store.newDid(using: storePass)
            XCTAssertTrue(doc.isValid)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            
            var resolved: DIDDocument = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())
            
            var target = try store.newDid(using: storePass)
            let db: DIDDocumentBuilder = target.editing()
            _ = try db.authorizationDid(with: "recovery", controller: doc.subject.toString())
            target = try db.sealed(using: storePass)
            XCTAssertNotNil(target)
            XCTAssertEqual(1, target.authorizationKeyCount)
            XCTAssertEqual(doc.subject, target.authorizationKeys()[0].controller)

            try store.storeDid(using: target)
            _ = try store.publishDid(for: target.subject, using: storePass)
            resolved = try target.subject.resolve()
            XCTAssertNotNil(resolved)
            XCTAssertEqual(target.toString(), resolved.toString())
            _ = try store.deactivateDid(for: target.subject, withAuthroizationDid: doc.subject, storePassword: storePass)
            let did = target.subject

            XCTAssertThrowsError(try did.resolve(true)) { (error) in
                switch error {
                case DIDError.didDeactivated: break
                //everything is fine
                default: //TODO:
                    XCTFail("Unexpected error thrown")
                }
            }
        } catch  {
            XCTFail()
        }
    }
    
    func testDeactivateWithAuthorization2() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc = try store.newDid(using: storePass)
            var db: DIDDocumentBuilder = doc.editing()
            let key = try TestData.generateKeypair()
            let id = try DIDURL(doc.subject, "key-2")
            _ = try db.appendAuthenticationKey(with: id, keyBase58: key.getPublicKeyBase58())
            try store.storePrivateKey(for: doc.subject, id: id, privateKey: key.getPrivateKeyData(), using: storePass)
            doc = try db.sealed(using: storePass)
            XCTAssertTrue(doc.isValid)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            var resolved: DIDDocument = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), doc.toString())
            
            var target: DIDDocument = try store.newDid(using: storePass)
            db = target.editing()
            _ = try db.appendAuthorizationKey(with: "recovery", controller: doc.subject.toString(), keyBase58: key.getPublicKeyBase58())
            target = try db.sealed(using: storePass)
            XCTAssertNotNil(target)
            XCTAssertEqual(1, target.authorizationKeyCount)
            let controller = target.authorizationKeys()[0].controller
            XCTAssertEqual(doc.subject, controller)
            try store.storeDid(using: target)
            
            _ = try store.publishDid(for: target.subject, using: storePass)
            
            resolved = try target.subject.resolve()
            XCTAssertNotNil(resolved)
            XCTAssertEqual(target.toString(), resolved.toString())
            
            _ = try store.deactivateDid(for: target.subject, withAuthroizationDid: doc.subject, using: id, storePassword: storePass)
            let did = target.subject

            XCTAssertThrowsError(try did.resolve(true)) { (error) in
                switch error {
                case DIDError.didDeactivated: break
                //everything is fine
                default: break //TODO:
                XCTFail("Unexpected error thrown")
                }
            }
        } catch  {
            XCTFail()
        }
    }
    
    func testDeactivateWithAuthorization3() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            var doc = try store.newDid(using: storePass)
            var db: DIDDocumentBuilder = doc.editing()
            
            let key = try TestData.generateKeypair()
            let id: DIDURL = try DIDURL(doc.subject, "key-2")
            _ = try db.appendAuthenticationKey(with: id, keyBase58: key.getPublicKeyBase58())
            
            try store.storePrivateKey(for: doc.subject, id: id, privateKey: key.getPrivateKeyData(), using: storePass)
            doc = try db.sealed(using: storePass)
            XCTAssertTrue(doc.isValid)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)
            
            _ = try store.publishDid(for: doc.subject, using: storePass)
            
            var resolved: DIDDocument = try doc.subject.resolve(true)
            XCTAssertNotNil(resolved)
            XCTAssertEqual(doc.toString(), resolved.toString())
            
            var target = try store.newDid(using: storePass)
            db = target.editing()
            _ = try db.appendAuthorizationKey(with: "recovery", controller: doc.subject.toString(), keyBase58: key.getPublicKeyBase58())
            target = try db.sealed(using: storePass)
            XCTAssertNotNil(target)
            XCTAssertEqual(1, target.authorizationKeyCount)
            let controller = target.authorizationKeys()[0].controller
            XCTAssertEqual(doc.subject, controller)
            try store.storeDid(using: target)
            
            _ = try store.publishDid(for: target.subject, using: storePass)
            
            resolved = try target.subject.resolve()
            XCTAssertNotNil(resolved)
            XCTAssertEqual(target.toString(), resolved.toString())
            
            _ = try store.deactivateDid(for: target.subject, withAuthroizationDid: doc.subject, storePassword: storePass)
            let did = target.subject
            XCTAssertThrowsError(try did.resolve(true)) { (error) in
                switch error {
                case DIDError.didDeactivated: break
                //everything is fine
                default: break //TODO:
                XCTFail("Unexpected error thrown")
                }
            }
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
                let doc: DIDDocument = try! store.newDid(withAlias: alias, using: storePass )
                XCTAssertTrue(doc.isValid)
                
                var resolved = try? doc.subject.resolve(true)
                XCTAssertNil(resolved)
                
                _ = try store.publishDid(for: doc.subject, using: storePass)
                
                var path = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/document"
                XCTAssertTrue(testData.existsFile(path))
                
                path = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/.meta"
                XCTAssertTrue(testData.existsFile(path))
                
                resolved = try doc.subject.resolve(true)
                try store.storeDid(using: resolved!)
                XCTAssertNotNil(resolved)
                XCTAssertEqual(alias, resolved!.aliasName)
                XCTAssertEqual(doc.subject, resolved!.subject)
                XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
                XCTAssertTrue(resolved!.isValid)
            }
            var dids: Array<DID> = try store.listDids(using: DIDStore.DID_ALL)
            XCTAssertEqual(100, dids.count)
            
            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(100, dids.count)
            
            dids = try store.listDids(using: DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, dids.count)
        } catch {
            XCTFail()
        }
    }
    
    func testDeleteDID() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            // Create test DIDs
            var dids: Array<DID> = []
            for i in 0..<100 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try! store.newDid(withAlias: alias, using: storePass)
                _ =  try! store.publishDid(for: doc.subject, using: storePass)
                dids.append(doc.subject)
            }
            
            for i in 0..<100 {
                if (i % 5 != 0){
                    continue
                }
                
                let did: DID = dids[i]
                
                var deleted: Bool = store.deleteDid(did)
                XCTAssertTrue(deleted)
                
                let path = storeRoot + "/ids/" + did.methodSpecificId
                XCTAssertFalse(testData.exists(path))
                
                deleted = store.deleteDid(did)
                XCTAssertFalse(deleted)
            }
            var remains: Array<DID> = try! store.listDids(using: DIDStore.DID_ALL)
            XCTAssertEqual(80, remains.count)
            
            remains = try! store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(80, remains.count)
            
            remains = try! store.listDids(using: DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, remains.count)
        } catch  {
            XCTFail()
        }
    }
    
    func testStoreAndLoadDID() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            // Store test data into current store
            let issuer: DIDDocument = try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
                        
            var doc: DIDDocument = try  store.loadDid(issuer.subject)
            XCTAssertEqual(issuer.subject, doc.subject)
            XCTAssertEqual(issuer.proof.signature, doc.proof.signature)
            XCTAssertTrue(doc.isValid)
            
            doc = try store.loadDid(test.subject.description)
            XCTAssertEqual(test.subject, doc.subject)
            XCTAssertEqual(test.proof.signature, doc.proof.signature)
            XCTAssertTrue(doc.isValid)
            
            var dids: Array<DID> = try store.listDids(using: DIDStore.DID_ALL)
            XCTAssertEqual(2, dids.count)
            
            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(2, dids.count)
            
            dids = try store.listDids(using: DIDStore.DID_NO_PRIVATEKEY)
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
            _ = try testData.initIdentity()
            
            // Store test data into current store
            _ = try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
                        
            var id: DIDURL = try DIDURL(test.subject, "profile")
            vc = try store.loadCredential(for: test.subject, byId: id)
            XCTAssertNotNil(vc)
            XCTAssertEqual("MyProfile", vc!.aliasName)
            XCTAssertEqual(test.subject, vc!.subject.did)
            XCTAssertEqual(id, vc!.getId())
            XCTAssertTrue(vc!.isValid)
            
            // try with full id string
            vc = try store.loadCredential(for: test.subject.description, byId: id.description)
            XCTAssertNotNil(vc)
            XCTAssertEqual("MyProfile", vc!.aliasName)
            XCTAssertEqual(test.subject, vc!.subject.did)
            XCTAssertEqual(id, vc!.getId())
            XCTAssertTrue(vc!.isValid)
            
            id = try DIDURL(test.subject, "twitter")
            vc = try store.loadCredential(for: test.subject.description, byId: "twitter")
            XCTAssertNotNil(vc)
            XCTAssertEqual("Twitter", vc!.aliasName)
            XCTAssertEqual(test.subject, vc!.subject.did)
            XCTAssertEqual(id, vc!.getId())
            XCTAssertTrue(vc!.isValid)
            
            vc = try store.loadCredential(for: test.subject.description, byId: "notExist")
            XCTAssertNil(vc)

            id = try DIDURL(test.subject, "twitter")
            XCTAssertTrue(try store.containsCredential(test.subject, id))
            XCTAssertTrue(try store.containsCredential(test.subject.description, "twitter"))
            XCTAssertFalse(try store.containsCredential(test.subject.description, "notExist"))
        }
        catch {
            XCTFail()
        }
    }
    
    func testListCredentials() {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            // Store test data into current store
            _ = try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
            
            let vcs: Array<DIDURL> = try store.listCredentials(for: test.subject)
            XCTAssertEqual(4, vcs.count)
            for id in vcs {
                var re = id.fragment == "profile" || id.fragment == "email" || id.fragment == "twitter" || id.fragment == "passport"
                XCTAssertTrue(re)
                
                re = id.aliasName == "MyProfile" || id.aliasName == "Email" || id.aliasName == "Twitter" || id.aliasName == "Passport"
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
            _ = try testData.initIdentity()
            
            // Store test data into current store
            _ = try testData.loadTestIssuer()
            let test: DIDDocument = try testData.loadTestDocument()
            var vc = try testData.loadProfileCredential()
            try vc!.setAlias("MyProfile")
            vc = try testData.loadEmailCredential()
            try vc!.setAlias("Email")
            vc = try testData.loadTwitterCredential()
            try vc!.setAlias("Twitter")
            vc = try testData.loadPassportCredential()
            try vc!.setAlias("Passport")
            
            var path = storeRoot + "/ids/" + test.subject.methodSpecificId + "/credentials/twitter/credential"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storeRoot + "/" + "ids" + "/" + test.subject.methodSpecificId + "/" + "credentials" + "/" + "twitter" + "/" + ".meta"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storeRoot + "/" + "ids" + "/" + test.subject.methodSpecificId + "/" + "credentials" + "/" + "passport" + "/" + "credential"
            XCTAssertTrue(testData.existsFile(path))
            
            path = storeRoot + "/" + "ids" + "/" + test.subject.methodSpecificId
                + "/" + "credentials" + "/" + "passport" + "/" + ".meta"
            XCTAssertTrue(testData.existsFile(path))
            
            var deleted: Bool = store.deleteCredential(for: test.subject, id: try DIDURL(test.subject, "twitter"))
            XCTAssertTrue(deleted)
            
            deleted = store.deleteCredential(for: test.subject.description, id: "passport")
            XCTAssertTrue(deleted)
            
            deleted = store.deleteCredential(for: test.subject.description, id: "notExist")
            XCTAssertFalse(deleted)
            
            path = storeRoot + "/" + "ids"
                + "/" + test.subject.methodSpecificId
                + "/" + "credentials" + "/" + "twitter"
            XCTAssertFalse(testData.existsFile(path))
            
            path = storeRoot + "/" + "ids"
                + "/" + test.subject.methodSpecificId
                + "/" + "credentials" + "/" + "passport"
            XCTAssertFalse(testData.existsFile(path))
            
            XCTAssertTrue(try store.containsCredential(test.subject.description, "email"))
            XCTAssertTrue(try store.containsCredential(test.subject.description, "profile"))
            
            XCTAssertFalse(try store.containsCredential(test.subject.description, "twitter"))
            XCTAssertFalse(try store.containsCredential(test.subject.description, "passport"))
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testCompatibility() throws {
        let bundle = Bundle(for: type(of: self))
        let jsonPath: String = bundle.path(forResource: "teststore", ofType: "")!
        print(jsonPath)
        
        let adapter = DummyAdapter()
        try DIDBackend.initializeInstance(resolver, TestData.getResolverCacheDir())
        let store = try DIDStore.open(atPath: jsonPath, withType: "filesystem", adapter: adapter)
        
        let dids = try store.listDids(using: DIDStore.DID_ALL)
        XCTAssertEqual(2, dids.count)
        
        for did in dids {
            if did.aliasName == "Issuer" {
                let vcs: [DIDURL] = try store.listCredentials(for: did)
                XCTAssertEqual(1, vcs.count)
                
                let id: DIDURL = vcs[0]
                XCTAssertEqual("Profile", id.aliasName)
                
                XCTAssertNotNil(try store.loadCredential(for: did, byId: id))
            } else if did.aliasName == "Test" {
                let vcs: [DIDURL] = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)
                
                for id: DIDURL in vcs {
                    XCTAssertTrue(id.aliasName == "Profile"
                    || id.aliasName == "Email"
                    || id.aliasName == "Passport"
                    || id.aliasName == "Twitter")
                    
                    XCTAssertNotNil(try store.loadCredential(for: did, byId: id))
                }
            }
        }
    }

    func testCompatibilityNewDIDandGetDID() {
        do {
            let bundle = Bundle(for: type(of: self))
            let jsonPath: String = bundle.path(forResource: "teststore", ofType: "")!

            let adapter = DummyAdapter()
            try DIDBackend.initializeInstance(adapter, TestData.getResolverCacheDir())
            let store = try DIDStore.open(atPath: jsonPath, withType: "filesystem", adapter: adapter)

            var doc = try store.newDid(using: storePass)
            XCTAssertNotNil(doc)

            _ = store.deleteDid(doc.subject)

            let did = try store.getDid(byPrivateIdentityIndex: 1000)

            doc = try store.newDid(withPrivateIdentityIndex: 1000, using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(doc.subject, did)

            _ = store.deleteDid(doc.subject)
        } catch {
            XCTFail()
        }
    }
    
    func testCompatibilityNewDIDWithWrongPass() {
        do {
            try DIDBackend.initializeInstance(resolver, TestData.getResolverCacheDir())
            let bundle = Bundle(for: type(of: self))
            let jsonPath = bundle.path(forResource: "teststore", ofType: "")
            let store = try DIDStore.open(atPath: jsonPath!, withType: "filesystem", adapter: DummyAdapter())

            _ = try store.newDid(using: "wrongpass")
        } catch {
            if error is DIDError {
                let err = error as! DIDError
                switch err {
                case .didStoreError(_desc: "decryptFromBase64 error."):
                    XCTAssertTrue(true)
                default:
                    XCTFail()
                }
            }
        }
    }
    
    func testCompatibilityNewDID() throws {
        try DIDBackend.initializeInstance(resolver, TestData.getResolverCacheDir())
        let bundle = Bundle(for: type(of: self))
        let jsonPath = bundle.path(forResource: "teststore", ofType: "")
        let store = try DIDStore.open(atPath: jsonPath!, withType: "filesystem", adapter: DummyAdapter())
        
        let doc: DIDDocument = try store.newDid(using: storePass)
        XCTAssertNotNil(doc)
                
        _ = store.deleteDid(doc.subject)
    }

    func createDataForPerformanceTest(_ store: DIDStore) {
        do {
            var props: Dictionary<String, String> = [: ]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"
            
            for i in 0..<10 {
                let alias: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(withAlias: alias, using: storePass)
                
                let issuer = try VerifiableCredentialIssuer(doc)
                let cb = issuer.editingVerifiableCredentialFor(did: doc.subject)
                let vc: VerifiableCredential = try cb.withId("cred-1")
                    .withTypes("BasicProfileCredential", "InternetAccountCredential")
                    .withProperties(props)
                    .sealed(using: storePass)
                try store.storeCredential(using: vc)
            }
        } catch {
            print(error)
            XCTFail()
        }
    }
    
    func testStorePerformance(_ cached: Bool) {
        do {
            let adapter = DummyAdapter()
            _ = TestData()
            TestData.deleteFile(storeRoot)
            var store: DIDStore
            if (cached){
                store = try DIDStore.open(atPath: storeRoot, withType: "filesystem", adapter: adapter)
            }
            else {
                store = try DIDStore.open(atPath: storeRoot, withType: "filesystem", initialCacheCapacity: 0, maxCacheCapacity: 0, adapter: adapter)
            }
                        
            let mnemonic: String = try Mnemonic.generate("0")
            try store.initializePrivateIdentity(using: "0", mnemonic: mnemonic, passPhrase: passphrase, storePassword: storePass)
            
            createDataForPerformanceTest(store)
            let dids: Array<DID> = try store.listDids(using: DIDStore.DID_ALL)
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
                let path = storeRoot + String(i)
                TestData.deleteFile(path)
                let store = try DIDStore.open(atPath: storeRoot + String(i), withType: "filesystem", adapter: DummyAdapter())
                stores.append(store)
                let mnemonic: String = try Mnemonic.generate("0")
                try store.initializePrivateIdentity(using: "0", mnemonic: mnemonic, passPhrase: passphrase, storePassword: storePass, true)
            }
            
            for i in 0..<10 {
                let doc: DIDDocument = try stores[i].newDid(using: storePass)
                XCTAssertNotNil(doc)
                docs.append(doc)
            }
            
            for i in 0..<10 {
                let doc = try stores[i].loadDid(docs[i].subject)
                XCTAssertNotNil(doc)
                XCTAssertEqual(docs[i].toString(true, forSign: true), doc.toString(true, forSign: true))
            }
        } catch {
            print(error)
            XCTFail()
        }

    }
    
    func testChangePassword() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            
            for i in 0..<10 {
                let alias: String = "my did \(i)"
                let doc = try store.newDid(withAlias: alias, using: storePass)
                XCTAssertTrue(doc.isValid)
                var resolved = try? doc.subject.resolve(true)
                XCTAssertNil(resolved)
                _ = try store.publishDid(for: doc.subject, using: storePass)
                var path: String = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/document"
                XCTAssertTrue(testData.existsFile(path))
                
                path = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/.meta"
                XCTAssertTrue(testData.existsFile(path))
                resolved = try doc.subject.resolve(true)
                XCTAssertNotNil(resolved)
                try store.storeDid(using: resolved!)
                XCTAssertEqual(alias, resolved!.aliasName)
                XCTAssertEqual(doc.subject, resolved!.subject)
                XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
                XCTAssertTrue(resolved!.isValid)
            }
            var dids = try store.listDids(using: DIDStore.DID_ALL)
            XCTAssertEqual(10, dids.count)

            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY);
            XCTAssertEqual(10, dids.count)

            dids = try store.listDids(using: DIDStore.DID_NO_PRIVATEKEY);
            XCTAssertEqual(0, dids.count)

            try store.changePassword(storePass, "newpasswd")

            dids = try store.listDids(using: DIDStore.DID_ALL)
            XCTAssertEqual(10, dids.count)

            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(10, dids.count)

            dids = try store.listDids(using: DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, dids.count)

            let doc = try store.newDid(using: "newpasswd")
            XCTAssertNotNil(doc)
        } catch {
            print(error)
            XCTFail()
        }
    }

    func testChangePasswordWithWrongPassword() {
        do {
            let testData: TestData = TestData()
            let store = try testData.setupStore(true)
            _ = try testData.initIdentity()
            for i in 0..<10 {
                let alias = "my did \(i)"
                let doc = try store.newDid(withAlias: alias, using: storePass)
                XCTAssertTrue(doc.isValid)
                var resolved = try? doc.subject.resolve(true)
                XCTAssertNil(resolved)
                _ = try store.publishDid(for: doc.subject, using: storePass)
                var path: String = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/document"
                XCTAssertTrue(testData.existsFile(path))
                
                path = storeRoot + "/ids/" + doc.subject.methodSpecificId + "/.meta"
                XCTAssertTrue(testData.existsFile(path))
                resolved = try doc.subject.resolve(true)
                XCTAssertNotNil(resolved)
                try store.storeDid(using: resolved!)
                XCTAssertEqual(alias, resolved!.aliasName)
                XCTAssertEqual(doc.subject, resolved!.subject)
                XCTAssertEqual(doc.proof.signature, resolved!.proof.signature)
                XCTAssertTrue(resolved!.isValid)
            }

            var dids = try store.listDids(using: DIDStore.DID_ALL)
            XCTAssertEqual(10, dids.count)

            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(10, dids.count)

            dids = try store.listDids(using: DIDStore.DID_NO_PRIVATEKEY)
            XCTAssertEqual(0, dids.count)

            XCTAssertThrowsError(try store.changePassword("wrongpasswd", "newpasswd")) { error in
                switch error as! DIDError{
                case .didStoreError("Change store password failed."):
                    XCTAssertTrue(true)
                default:
                    XCTFail()
                }
            }
        } catch {
            XCTFail()
        }
    }

    func testExportAndImportDid() {
        do {
            let testData: TestData = TestData()
            let bundle = Bundle(for: type(of: self))
            let jsonPath: String = bundle.path(forResource: "teststore", ofType: "")!

            let adapter = DummyAdapter()
            try DIDBackend.initializeInstance(adapter, TestData.getResolverCacheDir())
            let store = try DIDStore.open(atPath: jsonPath, withType: "filesystem", adapter: adapter)

            let did = try store.listDids(using: DIDStore.DID_ALL)[0]

            let exportPath = tempDir + "/" + "didexport.json"
            try create(exportPath, forWrite: true)
            let fileHndle: FileHandle = FileHandle(forWritingAtPath: exportPath)!

            try store.exportDid(did, to: fileHndle, using: "password", storePassword: storePass)
            let restorePath = tempDir + "/" + "restore"
            TestData.deleteFile(restorePath)

            let store2 = try DIDStore.open(atPath: restorePath, withType: "filesystem", adapter: adapter)

            let readerHndle = FileHandle(forReadingAtPath: exportPath)
            readerHndle?.seek(toFileOffset: 0)
            try store2.importStore(from: readerHndle!, "password", storePass)

//            let didDirPath = storeRoot + "/ids/" + did.methodSpecificId
            let reDidDirPath = restorePath + "/ids/" + did.methodSpecificId

//            XCTAssertTrue(testData.existsFile(didDirPath))
            XCTAssertTrue(testData.exists(reDidDirPath))
//            XCTAssertTrue(Utils.equals(reDidDir, didDir))
            
        } catch {
            XCTFail()
        }
    }

    func testExportAndImportPrivateIdentity() {
        do {
            let testData: TestData = TestData()
            let bundle = Bundle(for: type(of: self))
            let jsonPath: String = bundle.path(forResource: "teststore", ofType: "")!

            let adapter = DummyAdapter()
            try DIDBackend.initializeInstance(adapter, TestData.getResolverCacheDir())
            let store = try DIDStore.open(atPath: jsonPath, withType: "filesystem", adapter: adapter)

            let exportPath = tempDir + "/" + "didexport2.json"
            try create(exportPath, forWrite: true)
            let fileHndle: FileHandle = FileHandle(forWritingAtPath: exportPath)!

            try store.exportPrivateIdentity(to: fileHndle, "password", storePass)
            let restorePath = tempDir + "/" + "restore"
            TestData.deleteFile(restorePath)

            let store2 = try DIDStore.open(atPath: restorePath, withType: "filesystem", adapter: adapter)

            let readerHndle = FileHandle(forReadingAtPath: exportPath)
            readerHndle?.seek(toFileOffset: 0)
            try store2.importPrivateIdentity(from: readerHndle!, using: "password", storePassword: storePass)

            //            let didDirPath = storeRoot + "/private"
            let reDidDirPath = restorePath + "/private"
            XCTAssertTrue(testData.exists(reDidDirPath))
        } catch {
            XCTFail()
        }
    }

    func create(_ path: String, forWrite: Bool) throws {

        if !FileManager.default.fileExists(atPath: path) && forWrite {
            let dirPath: String = PathExtracter(path).dirname()
            let fileM = FileManager.default
            let re = fileM.fileExists(atPath: dirPath)
            if !re {
                try fileM.createDirectory(atPath: dirPath, withIntermediateDirectories: true, attributes: nil)
            }
            FileManager.default.createFile(atPath: path, contents: nil, attributes: nil)
        }
    }
}

