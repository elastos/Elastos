
internal extension String {

    internal static func toHardString(_ str: String?) -> String {
        if (str == nil) {
            return "<nil>"
        } else if (str == "") {
            return "<empty>"
        } else {
            return str!
        }
    }

    internal init<T>(cCharPointer pointer: UnsafePointer<T>) {
        self.init(cString: UnsafeRawPointer(pointer).assumingMemoryBound(to: CChar.self))
    }

    @inline(__always) func writeToCCharPointer<T>(_ pointer : UnsafeMutablePointer<T>) {
        self.withCString({ src in
            let ptr = UnsafeMutableRawPointer(pointer).assumingMemoryBound(to: CChar.self)
            strcpy(ptr, src)
        })
    }
}

@inline(__always) internal func createCStringDuplicate(_ field: String?) -> UnsafePointer<CChar>? {
    if field != nil {
        return UnsafePointer<CChar>(field!.withCString {
            return strdup($0)
        })
    } else {
        return nil
    }
}

@inline(__always) internal func deallocCString(_ field: UnsafePointer<CChar>?) {
    if (field != nil) {
        free(UnsafeMutablePointer<CChar>(mutating: field))
    }
}
