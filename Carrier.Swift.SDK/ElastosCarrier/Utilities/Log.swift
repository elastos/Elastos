import Foundation

internal class Log {
    private static var logLevel = CarrierLogLevel.Info

    internal static func setLevel(_ level: CarrierLogLevel) {
        logLevel = level
    }

    internal static func d(_ tag: String, _ format: String, _ args: CVarArg...) {
        if (logLevel >= CarrierLogLevel.Debug) {
            let log = String(format: " D/" + tag + ": " + format, arguments: args)
            NSLog(log)
        }
    }

    internal static func i(_ tag: String, _ format: String, _ args: CVarArg...) {
        if (logLevel >= CarrierLogLevel.Info) {
            let log = String(format: " I/" + tag + ": " + format, arguments: args)
            NSLog(log)
        }
    }

    internal static func w(_ tag: String, _ format: String, _ args: CVarArg...) {
        if (logLevel >= CarrierLogLevel.Warning) {
            let log = String(format: " W/" + tag + ": " + format, arguments: args)
            NSLog(log)
        }
    }

    internal static func e(_ tag: String, _ format: String, _ args: CVarArg...) {
        if (logLevel >= CarrierLogLevel.Error) {
            let log = String(format: " E/" + tag + ": " + format, arguments: args)
            NSLog(log)
        }
    }
}
