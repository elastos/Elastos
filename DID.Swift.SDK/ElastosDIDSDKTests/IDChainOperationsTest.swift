
import XCTest
import ElastosDIDSDK

class IDChainOperationsTest: XCTestCase {

    public static let DUMMY_TEST = false

    public func testPublishAndResolve() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(false)
            _ = try testData.initIdentity()
            var adapter: SPVAdaptor? = nil
            adapter = (DIDBackend.shareInstance()!.didAdapter as? SPVAdaptor)
            
            if adapter != nil {
                while true {
                    if try adapter!.isAvailable() {
                        print("OK")
                        break
                    }
                    else {
                        print("...")
                    }
                    wait(interval: 30)
                }
            }

            // Create new DID and publish to ID sidechain.
            let doc = try store.newDid(storePass)
            let did = doc.subject
            let txid = try store.publishDid(did!, storePass)
            XCTAssertNotNil(txid)
            print("Published new DID: \(doc.subject!)")
            
            // Resolve new DID document
            if adapter != nil {
                print("Try to resolve new published DID.")
                print(Date())
                while true {
                    wait(interval: 30)
                    let rdoc = try did!.resolve(true)
                    print(Date())
                    if rdoc != nil {
                        print("OK")
                        break
                    }
                    else {
                        print("...")
                    }
                }
            }
            let resolved = try did!.resolve(true)
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(try resolved!.isValid())
            XCTAssertEqual(doc.description(true), resolved?.description(true))
        } catch {
            XCTFail()
        }
    }
    
    func testUpdateAndResolve() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            _ = try testData.initIdentity()
            var adapter: SPVAdaptor? = nil
            //             need synchronize?
            adapter = (DIDBackend.shareInstance()!.didAdapter as? SPVAdaptor)
            if adapter != nil {
                print("Waiting for wallet available to create DID")
                while true {
                    wait(interval: 30)
                    if try adapter!.isAvailable() {
                        print(" OK")
                        break
                    }else {
                        print(".")
                    }
                }
            }
            // Create new DID and publish to ID sidechain.
            var doc = try store.newDid(storePass)
            let did = doc.subject
            var txid = try store.publishDid(did!, storePass)
            XCTAssertNotNil(txid)
            print("Published new DID: \(did!)")
            // Resolve new DID document
            
            if adapter != nil {
                print("Waiting for create transaction confirm")
                while true {
                    wait(interval: 30)
                    if try adapter!.isAvailable() {
                        print(" OK")
                        break
                    }
                    else {
                        print(".")
                    }
                }
                print("Try to resolve new published DID")
                while true {
                    let rdoc = try did!.resolve(true)
                    if rdoc != nil {
                        print(" OK")
                        break
                    }
                    else {
                        print(".")
                    }
                    wait(interval: 30)
                }
            }
            var resolved = try did!.resolve(true)
            XCTAssertEqual(did, resolved?.subject)
            XCTAssertTrue(try resolved!.isValid())
            XCTAssertEqual(doc.description(true), resolved?.description(true))
            try store.storeDid(resolved!)
            var lastTxid = resolved!.getTransactionId()
            print("Last transaction id: \(lastTxid ?? "")")
            // Update
            var db: DIDDocumentBuilder = resolved!.edit()
            
            var key = try TestData.generateKeypair()
            _ = try db.addAuthenticationKey("key1", try key.getPublicKeyBase58())
            doc = try db.seal(storepass: storePass)
            XCTAssertEqual(2, doc.getPublicKeyCount())
            XCTAssertEqual(2, doc.getAuthenticationKeyCount())
            try store.storeDid(doc)

            txid = try store.publishDid(did!, storePass)
            XCTAssertNotNil(txid)
            print("Updated DID: \(did!)")
            if adapter != nil {
                print("Waiting for update transaction confirm")
                while true {
                    wait(interval: 30)
                    if try adapter!.isAvailable() {
                        print(" OK")
                        break
                    }
                    else {
                        print(".")
                    }
                }
                print("Try to resolve updated DID.")
                while true {
                    let rdoc = try did!.resolve(true)
                    if rdoc != nil && rdoc!.getTransactionId() != lastTxid {
                        print(" OK")
                        break
                    }
                    else {
                        print(".")
                    }
                    wait(interval: 30)
                }
            }
            resolved = try did!.resolve(true)
            XCTAssertEqual(did, resolved?.subject)
            XCTAssertTrue(try resolved!.isValid())
            XCTAssertEqual(doc.description(true), resolved?.description(true))
            try store.storeDid(resolved!)
            lastTxid = resolved!.getTransactionId()
            print("Last transaction id: \(lastTxid ?? "")")
            // Update
            db = resolved!.edit()
            key = try TestData.generateKeypair()
            _ = try db.addAuthenticationKey("key2", key.getPublicKeyBase58())
            doc = try db.seal(storepass: storePass)
            XCTAssertEqual(3, doc.getPublicKeyCount())
            XCTAssertEqual(3, doc.getAuthenticationKeyCount())
            try store.storeDid(doc);
            txid = try store.publishDid(did!, storePass)
            XCTAssertNotNil(txid)
            print("Updated DID: \(did!)")
            
            if adapter != nil {
                print("Waiting for update transaction confirm")
                while true {
                    wait(interval: 30)
                    if try adapter!.isAvailable() {
                        print(" OK")
                        break
                    }
                    else {
                        print(".")
                    }
                }
                print("Try to resolve updated DID.")
                while true {
                    let rdoc = try did!.resolve(true)
                    if rdoc != nil && rdoc?.getTransactionId() != lastTxid {
                        print(" OK")
                        break
                    }
                    else {
                        print(".")
                    }
                    wait(interval: 30)
                }
            }
            resolved = try did!.resolve(true)
            XCTAssertEqual(did, resolved?.subject)
            XCTAssertTrue(try resolved!.isValid())
            XCTAssertEqual(doc.description(true), resolved?.description(true))
            
            lastTxid = resolved!.getTransactionId()
            print("Last transaction id: \(lastTxid ?? "")")
        }
        catch {
            
        }
    }

    public func testUpdateAndResolveWithCredentials() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            let mnemonic: String = try testData.initIdentity()
            print("Mnemonic: " + mnemonic)
            var adapter: SPVAdaptor? = nil

            // need synchronize?
            if DIDBackend.shareInstance()!.didAdapter is SPVAdaptor {
                adapter = DIDBackend.shareInstance()!.didAdapter as? SPVAdaptor
            }

            if (adapter != nil) {
                print("Waiting for wallet available to create DID")
                while true {
                    if try adapter!.isAvailable() {
                        print("OK")
                        break
                    } else {
                        print("...")
                    }
                    wait(interval: 30)
                }
            }

            // Create new DID and publish to ID sidechain.
            var doc: DIDDocument = try store.newDid(storePass)
            let did: DID = doc.subject!
            
            var props: OrderedDictionary<String, String> = OrderedDictionary()
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"

            var selfIssuer = try Issuer(doc)
            var cb: CredentialBuilder = selfIssuer.issueFor(did: did)
            var vc: VerifiableCredential = try cb.set(idString: "profile")
                .set(types: ["BasicProfileCredential", "InternetAccountCredential"])
                .set(properties: props)
                .seal(storepass: storePass)

            XCTAssertNotNil(vc)
            var db: DIDDocumentBuilder = doc.edit()
            _ = db.addCredential(vc)
            doc = try db.seal(storepass: storePass)

            XCTAssertNotNil(doc)
            XCTAssertEqual(1, doc.getCredentialCount())
            try store.storeDid(doc)

            var txid = try store.publishDid(did, storePass)
            XCTAssertNotNil(txid)

            print("Published new DID: \(did)")

            // Resolve new DID document
            if adapter != nil {
                print("Waiting for create transaction confirm")
                while true {
                    wait(interval: 30)
                    if try adapter!.isAvailable() {
                        print("OK")
                        break
                    } else {
                        print("...")
                    }
                }
                
                while true {
                    print("Try to resolve new published DID")
                    let rdoc: DIDDocument? = try did.resolve(true)
                    if rdoc != nil {
                        print("OK")
                        break
                    } else {
                        print("...")
                    }
                    wait(interval: 30)
                }
            }
            
            var resolved: DIDDocument = try did.resolve(true)!
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(try resolved.isValid())
            XCTAssertEqual(doc.description(true), resolved.description(true))
            try store.storeDid(resolved)
            var lastTxid: String = resolved.getTransactionId()!
            print("Last transaction id: \(lastTxid)")
            
            // Update
            selfIssuer = try Issuer(resolved)
            props.removeAll(keepCapacity: 0)
            props["nation"] = "Singapore"
            props["passport"] = "S653258Z07"

            cb = selfIssuer.issueFor(did: did)
            vc = try cb.set(idString: "passport")
                .set(types: ["BasicProfileCredential", "SelfProclaimedCredential"])
                .set(properties: props)
                .seal(storepass: storePass)
            
            XCTAssertNotNil(vc)
            db = resolved.edit()
            _ = db.addCredential(vc)
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(2, doc.getCredentialCount())
            try store.storeDid(doc)
            
            txid = try store.publishDid(did, storePass)
            XCTAssertNotNil(txid)
            print("Updated DID: \(did)")
            
            if adapter != nil {
                print("Waiting for update transaction confirm")
                while true {
                    wait(interval: 30)
                    if try adapter!.isAvailable() {
                        print("OK")
                        break
                    } else {
                        print("...")
                    }
                }
                print("Try to resolve updated DID.")
                while true {
                    let rdoc: DIDDocument? = try did.resolve(true)
                    if rdoc != nil && rdoc!.getTransactionId() != lastTxid {
                        print("OK")
                        break
                    } else {
                        print("...")
                    }
                    wait(interval: 30)
                }
            }
            resolved = try did.resolve(true)!
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(try resolved.isValid())
            XCTAssertEqual(doc.description(true), resolved.description(true))
            try store.storeDid(resolved)
            lastTxid = resolved.getTransactionId()!
            print("Last transaction id: \(lastTxid)")
            
            // Update
            selfIssuer = try Issuer(resolved)
            cb = selfIssuer.issueFor(did: did)
            props.removeAll(keepCapacity: 0)
            props["Abc"] = "Abc"
            props["abc"] = "abc"
            props["Foobar"] = "Foobar"
            props["foobar"] = "foobar"
            props["zoo"] = "zoo"
            props["Zoo"] = "Zoo"
            vc = try cb.set(idString: "test")
                .set(types: ["TestCredential", "SelfProclaimedCredential"])
                .set(properties: props)
                .seal(storepass: storePass)
            
            XCTAssertNotNil(vc)
            db = resolved.edit()
            _ = db.addCredential(vc)
            doc = try db.seal(storepass: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(3, doc.getCredentialCount())
            try store.storeDid(doc)
            txid = try store.publishDid(did, storePass)
            XCTAssertNotNil(txid)
            print("Updated DID: \(did)")
            
            if adapter != nil {
                print("Waiting for update transaction confirm")
                while true {
                    wait(interval: 30)
                    if try adapter!.isAvailable() {
                        print("OK")
                        break
                    } else {
                        print("...")
                    }
                }
                print("Try to resolve updated DID.")
                while true {
                    let rdoc: DIDDocument? = try did.resolve(true) ?? nil
                    if rdoc != nil && rdoc!.getTransactionId() != lastTxid {
                        print("OK")
                        break
                    } else {
                        print("...")
                    }
                    wait(interval: 30)
                }
            }
            
            resolved = try did.resolve(true)!
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(try resolved.isValid())
            XCTAssertEqual(doc.description(true), resolved.description(true))
            try store.storeDid(resolved)
            lastTxid = resolved.getTransactionId()!
            print("Last transaction id: \(lastTxid)")
        } catch {
            XCTFail()
        }
    }
    
    public func testRestore() throws {
        let testData: TestData = TestData()
        let store: DIDStore = try testData.setupStore(false)
        let mnemonic: String = try testData.loadRestoreMnemonic()
        try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
        try store.synchronize(storePass) //5 
        print("Synchronizing from IDChain...")
        print("OK")
        
        let dids: Array<DID> = try store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
        var didStrings: Array<String> = []
        XCTAssertEqual(5, dids.count)
        for id in dids {
            didStrings.append(id.description)
        }
        let bl = Bundle(for: type(of: self))
        let path = bl.path(forResource: "dids", ofType: "restore")!
        let jsonstr = try String(contentsOfFile: path)
        let jsonArry = jsonstr.components(separatedBy: "\n")
        for did: String in jsonArry {
            XCTAssertTrue(didStrings.contains(did))
        }
    }
    
    func wait(interval: Double) {
        
        let lock = XCTestExpectation(description: "")
        
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + interval) {
            lock.fulfill()
        }
        wait(for: [lock], timeout: interval + 10)
    }
}
