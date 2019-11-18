

import XCTest
import ElastosDIDSDK

class DIDStoreCreateTests: XCTestCase {
    var adapter: SPVAdaptor!

    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    func test00CreateEmptyStore0() {
        TestUtils.deleteFile(storePath)
        do {
            let cblock: PasswordCallback = ({(walletDir, walletId) -> String in return "helloworld"})
            adapter = SPVAdaptor(walletDir, walletId, networkConfig, resolver, cblock)
            try DIDStore.creatInstance("filesystem", location: storePath, storepass: storePath, adapter)
            let tempStore: DIDStore = try DIDStore.shareInstance()!
            XCTAssertFalse(try! tempStore.hasPrivateIdentity())
            XCTAssertTrue(TestUtils.exists(storePath))
            let path: String = storePath + "/" + ".DIDStore"
            let filemanage: FileManager = FileManager.default
            filemanage.fileExists(atPath: path)
        } catch {
            print("test00CreateEmptyStore0 error: \(error)")
        }
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

}
