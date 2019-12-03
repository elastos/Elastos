
import XCTest
import ElastosDIDSDK

class BulkCreate: XCTestCase {
    
    var adapter: SPVAdaptor!
    var timeout: Double = 1800.0
    
    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
        let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "test111111"})
        adapter = SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock)
        TestUtils.deleteFile(storePath)
        try! DIDStore.creatInstance("filesystem", storePath, adapter)
    }
    
    func test30PrepareForRestore() {
        do {
            let store: DIDStore = try DIDStore.shareInstance()!
            let mnemonic: String = HDKey.generateMnemonic(0)
            try store.initPrivateIdentity(0, mnemonic, passphrase, storePass, true)
            for i in 0...10 {
                let lock = XCTestExpectation(description: "******** Waiting for wallet available")
                isAvailable(lock)
                wait(for: [lock], timeout: timeout)
            
                let hint: String = "my did \(i)"
                let doc: DIDDocument = try store.newDid(storePass, hint)
                let path: String = storePath + "/ids/" + doc.subject!.methodSpecificId + "/document"
                XCTAssertTrue(TestUtils.existsFile(path))

                let path2: String = storePath + "/ids/." + doc.subject!.methodSpecificId + ".meta"
                XCTAssertTrue(TestUtils.existsFile(path2))

                _ = try store.publishDid(doc, storePass)
                print("******** Waiting for wallet available")

                let lock2 = XCTestExpectation(description: "******** Waiting for wallet available")
                wait(for: [lock2], timeout: timeout)
                let d: DIDDocument = try store.resolveDid(doc.subject!, true)!
            }
        } catch {
            print(error)
        }
    }
    
    func isAvailable(_ lock: XCTestExpectation) {
        do {
            let rc: Bool = try adapter.isAvailable()
            if rc == false {
                isAvailable(lock)
            }
            else {
                lock.fulfill()
            }
        } catch {
            print(error)
        }
    }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func testExample() {
        // This is an example of a functional test case.
        // Use XCTAssert and related functions to verify your tests produce the correct results.
    }

    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measure {
            // Put the code you want to measure the time of here.
        }
    }

}
