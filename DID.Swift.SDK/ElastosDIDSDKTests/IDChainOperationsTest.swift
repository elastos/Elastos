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

    func wait(interval: Double) {

        let lock = XCTestExpectation(description: "")

        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + interval) {
            lock.fulfill()
        }
        wait(for: [lock], timeout: interval + 10)
    }
}
