
import Foundation

public enum TestError: Error {
    case failue(_ des: String?)
}

extension TestError {
    static func des(_ error: TestError) -> String {
        switch error {
        case .failue(let err):
            return err ?? "Operation failed"
        }
    }
}
