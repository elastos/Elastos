import Foundation

public class DIDObject: NSObject {
    public var id: DIDURL!
    public var type: String!

    override init() {
        super.init()
    }
    
    init(_ id: DIDURL, _ type: String) {
        self.id = id
        self.type = type
    }
}
