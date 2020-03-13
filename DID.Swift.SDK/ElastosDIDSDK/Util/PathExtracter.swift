import Foundation

class PathExtracter {
    private let path: String

    init(_ path: String) {
        self.path = path
    }

    /*
     * example: /foo/bar/example.txt
     * dirNamePart() -> "/foo/bar/"
     */
    func dirname() -> String {
        let index = path.range(of: "/", options: .backwards)?.lowerBound
        let str = index.map(path.prefix(upTo:)) ?? ""
        return "\(str)/"
    }

    /*
     * example: /foo/bar/example.txt
     * baseNamePart() -> "exmaple.txt"
     */
    func basename() -> String {
        let arr = path.components(separatedBy: "/")
        let str = arr.last ?? ""
        return String(str)
    }
}
