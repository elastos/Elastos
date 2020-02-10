import Foundation

extension String {
    var base64EncodedString: String {
        return Data(self.utf8).base64EncodedString()
    }

    var base64DecodedString: String? {
        guard let data = Data(base64Encoded: self) else {
            return nil
        }
        return String(data: data, encoding: .utf8)
    }
}
