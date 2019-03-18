#
#  Be sure to run `pod spec lint carrier.podspec.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see https://docs.cocoapods.org/specification.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |s|

  s.name         = "carrier"
  s.version      = "5.2.2"
  s.summary      = "Elastos Carrier iOS SDK Distribution."
  s.swift_version = '4.0'
  s.description  = 'Elastos carrier ios sdk framework distribution.'
  s.homepage     = "https://www.elastos.org"
  s.license      = { :type => "MIT", :file => "LICENSE" }
  s.author             = { "carrier-dev" => "release@elastos.org" }
  s.platform     = :ios, "9.0"
  s.ios.deployment_target = "9.0"
  s.source       = { :git => "https://github.com/elastos/Elastos.NET.Carrier.iOS.SDK.git", :tag => s.version  }
  s.vendored_frameworks = 'Frameworks/*.framework'
  s.source_files = 'Frameworks/ElastosCarrier.framework/**/*.h'
  s.preserve_paths = 'CocoaPods/*'


end
