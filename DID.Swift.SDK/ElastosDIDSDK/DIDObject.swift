import Foundation

public class DIDObject {
    private var _id: DIDURL?
    private var _type: String?

    init() {}

    init(_ id: DIDURL, _ type: String) {
        self._id = id
        self._type = type
    }

    public func getId() -> DIDURL {
        return _id!
    }

    func setId(_ id: DIDURL) {
        self._id = id
    }

    public func getType() -> String {
        return _type!
    }

    func setType(_ type: String) {
        self._type = type
    }

    func isDefType() -> Bool {
        return _type == Constants.DEFAULT_PUBLICKEY_TYPE
    }

    func equalsTo(_ other: DIDObject) -> Bool {
        return getId() == other.getId() &&
               getType() == other.getType()
    }
}
