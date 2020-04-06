import XCTest
@testable import ElastosDIDSDK
import PromiseKit

class IDChainOperationsTest: XCTestCase {

    public static let DUMMY_TEST = false

    public func testPublishAndResolve() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            _ = try testData.initIdentity()
            try testData.waitForWalletAvaliable()

            // Create new DID and publish to ID sidechain.
            let doc = try store.newDid(using: storePass)
            let did = doc.subject
            print("Publishing new DID: \(did)...")
            let txid = try store.publishDid(for: did, waitForConfirms: 1, using: storePass)
            XCTAssertNotNil(txid)

            // Resolve new DID document
            try testData.waitForWalletAvaliable()
            let resolved = try did.resolve(true)
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(resolved.isValid)
            XCTAssertEqual(doc.toString(), resolved.toString())
        } catch {
            XCTFail()
        }
    }

    func testUpdateAndResolve() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST) 
            _ = try testData.initIdentity()
            try testData.waitForWalletAvaliable()

            // Create new DID and publish to ID sidechain.
            let doc = try store.newDid(using: storePass)
            let did = doc.subject
            //TODO:
            var lock = XCTestExpectation(description: "publishDidAsync")
            var txid: String?
            _ = store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass).done { str in
                txid = str
                XCTAssertTrue(true)
                lock.fulfill()
            }.catch { error in
                XCTFail()
                lock.fulfill()
            }
            self.wait(for: [lock], timeout: 100.0)
            XCTAssertNotNil(txid)
            print("Published new DID: \(did)")

            // Resolve new DID document
            try testData.waitForWalletAvaliable()
            var resolved: DIDDocument?
            lock = XCTestExpectation(description: "resolveAsync")
            _ = did.resolveAsync(true).done{ doc in
                resolved = doc
                XCTAssertTrue(true)
                lock.fulfill()
            }.catch{ error in
                XCTFail()
                lock.fulfill()
            }
            self.wait(for: [lock], timeout: 100.0)
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved?.toString(true))
        }
        catch {
            XCTFail()
        }
    }

    func testPublishAndResolveAsync() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            _ = try testData.initIdentity()
            try testData.waitForWalletAvaliable()

            // Create new DID and publish to ID sidechain.
            let doc = try store.newDid(using: storePass)
            let did = doc.subject

            print("Publishing new DID: \(did)...")
            let lock = XCTestExpectation(description: "publishDidAsync")
            store.publishDidAsync(for: did, using: storePass).done { tx in
                XCTAssertNotNil(tx)
                XCTAssertTrue(true)
                lock.fulfill()
            }.catch { error in
                XCTFail()
                lock.fulfill()
            }
            self.wait(for: [lock], timeout: 150.0)

            try testData.waitForWalletAvaliable()
            var resolved: DIDDocument?
            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print(".")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved!.toString(true))
        } catch {
            XCTFail()
        }
    }
    
    func testPublishAndResolveAsync2() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            _ = try testData.initIdentity()
            try testData.waitForWalletAvaliable()
            // Create new DID and publish to ID sidechain.
            let doc = try store.newDid(using: storePass)
            let did = doc.subject
            print("Publishing new DID and resolve: \(did)...")
            var resolved: DIDDocument?
            let lock = XCTestExpectation(description: "publishDidAsync")
            store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass)
                .done{ doc in
                    lock.fulfill()
            }
            .catch { error in
                XCTFail()
                lock.fulfill()
            }
            self.wait(for: [lock], timeout: 100.0)

            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print("...")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }

            XCTAssertEqual(did, resolved?.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved?.toString(true))
        } catch {
            XCTFail()
        }
    }

    func testUpdateAndResolveAsync() {
        do {
            var txids: [String] = []
            var sigs: [String] = []

            let testData = TestData()
            let store = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            _ = try testData.initIdentity()
            try testData.waitForWalletAvaliable()

            // Create new DID and publish to ID sidechain.
            var doc = try store.newDid(using: storePass)
            let did = doc.subject
            print("Publishing new DID:  \(did)...")

            var lock = XCTestExpectation()
            store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass).done { tx in
                print("OK")
                txids.append(tx)
                sigs.append(doc.proof.signature)
                XCTAssertNotNil(tx)
                XCTAssertTrue(true)
                lock.fulfill()
            }.catch { error in
                XCTFail()
                lock.fulfill()
            }
            self.wait(for: [lock], timeout: 100.0)

            try testData.waitForWalletAvaliable()
            var resolved: DIDDocument?
            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print("...")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved!.toString(true))

            var lastTxid = resolved!.transactionId
            print("Last transaction id: \(lastTxid)")

            // Update
            var db = doc.editing()
            var key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key1", keyBase58: key.getPublicKeyBase58())

            doc = try db.sealed(using: storePass)
            XCTAssertEqual(2, doc.publicKeyCount)
            XCTAssertEqual(2, doc.authenticationKeyCount)
            try store.storeDid(using: doc)

            lock = XCTestExpectation()
            store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass).done { tx in
                print("OK")
                txids.append(tx)
                sigs.append(doc.proof.signature)
                XCTAssertNotNil(tx)
                XCTAssertTrue(true)
                lock.fulfill()
            }.catch { error in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)

            try testData.waitForWalletAvaliable()
            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print("...")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }
            XCTAssertNotEqual(lastTxid, resolved!.transactionId)
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved!.toString(true))

            lastTxid = resolved!.transactionId
            print("Last transaction id: \(lastTxid)")

            // Update again
            db = doc.editing()
            key = try TestData.generateKeypair()
            _ = try db.appendAuthenticationKey(with: "key2", keyBase58: key.getPublicKeyBase58())
            doc = try db.sealed(using: storePass)
            XCTAssertEqual(3, doc.publicKeyCount)
            XCTAssertEqual(3, doc.authenticationKeyCount)
            try store.storeDid(using: doc)
            print("Updating DID: \(did)...")

            lock = XCTestExpectation()
            _ = try store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass).done{ tx in
                print("OK")
                txids.append(tx)
                sigs.append(doc.proof.signature)
                XCTAssertNotNil(tx)
                XCTAssertTrue(true)
                lock.fulfill()
            }.catch{ error in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)

            try testData.waitForWalletAvaliable()
            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print("...")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }
            XCTAssertNotEqual(lastTxid, resolved!.transactionId)
            XCTAssertEqual(did, resolved?.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved!.toString(true))

            lastTxid = resolved?.transactionId
            print("Last transaction id: \(lastTxid)")
            lock = XCTestExpectation()
            did.resolveHistoryAsync().done{ his in
                XCTAssertNotNil(his)
                XCTAssertEqual(did, his.getDid())
                XCTAssertEqual(ResolveResultStatus.STATUS_VALID, his.getsStatus())
                XCTAssertEqual(3, his.getTransactionCount())
                var txs = his.getAllTransactions()
                XCTAssertNotNil(txs)
                XCTAssertEqual(3, txs.count)

                txs.reverse()
//                sigs.reverse()
                for i in 0..<txs.count {
                    let tx = txs[i]
                    XCTAssertEqual(did, tx.getDid())
                    XCTAssertEqual(txids[i], tx.getTransactionId());
                    XCTAssertEqual(sigs[i], tx.getDocument().proof.signature)
                }
                lock.fulfill()
            }.catch{ error in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)
        } catch {
            XCTFail()
        }
    }

    public func testUpdateAndResolveWithCredentials() throws {
        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            let mnemonic: String = try testData.initIdentity()
            print("Mnemonic: " + mnemonic)
            try testData.waitForWalletAvaliable()

            // Create new DID and publish to ID sidechain.
            var doc: DIDDocument = try store.newDid(using: storePass)
            let did: DID = doc.subject

            var selfIssuer = try VerifiableCredentialIssuer(doc)
            var cb = selfIssuer.editingVerifiableCredentialFor(did: did)

            var props: Dictionary<String, String> = [: ]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"

            var vc: VerifiableCredential = try cb.withId("profile")
                .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)

            XCTAssertNotNil(vc)

            var db: DIDDocumentBuilder = doc.editing()
            doc = try db.appendCredential(with: vc)
                .sealed(using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(1, doc.credentialCount)
            try store.storeDid(using: doc)

            print("Published new DID: \(did)")
            var txid = try store.publishDid(for: did, waitForConfirms: 1, using: storePass)
            XCTAssertNotNil(txid)

            try! testData.waitForWalletAvaliable()
            var resolved: DIDDocument = try! did.resolve(true)
            XCTAssertEqual(did, resolved.subject)
            XCTAssertTrue(resolved.isValid)
            XCTAssertEqual(doc.toString(true), resolved.toString(true))

            var lastTxid = resolved.transactionId
            print("Last transaction id: \(lastTxid)")

            // Update
            selfIssuer = try! VerifiableCredentialIssuer(doc)
            cb = selfIssuer.editingVerifiableCredentialFor(did: did)

            props.removeAll()
            props["nation"] = "Singapore"
            props["passport"] = "S653258Z07"

            vc = try! cb.withId("passport")
                .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)
            XCTAssertNotNil(vc)

            db = doc.editing()
            doc = try! db.appendCredential(with: vc)
                .sealed(using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(2, doc.credentialCount)
            try! store.storeDid(using: doc)

            print("Updated DID: \(did)")
            txid = try! store.publishDid(for: did, waitForConfirms: 1, using: storePass)
            XCTAssertNotNil(txid)

            try! testData.waitForWalletAvaliable()
            resolved = try! did.resolve(true)
            XCTAssertEqual(did, resolved.subject)
            XCTAssertNotEqual(lastTxid, resolved.transactionId)
            XCTAssertTrue(resolved.isValid)
            XCTAssertEqual(doc.toString(true), resolved.toString(true))

            lastTxid = resolved.transactionId
            print("Last transaction id: \(lastTxid)")

            // Update again
            selfIssuer = try! VerifiableCredentialIssuer(doc)
            cb = selfIssuer.editingVerifiableCredentialFor(did: did)
            props.removeAll()
            props["Abc"] = "Abc"
            props["abc"] = "abc"
            props["Foobar"] = "Foobar"
            props["foobar"] = "foobar"
            props["zoo"] = "zoo"
            props["Zoo"] = "Zoo"
            vc = try! cb.withId("test")
                .withTypes("TestCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)
            XCTAssertNotNil(vc)

            db = doc.editing()
            doc = try! db.appendCredential(with: vc)
                .sealed(using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(3, doc.credentialCount)
            try! store.storeDid(using: doc)

            print("Updated DID: \(did)")
            txid = try! store.publishDid(for: did, waitForConfirms: 1, using: storePass)
            XCTAssertNotNil(txid)

            try! testData.waitForWalletAvaliable()
            resolved = try! did.resolve(true)
            XCTAssertEqual(did, resolved.subject)
            XCTAssertNotEqual(lastTxid, resolved.transactionId)
            XCTAssertTrue(resolved.isValid)
            XCTAssertEqual(doc.toString(true), resolved.toString(true))
            lastTxid = resolved.transactionId
            print("Last transaction id: \(lastTxid)")
        } catch {
            XCTFail()
        }
    }

    func testUpdateAndResolveWithCredentialsAsync() {
        do {
            let testData = TestData()
            let store = try testData.setupStore(IDChainOperationsTest.DUMMY_TEST)
            _ =  try testData.initIdentity()
            try testData.waitForWalletAvaliable()

            // Create new DID and publish to ID sidechain.
            var doc = try store.newDid(using: storePass)
            let did = doc.subject

            var selfIssuer = try VerifiableCredentialIssuer(doc)
            var cb = selfIssuer.editingVerifiableCredentialFor(did: did)

            var props: [String: String] = [: ]
            props["name"] = "John"
            props["gender"] = "Male"
            props["nation"] = "Singapore"
            props["language"] = "English"
            props["email"] = "john@example.com"
            props["twitter"] = "@john"

            var vc = try cb.withId("profile")
                .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)
            XCTAssertNotNil(vc)

            var db = doc.editing()
            _ = try db.appendCredential(with: vc)
            doc = try db.sealed(using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(1, doc.credentialCount)
            try store.storeDid(using: doc)

            print("Publishing new DID: \(did)...")
            var lock = XCTestExpectation()
            _ =  try store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass).done{ tx in
                print("OK")
                XCTAssertNotNil(tx)
                lock.fulfill()
            }.catch{ error in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)

            try testData.waitForWalletAvaliable()
            var resolved: DIDDocument?
            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print("...")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved!.toString(true))

            var lastTxid = resolved!.transactionId
            print("Last transaction id: \(lastTxid)")

            // Update
            selfIssuer = try VerifiableCredentialIssuer(doc)
            cb = selfIssuer.editingVerifiableCredentialFor(did: did)

            props.removeAll()
            props["nation"] = "Singapore"
            props["passport"] = "S653258Z07"

            vc = try cb.withId("passport")
                .withTypes("BasicProfileCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)
            XCTAssertNotNil(vc)

            db = doc.editing()
            _ = try db.appendCredential(with: vc)
            try doc = db.sealed(using: storePass)
            XCTAssertNotNil(doc)
            XCTAssertEqual(2, doc.credentialCount)
            try store.storeDid(using: doc)

            lock = XCTestExpectation()
            print("Updating DID: \(did)...")
            store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass).done { tx in
                print("OK")
                XCTAssertNotNil(tx)
                lock.fulfill()
            }.catch { error in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)

            try testData.waitForWalletAvaliable()
            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print("...")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }

            XCTAssertNotEqual(lastTxid, resolved!.transactionId)
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved!.toString(true))

            lastTxid = resolved!.transactionId
            print("Last transaction id: \(lastTxid)")

            // Update again
            selfIssuer = try VerifiableCredentialIssuer(doc)
            cb = selfIssuer.editingVerifiableCredentialFor(did: did)

            props.removeAll()
            props["Abc"] = "Abc"
            props["abc"] = "abc"
            props["Foobar"] = "Foobar"
            props["foobar"] = "foobar"
            props["zoo"] = "zoo"
            props["Zoo"]  = "Zoo"

            vc = try cb.withId("test")
                .withTypes("TestCredential", "SelfProclaimedCredential")
                .withProperties(props)
                .sealed(using: storePass)
            XCTAssertNotNil(vc)

            db = doc.editing()
            _ = try db.appendCredential(with: vc)
            doc = try db.sealed(using: storePass)
            XCTAssertNotNil(doc)
            try store.storeDid(using: doc)

            print("Updating DID: \(did)...")
            lock = XCTestExpectation()
            store.publishDidAsync(for: did, waitForConfirms: 1, using: storePass).done { tx in
                print("OK")
                XCTAssertNotNil(tx)
                lock.fulfill()
            }.catch { error in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)

            try testData.waitForWalletAvaliable()
            while true {
                did.resolveAsync(true).done{ doc in
                    print(" OK")
                    resolved = doc
                }.catch{ error in
                    print("...")
                }
                wait(interval: 30)
                if resolved != nil {
                    break
                }
            }
            XCTAssertNotEqual(lastTxid, resolved!.transactionId)
            XCTAssertEqual(did, resolved!.subject)
            XCTAssertTrue(resolved!.isValid)
            XCTAssertEqual(doc.toString(true), resolved!.toString(true))

            lastTxid = resolved!.transactionId
            print("Last transaction id: \(lastTxid)")
        } catch {
            XCTFail()
        }
    }

    public func testRestore() throws {
        if (IDChainOperationsTest.DUMMY_TEST) {
            return
        }

        do {
            let testData: TestData = TestData()
            let store: DIDStore = try testData.setupStore(false)
            let mnemonic: String = try testData.loadRestoreMnemonic()
            try store.initializePrivateIdentity(Mnemonic.ENGLISH, mnemonic, passphrase, storePass, true)
            try store.synchronize(using: storePass) //5
            print("Synchronizing from IDChain...")
            print("OK")

            let dids: Array<DID> = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            var didStrings: Array<String> = []
            XCTAssertEqual(5, dids.count)
            for id in dids {
                didStrings.append(id.toString())
            }
            let bl = Bundle(for: type(of: self))
            let path = bl.path(forResource: "dids", ofType: "restore")!
            let jsonstr = try String(contentsOfFile: path)
            let jsonArry = jsonstr.components(separatedBy: "\n")
            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))
                // TODO:
            }
        } catch {
            XCTFail()
        }
    }

    func testRestoreAsync() {
        if (IDChainOperationsTest.DUMMY_TEST) {
            return
        }
        do {
            let testData = TestData()
            let store = try testData.setupStore(false)

            let mnemonic = try testData.loadRestoreMnemonic()

            try store.initializePrivateIdentity(Mnemonic.ENGLISH, mnemonic, passphrase, storePass, true)

            print("Synchronizing from IDChain...")

            let lock = XCTestExpectation()
            try store.synchronizeAsync(using: storePass).done { _ in
                print("OK")
                XCTAssertTrue(true)
                lock.fulfill()
            }.catch { errro in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)

            let dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(5, dids.count)

            var didStrings: [String] = []
            dids.forEach { did in
                didStrings.append(did.toString())
            }

            let bl = Bundle(for: type(of: self))
            let path = bl.path(forResource: "dids", ofType: "restore")!
            let jsonstr = try String(contentsOfFile: path)
            let jsonArry = jsonstr.components(separatedBy: "\n")
            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))

                let did = try DID(did)
                let doc = try store.loadDid(did)
                XCTAssertNotNil(doc)
                XCTAssertEqual(did, doc.subject)
                XCTAssertEqual(4, doc.credentialCount)

                let vcs = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)

                try vcs.forEach { id in
                    let vc = try store.loadCredential(for: did, byId: id)
                    XCTAssertNotNil(vc)
                    XCTAssertEqual(id, vc!.getId())
                }
            }
        } catch {
            XCTFail()
        }
    }

    func testSyncWithLocalModification1() {
        if (IDChainOperationsTest.DUMMY_TEST) {
            return
        }
        do {
            let testData = TestData()
            let store = try testData.setupStore(false)

            let mnemonic = try testData.loadRestoreMnemonic()

            try store.initializePrivateIdentity(Mnemonic.ENGLISH, mnemonic, passphrase, storePass, true)
            print("Synchronizing from IDChain...")
            try store.synchronize(using: storePass)
            print("OK")

            var dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(5, dids.count)

            var didStrings: [String] = []
            dids.forEach { id in
                didStrings.append(id.toString())
            }

            var bl = Bundle(for: type(of: self))
            var path = bl.path(forResource: "dids", ofType: "restore")!
            var jsonstr = try String(contentsOfFile: path)
            var jsonArry = jsonstr.components(separatedBy: "\n")

            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))

                let did = try DID(did)
                let doc = try store.loadDid(did)
                XCTAssertNotNil(doc)
                XCTAssertEqual(did, doc.subject)
                XCTAssertEqual(4, doc.credentialCount)

                let vcs = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)

                try vcs.forEach { id in
                    let vc = try store.loadCredential(for: did, byId: id)
                    XCTAssertNotNil(vc)
                    XCTAssertEqual(id, vc!.getId())
                }
            }

            let modifiedDid = dids[0]
            var doc = try store.loadDid(modifiedDid)
            let db = doc.editing()
            _ = try db.appendService(with: "test1", type: "TestType", endpoint: "http://test.com/")
            doc = try db.sealed(using: storePass)
            try store.storeDid(using: doc)
            let modifiedSignature = doc.proof.signature

            print("Synchronizing again from IDChain...")
            try store.synchronize(using: storePass)
            print("OK")

            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(5, dids.count)

            didStrings.removeAll()
            dids.forEach { id in
                didStrings.append(id.toString())
            }

            bl = Bundle(for: type(of: self))
            path = bl.path(forResource: "dids", ofType: "restore")!
            jsonstr = try String(contentsOfFile: path)
            jsonArry = jsonstr.components(separatedBy: "\n")

            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))

                let did = try DID(did)
                let doc = try store.loadDid(did)
                XCTAssertNotNil(doc)
                XCTAssertEqual(did, doc.subject)
                XCTAssertEqual(4, doc.credentialCount)

                let vcs = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)

                try vcs.forEach { id in
                    let vc = try store.loadCredential(for: did, byId: id)
                    XCTAssertNotNil(vc)
                    XCTAssertEqual(id, vc!.getId())
                }
            }

            doc = try store.loadDid(modifiedDid)
            XCTAssertEqual(modifiedSignature, doc.proof.signature)
        } catch {
            XCTFail()
        }
    }

    func testSyncWithLocalModification2() {
        if (IDChainOperationsTest.DUMMY_TEST) {
            return
        }
        do {
            let testData = TestData()
            let store = try testData.setupStore(false)

            let mnemonic = try testData.loadRestoreMnemonic()

            try store.initializePrivateIdentity(Mnemonic.ENGLISH, mnemonic, passphrase, storePass, true)
            print("Synchronizing from IDChain...")
            try store.synchronize(using: storePass)
            print("OK")

            var dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(5, dids.count)

            var didStrings: [String] = []
            dids.forEach { id in
                didStrings.append(id.toString())
            }

            var bl = Bundle(for: type(of: self))
            var path = bl.path(forResource: "dids", ofType: "restore")!
            var jsonstr = try String(contentsOfFile: path)
            var jsonArry = jsonstr.components(separatedBy: "\n")

            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))

                let did = try DID(did)
                let doc = try store.loadDid(did)
                XCTAssertNotNil(doc)
                XCTAssertEqual(did, doc.subject)
                XCTAssertEqual(4, doc.credentialCount)

                let vcs = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)

                try vcs.forEach { id in
                    let vc = try store.loadCredential(for: did, byId: id)
                    XCTAssertNotNil(vc)
                    XCTAssertEqual(id, vc!.getId())
                }
            }

            var modifiedDid = dids[0]
            var doc = try store.loadDid(modifiedDid)
            var originSignature = doc.proof.signature

            var db = doc.editing()
            _ = try db.appendService(with: "test1", type: "TestType", endpoint: "http://test.com/")
            doc = try db.sealed(using: storePass)
            try store.storeDid(using: doc)
            XCTAssertNotEqual(originSignature, doc.proof.signature)

            print("OK")
            try store.synchronize(using: storePass, conflictHandler: { (chain, loca) -> DIDDocument in
                print("OK")
                return chain
            })

            modifiedDid = dids[0]
            doc = try store.loadDid(modifiedDid)
            originSignature = doc.proof.signature

            db = doc.editing()
            _ = try db.appendService(with: "test1", type: "TestType", endpoint: "http://test.com/")
            doc = try db.sealed(using: storePass)
            try store.storeDid(using: doc)
            XCTAssertNotEqual(originSignature, doc.proof.signature)

            print("Synchronizing again from IDChain...")
            try store.synchronize(using: storePass) { (chain, loca) -> DIDDocument in
                return chain
            }
            print("OK")

            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(5, dids.count)

            didStrings.removeAll()
            dids.forEach { id in
                didStrings.append(id.toString())
            }

            bl = Bundle(for: type(of: self))
            path = bl.path(forResource: "dids", ofType: "restore")!
            jsonstr = try String(contentsOfFile: path)
            jsonArry = jsonstr.components(separatedBy: "\n")

            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))

                let did = try DID(did)
                let doc = try store.loadDid(did)
                XCTAssertNotNil(doc)
                XCTAssertEqual(did, doc.subject)
                XCTAssertEqual(4, doc.credentialCount)

                let vcs = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)

                try vcs.forEach { id in
                    let vc = try store.loadCredential(for: did, byId: id)
                    XCTAssertNotNil(vc)
                    XCTAssertEqual(id, vc!.getId())
                }
            }

            doc = try store.loadDid(modifiedDid)
            XCTAssertEqual(originSignature, doc.proof.signature)
        } catch {
            XCTFail()
        }
    }

    func testSyncWithLocalModificationAsync() {
        if (IDChainOperationsTest.DUMMY_TEST) {
            return
        }
        do {
            let testData = TestData()
            let store = try testData.setupStore(false)

            let mnemonic = try testData.loadRestoreMnemonic()

            try store.initializePrivateIdentity(Mnemonic.ENGLISH, mnemonic, passphrase, storePass, true)
            print("Synchronizing from IDChain...")
            var lock = XCTestExpectation()
            try store.synchronizeAsync(using: storePass).done{ _ in
                print("OK")
                lock.fulfill()
            }.catch{ error in
                XCTFail()
                lock.fulfill()
            }
            wait(for: [lock], timeout: 100.0)

            var dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(5, dids.count)

            var didStrings: [String] = []
            dids.forEach { id in
                didStrings.append(id.toString())
            }

            var bl = Bundle(for: type(of: self))
            var path = bl.path(forResource: "dids", ofType: "restore")!
            var jsonstr = try String(contentsOfFile: path)
            var jsonArry = jsonstr.components(separatedBy: "\n")

            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))

                let did = try DID(did)
                let doc = try store.loadDid(did)
                XCTAssertNotNil(doc)
                XCTAssertEqual(did, doc.subject)
                XCTAssertEqual(4, doc.credentialCount)

                let vcs = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)

                try vcs.forEach { id in
                    let vc = try store.loadCredential(for: did, byId: id)
                    XCTAssertNotNil(vc)
                    XCTAssertEqual(id, vc!.getId())
                }
            }

            let modifiedDid = dids[0]
            var doc = try store.loadDid(modifiedDid)
            let originSignature = doc.proof.signature

            let db = doc.editing()
            _ = try db.appendService(with: "test1", type: "TestType", endpoint: "http://test.com/")
            doc = try db.sealed(using: storePass)
            try store.storeDid(using: doc)
            XCTAssertNotEqual(originSignature, doc.proof.signature)

            print("Synchronizing again from IDChain...")
            lock = XCTestExpectation()
            _ = store.synchornizeAsync(using: storePass) { (c, l) -> DIDDocument in
                print("OK")
                lock.fulfill()
                return c
            }
            wait(for: [lock], timeout: 100.0)

            dids = try store.listDids(using: DIDStore.DID_HAS_PRIVATEKEY)
            XCTAssertEqual(5, dids.count)

            didStrings.removeAll()
            dids.forEach { id in
                didStrings.append(id.toString())
            }

            bl = Bundle(for: type(of: self))
            path = bl.path(forResource: "dids", ofType: "restore")!
            jsonstr = try String(contentsOfFile: path)
            jsonArry = jsonstr.components(separatedBy: "\n")

            for did: String in jsonArry {
                XCTAssertTrue(didStrings.contains(did))

                let did = try DID(did)
                let doc = try store.loadDid(did)
                XCTAssertNotNil(doc)
                XCTAssertEqual(did, doc.subject)
                XCTAssertEqual(4, doc.credentialCount)

                let vcs = try store.listCredentials(for: did)
                XCTAssertEqual(4, vcs.count)

                try vcs.forEach { id in
                    let vc = try store.loadCredential(for: did, byId: id)
                    XCTAssertNotNil(vc)
                    XCTAssertEqual(id, vc!.getId())
                }
            }

            doc = try store.loadDid(modifiedDid)
            XCTAssertEqual(originSignature, doc.proof.signature)
        } catch {
            XCTFail()
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
