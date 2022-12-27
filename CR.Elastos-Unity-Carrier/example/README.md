# Example for carrier plugin

## Run unit test for javascript side
```
npm test
```

## Run autotest with appium and webdriver (iOS)
* Change config/wdio.ios.app.conf.js
```
  deviceName: 'iPhone XR', 
  platformName: 'iOS',    
  platformVersion: '12.1',

  // The path to the app
  app: join(process.cwd(), './apps/example.app'),
```
* Run **react-native run-ios**
* Build example.app with xcode and replace the ./apps/example.app with build result.
* Run **npm run autotest:ios**
<img src="./autotest.png" width="auto" />

## Run autotest with appium and webdriver (Android)
* Change config/wdio.android.app.conf.js
```
  deviceName: 'Genymotion Samsung',
  platformName: 'Android',
  platformVersion: '8.0',
  app: join(process.cwd(), './apps/app-debug.apk'),
```
* Run **react-native run-android**
* Build app.apk with gradle and replace the ./apps/app-debug.apk with build result.
* Run **npm run autotest:android**

## Vedio links
* [https://youtu.be/8N99PZEOIm0]