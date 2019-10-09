import Foundation


public class EmbeddedCredential: VerifiableCredential {
    
   override init() {
    super.init()
    }
    
    override init(_ vc: VerifiableCredential) {
        super.init(vc)
    }
    
//    class func fromJson(_ node: Any, _ ref: DID) -> VerifiableCredential {
//
//        return EmbeddedCredential(VerifiableCredential)
//    }
    
}
