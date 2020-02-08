import Foundation

class DIDURLComparator {
    
    // string comparator
    class func DIDStringComparator(_ a: String, _ b: String) -> Bool {
        return a.compare(b) == ComparisonResult.orderedAscending
    }
    
    class func DIDOrderedDictionaryComparatorByKey(_ source: OrderedDictionary<String, Any>) -> OrderedDictionary<String, Any> {
        
        var sortArray: Array<(String, Any)> = Array()
        var did: String = ""
        for (key, value) in source {
            if key == ID {
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
            result[ID] = did
        }
        for obj in sortArray {
            if obj.1 is Dictionary<String, Any> {
                result[obj.0] = DIDOrderedDictionaryComparatorByKey(source: obj.1 as! Dictionary<String, Any>)
            } else {
                result[obj.0] = obj.1
            }
        }
        return result
    }
    
    class func DIDOrderedDictionaryComparatorByKey(source: Dictionary<String, Any>) -> OrderedDictionary<String, Any> {
        
        var sortArray: Array<(String, Any)> = Array()
        var did: String = ""
        for (key, value) in source {
            if key == ID {
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
            result[ID] = did
        }
        for obj in sortArray {
            if obj.1 is Dictionary<String, Any> {
                result[obj.0] = DIDOrderedDictionaryComparatorByKey(source: obj.1 as! Dictionary<String, Any>)
            } else if obj.1 is [Any] {
                let array = obj.1 as! [Any]
                var temp = [Any]()
                for ob in array {
                    if ob is Dictionary<String, Any> {
                        temp.append(DIDOrderedDictionaryComparatorByKey(source: ob as! Dictionary<String, Any>))
                    } else {
                        temp.append(ob)
                    }
                }
                result[obj.0] = temp
            }
            else {
                result[obj.0] = obj.1
            }
        }
        return result
    }
    
    // DIDURL comparator
    class func DIDURLComparator(_ a: DIDURL, _ b: DIDURL) -> Bool {
        let aToken = a.toExternalForm()
        let bToken = b.toExternalForm()
        return aToken.compare(bToken) == ComparisonResult.orderedAscending
    }
    
    // OrderDictionary order
    class func DIDOrderedDictionaryComparator(_ source: Dictionary<DIDURL, DIDPublicKey>) -> OrderedDictionary<DIDURL, DIDPublicKey> {
        
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
    
    class func DIDOrderedDictionaryComparatorByVerifiableCredential(_ source: Dictionary<DIDURL, VerifiableCredential>) -> OrderedDictionary<DIDURL, VerifiableCredential> {
        
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
    
    class func DIDOrderedDictionaryComparatorByService(_ source: Dictionary<DIDURL, Service>) -> OrderedDictionary<DIDURL, Service> {
        
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
