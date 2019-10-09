import Foundation

public class DIDObject: NSObject {
     var id: DIDURL!
     var type: String!

    override init() {
        
    }
    
    init(_ id: DIDURL, _ type: String) {
        self.id = id
        self.type = type
    }
}
