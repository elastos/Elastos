require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name         = "Carrier"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.description  = <<-DESC
                  Carrier
                   DESC
  s.homepage     = "https://github.com/author/RNElastosUnityCarrier"
  s.license      = "MIT"
  # s.license    = { :type => "MIT", :file => "FILE_LICENSE" }
  s.author       = { "author" => "author@domain.cn" }
  s.platform     = :ios, "7.0"
  s.source       = { :git => "https://github.com/author/RNElastosUnityCarrier.git", :tag => "#{s.version}" }

  s.source_files = "ios/Carrier/*.{h,m}", "ios/Carrier/ElastosCarrierSDK.framework/Headers/*.h"
  s.requires_arc = true
  s.ios.vendored_framework = "ios/Carrier/ElastosCarrierSDK.framework"
  s.ios.public_header_files = 'ios/Carrier/ElastosCarrierSDK.framework/Headers/*.h'

  s.dependency "React"
  #s.dependency "others"
end

