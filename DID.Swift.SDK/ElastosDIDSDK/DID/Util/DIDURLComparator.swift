import Foundation

class DIDURLComparator {
    
    // TODO string 判断比较
    class func DIDStringComparator(_ a: String, _ b: String) -> Bool {
        return a.caseInsensitiveCompare(b) == ComparisonResult.orderedAscending
    }
    
    class func DIDOrderedDictionaryComparatorByKey(_ source: OrderedDictionary<String, Any>) -> OrderedDictionary<String, Any> {
        
        var sortArray: Array<(String, Any)> = Array()
        var did: String = ""
        for (key, value) in source {
            if key == Constants.id {
                did = String("\(value)")
            }
            else {
                sortArray.append((key, value))
            }
        }
        
        sortArray.sort { (a, b) -> Bool in
            let strA: String = a.0
            let strB: String = b.0
            return DIDStringComparator(strA, strB)
        }
        
        var result: OrderedDictionary<String, Any> = OrderedDictionary<String, Any>()
        if did != "" {
            result[Constants.id] = did
        }
        for obj in sortArray {
            result[obj.0] = obj.1
        }
        return result
    }
    
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
    
    class func DIDOrderedDictionaryComparatorByService(_ source: OrderedDictionary<DIDURL, Service>) -> OrderedDictionary<DIDURL, Service> {
        
        var sortArray: Array<(DIDURL, Service)> = Array()
        for (key, value) in source {
            sortArray.append((key, value))
        }
        
        sortArray.sort { (a, b) -> Bool in
            let urlA: DIDURL = a.0
            let urlB: DIDURL = b.0
            return DIDURLComparator(urlA, urlB)
        }
        
        var result: OrderedDictionary<DIDURL, Service> = OrderedDictionary<DIDURL, Service>()
        for obj in sortArray {
            result[obj.0] = obj.1
        }

        return result
    }

}
