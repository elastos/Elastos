
import Foundation

public class DateFormater {
    
    public class func parseDate(_ timestamp: String) -> Date? {
        let formatter = Foundation.DateFormatter()
        formatter.dateFormat = DATE_FORMAT
        formatter.timeZone = TimeZone.init(secondsFromGMT: 0)
        let date = formatter.date(from: timestamp)
        
        return date
    }
    
    public class func format(_ date: Date) -> String{
        let formatter = Foundation.DateFormatter()
        formatter.dateFormat = DATE_FORMAT
        formatter.timeZone = TimeZone.init(secondsFromGMT: 0)
        let localDate = formatter.string(from: date)
        return localDate
    }
    
    public class func currentDateToWantDate(_ year: Int)-> Date {
        let current = Date()
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone.current
        var comps:DateComponents?
        
        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: current)
        comps?.year = MAX_VALID_YEARS
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0
        let realDate = calendar.date(byAdding: comps!, to: current) ?? Date()
        let hour = calendar.component(.hour, from: realDate)
        let useDate = calendar.date(bySettingHour: hour, minute: 00, second: 00, of: realDate) ?? Date()
        
        return useDate
    }
    
    public class func dateToWantDate(_ date: Date, _ year: Int)-> Date {
        
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone.current
        var comps:DateComponents?
        
        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: date)
        comps?.year = MAX_VALID_YEARS
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0
        let realDate = calendar.date(byAdding: comps!, to: date) ?? Date()
        let hour = calendar.component(.hour, from: realDate)
        let useDate = calendar.date(bySettingHour: hour, minute: 00, second: 00, of: realDate) ?? Date()
        
        return useDate
    }
    
   public class func currentDate()-> Date {
        let current = Date()
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone(abbreviation: "UTC")!
        var comps:DateComponents?
        
        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: current)
        comps?.year = 0
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0
        let realDate = calendar.date(byAdding: comps!, to: current) ?? Date()
        
        return realDate
    }
    
  public class func setExpires(_ expire: Date) -> Date {
        
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone(abbreviation: "UTC")!
        let hour = calendar.component(.hour, from: expire)
        let date = calendar.date(bySettingHour: hour, minute: 00, second: 00, of: expire) ?? Date()
        
        return date
    }
    
   public class func comporsDate(_ expireDate: Date, _ defaultDate: Date) -> Bool {
        return  defaultDate > expireDate
    }
}
