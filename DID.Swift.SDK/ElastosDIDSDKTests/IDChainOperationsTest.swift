
import XCTest
import ElastosDIDSDK

class IDChainOperationsTest: XCTestCase {

    public static let DUMMY_TEST = false

    public func testPublishAndResolve() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(false)
            try testData.initIdentity()
            let adapter: SPVAdaptor = DIDBackend.shareInstance().adapter as! SPVAdaptor
            
            while true {
                if try adapter.isAvailable() {
                    print("OK")
                    break
                }
                else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting for wallet available, Waiting 30s")
                wait(for: [lock], timeout: 30)
            }
            let doc = try store.newDid(storePass)
            let did = doc.subject
            let txid = try store.publishDid(did!, storePass)
            XCTAssertNotNil(txid)
            print("Published new DID: \(doc.subject!)")
            
            print("Try to resolve new published DID.")
            while true {
                let lock = XCTestExpectation(description: "******** Waiting for wallet available, Waiting 30s")
                wait(for: [lock], timeout: 30)
                let resolved = try store.resolveDid(doc.subject!, true)
                if resolved != nil {
                    print("OK")
                    XCTAssertEqual(doc.subject!, resolved!.subject)
                    XCTAssertTrue(try resolved!.isValid())
                    break
                }
                else {
                    print("...")
                }
            }
        } catch {
            XCTFail()
        }
    }
    
    func testUpdateAndResolve() {
        do {
        let testData = TestData()
            var store = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            try testData.initIdentity()
            var adapter: SPVAdaptor? = nil
//             need synchronize?
            adapter = (DIDBackend.shareInstance().adapter as? SPVAdaptor)
            if adapter == nil {
                print("Waiting for wallet available to create DID")
                while true {
                    if try (adapter?.isAvailable())! {
                        print(" OK")
                        break
                    }else {
                        print(".")
                    }
                    let lock = XCTestExpectation(description: "******** Waiting for wallet available, Waiting 30s")
                    wait(for: [lock], timeout: 30)
                }
            }
            // Create new DID and publish to ID sidechain.
            var doc = try store.newDid(storePass)
            var did = doc.subject
            var txid = try store.publishDid(did!, storePass)
            XCTAssertNotNil(txid)
            print("Published new DID: \(did)")
            // Resolve new DID document
            
            if adapter != nil {
                print("Waiting for create transaction confirm")
                while true {
                    let lock = XCTestExpectation(description: "******** Waiting for wallet available, Waiting 30s")
                    wait(for: [lock], timeout: 30)
                    if try adapter!.isAvailable() {
                        print(" OK")
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
                    }
                    else {
                        print(".")
                    }
                    let lock = XCTestExpectation(description: "******** Waiting for wallet available, Waiting 30s")
                    wait(for: [lock], timeout: 30)
                }
            }
            var resolved = try did!.resolve(true)
            XCTAssertEqual(did, resolved?.subject)
            XCTAssertTrue(try resolved!.isValid())
            XCTAssertEqual(try doc.description(true), try resolved?.description(true))
            try store.storeDid(resolved!)
            var lastTxid = resolved!.getTransactionId()
            print("Last transaction id: \(lastTxid)")
            // Update
//            DIDDocument.Builder db = resolved.edit();
//            let key = try TestData.generateKeypair()
//            try resolved?.addAuthenticationKey("key1", try key.getPublicKeyBase58())
//            doc = resolved!.seal(, storePass)
//            XCTAssertEqual(2, doc.getPublicKeyCount())
//            XCTAssertEqual(2, doc.getAuthenticationKeyCount())
//            try store.storeDid(doc)
        }
        catch {
            
        }
    }
    /*
     public void testUpdateAndResolve() throws DIDException {


         txid = store.publishDid(did, TestConfig.storePass);
         assertNotNull(txid);
         System.out.println("Updated DID: " + did);

         if (adapter != null) {
             System.out.print("Waiting for update transaction confirm");
             while (true) {
                 try {
                     Thread.sleep(30000);
                 } catch (InterruptedException ignore) {
                 }

                 if (adapter.isAvailable()) {
                     System.out.println(" OK");
                     break;
                 } else {
                     System.out.print(".");
                 }
             }

             System.out.print("Try to resolve updated DID.");
             while (true) {
                 try {
                     DIDDocument rdoc = did.resolve(true);
                     if (rdoc != null && rdoc.getTransactionId() != lastTxid) {
                         System.out.println(" OK");
                         break;
                     } else {
                         System.out.print(".");
                     }
                 } catch (Exception ignore) {
                     System.out.print("x");
                 }

                 try {
                     Thread.sleep(30000);
                 } catch (InterruptedException ignore) {
                 }
             }
         }

         resolved = did.resolve(true);
         assertEquals(did, resolved.getSubject());
         assertTrue(resolved.isValid());
         assertEquals(doc.toString(true), resolved.toString(true));
         store.storeDid(resolved);

         lastTxid = resolved.getTransactionId();
         System.out.println("Last transaction id: " + lastTxid);

         // Update
         db = resolved.edit();
         key = TestData.generateKeypair();
         db.addAuthenticationKey("key2", key.getPublicKeyBase58());
         doc = db.seal(TestConfig.storePass);
         assertEquals(3, doc.getPublicKeyCount());
         assertEquals(3, doc.getAuthenticationKeyCount());
         store.storeDid(doc);

         txid = store.publishDid(did, TestConfig.storePass);
         assertNotNull(txid);
         System.out.println("Updated DID: " + did);

         if (adapter != null) {
             System.out.print("Waiting for update transaction confirm");
             while (true) {
                 try {
                     Thread.sleep(30000);
                 } catch (InterruptedException ignore) {
                 }

                 if (adapter.isAvailable()) {
                     System.out.println(" OK");
                     break;
                 } else {
                     System.out.print(".");
                 }
             }

             System.out.print("Try to resolve updated DID.");
             while (true) {
                 try {
                     DIDDocument rdoc = did.resolve(true);
                     if (rdoc != null && rdoc.getTransactionId() != lastTxid) {
                         System.out.println(" OK");
                         break;
                     } else {
                         System.out.print(".");
                     }
                 } catch (Exception ignore) {
                     System.out.print("x");
                 }

                 try {
                     Thread.sleep(30000);
                 } catch (InterruptedException ignore) {
                 }
             }
         }

         resolved = did.resolve(true);
         assertEquals(did, resolved.getSubject());
         assertTrue(resolved.isValid());
         assertEquals(doc.toString(true), resolved.toString(true));

         lastTxid = resolved.getTransactionId();
         System.out.println("Last transaction id: " + lastTxid);
     }
     */
    public func testUpdateAndResolveWithCredentials() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(false)
            let mnemonic: String = try testData.initIdentity()
            print("Mnemonic: " + mnemonic)

            var adapter: SPVAdaptor = DIDBackend.shareInstance().adapter as! SPVAdaptor

            // need synchronize?
            if DIDBackend.shareInstance().adapter is SPVAdaptor {
                adapter = DIDBackend.shareInstance().adapter as! SPVAdaptor
            }

            print("Waiting for wallet available to create DID")
            while true {
                if try adapter.isAvailable() {
                    print("OK")
                    break
                } else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting for adapter available, Waiting 30s")
                wait(for: [lock], timeout: 30)
            }

            // Create new DID and publish to ID sidechain.

            var doc: DIDDocument = try store.newDid(storePass)
            let did: DID = doc.subject!

            var selfIssuer: Issuer = try Issuer(doc)


            var credential: VerifiableCredential = VerifiableCredential()
            credential.id = try DIDURL(did, "profile")
            credential.types = ["BasicProfileCredential", "SelfProclaimedCredential"]

            var cs: CredentialSubject = CredentialSubject(did)
            cs.addProperty("name", "John")
            cs.addProperty("gender", "Male")
            cs.addProperty("nation", "Singapore")
            cs.addProperty("language", "English")
            cs.addProperty("email", "john@example.com")
            cs.addProperty("twitter", "@john")

            var vc: VerifiableCredential = try selfIssuer.seal(for: did, "profile", credential.types, cs.properties, storePass)

            doc.addCredential(vc)
            doc = try doc.seal(store, storePass)

            XCTAssertNotNil(doc)
            XCTAssertEqual(1, doc.getCredentialCount())
            try store.storeDid(doc)

            var txid: String = try store.publishDid(did, storePass)!
            XCTAssertNotNil(txid)

            print("Published new DID: \(did)")

            print("Waiting for create transaction confirm")
            while true {
                if try adapter.isAvailable() {
                    print("OK")
                    break
                } else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting for adapter available, Waiting 30s")
                wait(for: [lock], timeout: 30)
            }

            print("Try to resolve new published DID")

            while true {
                let rdoc: DIDDocument? = try did.resolve(true) ?? nil
                if rdoc != nil {
                    print("OK")
                    break
                } else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting 30s")
                wait(for: [lock], timeout: 30)
            }

            var resolved: DIDDocument = try did.resolve(true)!
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(try resolved.isValid())
            // TODO 判断方法有问题
            XCTAssertEqual(did.description, resolved.description)
            try store.storeDid(resolved)
            var lastTxid: String = resolved.getTransactionId()
            print("Last transaction id: \(lastTxid)")

            // Update
            selfIssuer = try Issuer(resolved);
            credential = VerifiableCredential()
            credential.id = try DIDURL(did, "passport")
            credential.types = ["BasicProfileCredential", "SelfProclaimedCredential"]

            cs = CredentialSubject(resolved.subject!)
            cs.addProperty("nation", "Singapore")
            cs.addProperty("passport", "S653258Z07")
            vc = try selfIssuer.seal(for: resolved.subject!, "passport", credential.types, cs.properties, storePass)

            XCTAssertNotNil(vc)
            _ = resolved.addCredential(vc)
            _ = try resolved.seal(store, storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(2, doc.getCredentialCount())
            try store.storeDid(doc)
            txid = try store.publishDid(did, storePass)!
            XCTAssertNotNil(txid)
            print("Updated DID: \(did)")

            print("Waiting for update transaction confirm")
            while true {
                if try adapter.isAvailable() {
                    print("OK")
                    break
                } else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting for adapter available, Waiting 30s")
                wait(for: [lock], timeout: 30)
            }
            print("Try to resolve updated DID.")
            while true {
                let rdoc: DIDDocument? = try did.resolve(true) ?? nil
                if rdoc != nil {
                    print("OK")
                    break
                } else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting 30s")
                wait(for: [lock], timeout: 30)
            }

            resolved = try did.resolve(true)!
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(try resolved.isValid())
            // TODO 判断方法有问题
            XCTAssertEqual(did.description, resolved.description)
            try store.storeDid(resolved)
            lastTxid = resolved.getTransactionId()
            print("Last transaction id: \(lastTxid)")

            selfIssuer = try Issuer(resolved);
            credential = VerifiableCredential()
            credential.id = try DIDURL(did, "test")
            credential.types = ["TestCredential", "SelfProclaimedCredential"]

            cs = CredentialSubject(resolved.subject!)
            cs.addProperty("Abc", "Abc")
            cs.addProperty("abc", "abc")
            cs.addProperty("Foobar", "Foobar")
            cs.addProperty("foobar", "foobar")
            cs.addProperty("zoo", "zoo")
            cs.addProperty("Zoo", "Zoo")
            vc = try selfIssuer.seal(for: resolved.subject!, "test", credential.types, cs.properties, storePass)

            XCTAssertNotNil(vc)
            resolved.addCredential(vc)
            try resolved.seal(store, storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(3, doc.getCredentialCount())
            try store.storeDid(doc)
            txid = try store.publishDid(did, storePass)!
            XCTAssertNotNil(txid)
            print("Updated DID: \(did)")

            print("Waiting for update transaction confirm")
            while true {
                if try adapter.isAvailable() {
                    print("OK")
                    break
                } else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting for adapter available, Waiting 30s")
                wait(for: [lock], timeout: 30)
            }
            print("Try to resolve updated DID.")
            while true {
                let rdoc: DIDDocument? = try did.resolve(true) ?? nil
                if rdoc != nil {
                    print("OK")
                    break
                } else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting 30s")
                wait(for: [lock], timeout: 30)
            }

            resolved = try did.resolve(true)!
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(try resolved.isValid())
            // TODO 判断方法有问题
            XCTAssertEqual(did.description, resolved.description)
            try store.storeDid(resolved)
            lastTxid = resolved.getTransactionId()
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
        try store.synchronize(storePass)
        
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

}
