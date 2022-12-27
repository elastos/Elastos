def import_pods
  pod 'PromiseKit'
  pod 'BlueRSA', '~> 1.0'
 # pod 'BlueECC', '~> 1.1'
  pod 'LoggerAPI', '~> 1.7'
  pod 'KituraContracts', '~> 1.1'
  pod 'BlueCryptor', '~> 1.0'
end

target :ElastosDIDSDK do
  platform :ios, '10.10'
  use_frameworks!
  import_pods
  target 'ElastosDIDSDKTests' do
    inherit! :search_paths
    import_pods
  end
end

