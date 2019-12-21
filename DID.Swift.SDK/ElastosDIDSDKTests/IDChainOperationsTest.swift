
import XCTest
import ElastosDIDSDK

class IDChainOperationsTest: XCTestCase {

    public func testPublishAndResolve() throws {
        do {
            let testData: TestData = TestData()
            try testData.setupStore(false)
            try testData.initIdentity()
            let store: DIDStore = try DIDStore.shareInstance()!
//            XCTAssertTrue(store.getAdapter())
            let adapter: SPVAdaptor = store.getAdapter() as! SPVAdaptor
            
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
            let success = try store.publishDid(doc, storePass)
            XCTAssertTrue(success)
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
            
        }
    }
    
    public func testRestore() throws {
        let testData: TestData = TestData()
        try testData.setupStore(false)
        let store: DIDStore = try DIDStore.shareInstance()!
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
