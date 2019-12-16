
import XCTest
import ElastosDIDSDK

class IDChainOperationsTest: XCTestCase {

    public func testPublishAndResolve() throws {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(false)
            try testData.initIdentity()
            let store: DIDStore = try DIDStore.shareInstance()!
            XCTAssertTrue(store.getAdapter())
            let adapter: SPVAdapter = store.getAdapter()
            
            while true {
                if adapter.isAvailable() {
                    print("OK")
                    break
                }
                else {
                    print("...")
                }
                let lock = XCTestExpectation(description: "******** Waiting for wallet available, Waiting 30s")
                wait(for: [lock], timeout: 30)
            }
            let doc = store.newDid(storePass)
            let success = store.publishDid(doc, storePass)
            XCTAssertTrue(success)
            print("Published new DID: \(doc.subject)")
            
            let resolved: DIDDocument?
            print("Try to resolve new published DID.")
            while true {
                let lock = XCTestExpectation(description: "******** Waiting for wallet available, Waiting 30s")
                wait(for: [lock], timeout: 30)
                resolved = store.resolveDid(doc.subject, true)
                if resolved != nil {
                    print("OK")
                    break
                }
                else {
                    print("...")
                }
            }
            XCTAssertEqual(doc.subject, resolved.subject)
            XCTAssertTrue(resolved.isValid)
        } catch {
            
        }
    }
    
    public func testRestore() throws {
        let testData: TestData = TestData()
        testData.setupStore(false)
        let store: DIDStore = try DIDStore.shareInstance()
        let mnemonic: String = try testData.loadRestoreMnemonic()
        store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
        store.synchronize(storePass)
        
        let dids: Array<DID> = store.listDids(DIDStore.DID_HAS_PRIVATEKEY)
        var didStrings: Array<String> = []
        XCTAssertEqual(5, dids.count)
        for item in dids {
            didStrings.add(id.description)
        }
        let bl = Bundle(for: type(of: self))
        let jsonstr = bl.path(forResource: "dids.restore", ofType: "")
        // TODO: 按行读取
        
    }

}
