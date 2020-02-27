import Foundation

class IDGetter {
    private let id: DIDURL
    private let refDid: DID?

    init(_ id: DIDURL, _ ref: DID?) {
        self.id = id
        self.refDid = ref
    }

    func value(_ normalized: Bool) -> String {
        let value: String

        if normalized || refDid == nil || refDid != id.did {
            value = id.toString()
        } else {
            value = "#" + id.fragment!
        }
        return value
    }
}
