import Foundation

public protocol DIDAdapter {
    func createIdTransaction(_ payload: String,
                             _ memo: String?,
                             _ confirms: Int,
                             _ callback: (_ transctionId: String, _ status: Int, _ message: String?) -> Void )
}
