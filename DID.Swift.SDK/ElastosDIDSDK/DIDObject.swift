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
        return self._id!
    }

    func setId(_ id: DIDURL) {
        self._id = id
    }

    public func getType() -> String {
        return self._type!
    }

    func setType(_ type: String) {
        self._type = type
    }

    func isDefType() -> Bool {
        return self._type == Constants.DEFAULT_PUBLICKEY_TYPE
    }

    func equalsTo(_ other: DIDObject) -> Bool {
        return self.getId() == other.getId() &&
               self.getType() == other.getType()
    }
}
