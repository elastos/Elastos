import Foundation

class DIDURLComparator {
    
    // TODO DIDURL 判断比较
    class func DIDURLComparator(_ a: DIDURL, _ b: DIDURL) -> Bool {
        let aToken = a.toExternalForm()
        let bToken = b.toExternalForm()
        return aToken.caseInsensitiveCompare(bToken) == ComparisonResult.orderedAscending
    }
    
    // TODO OrderDictionary 排序
    class func DIDOrderedDictionaryComparator(_ source: OrderedDictionary<DIDURL, DIDPublicKey>) -> OrderedDictionary<DIDURL, DIDPublicKey> {
        
        var sortArray: Array<(DIDURL, DIDPublicKey)> = Array()
        for (key, value) in source {
            sortArray.append((key, value))
        }
        
        sortArray.sort { (a, b) -> Bool in
            let urlA: DIDURL = a.0
            let urlB: DIDURL = b.0
            return DIDURLComparator(urlA, urlB)
        }
        
        var result: OrderedDictionary<DIDURL, DIDPublicKey> = OrderedDictionary<DIDURL, DIDPublicKey>()
        for obj in sortArray {
            result[obj.0] = obj.1
        }

        return result
    }
    
    class func DIDOrderedDictionaryComparatorByVerifiableCredential(_ source: OrderedDictionary<DIDURL, VerifiableCredential>) -> OrderedDictionary<DIDURL, VerifiableCredential> {
        
        var sortArray: Array<(DIDURL, VerifiableCredential)> = Array()
        for (key, value) in source {
            sortArray.append((key, value))
        }
        
        sortArray.sort { (a, b) -> Bool in
            let urlA: DIDURL = a.0
            let urlB: DIDURL = b.0
            return DIDURLComparator(urlA, urlB)
        }
        
        var result: OrderedDictionary<DIDURL, VerifiableCredential> = OrderedDictionary<DIDURL, VerifiableCredential>()
        for obj in sortArray {
            result[obj.0] = obj.1
        }

        return result
    }
}
