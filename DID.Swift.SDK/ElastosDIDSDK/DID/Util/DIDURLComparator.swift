import Foundation

class DIDURLComparator {
    
    // TODO DIDURL 判断比较
    class func DIDURLComparator(_ a: DIDURL, _ b: DIDURL) -> Bool {
        let aToken = a.toExternalForm()
        let bToken = b.toExternalForm()
        return aToken.caseInsensitiveCompare(bToken) == ComparisonResult.orderedAscending
    }
}

