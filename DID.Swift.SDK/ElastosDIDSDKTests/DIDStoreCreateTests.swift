

import XCTest
import ElastosDIDSDK

class DIDStoreCreateTests: XCTestCase {
    
    let storePath: String = "\(NSHomeDirectory())/Library/Caches/DIDStore"
    let passphrase: String = "secret"
    
    override func setUp() {
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    func test00CreateEmptyStore0() {
        TestUtils.deleteFile(storePath)
        do {
            try DIDStore.creatInstance("filesystem", location: storePath, passphase: passphrase)
            let tempStore: DIDStore = DIDStore.shareInstance()!
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
