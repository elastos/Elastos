import React from 'react'
import '../../style.scss'

export default class extends React.Component {

  render () {
    return (
      <div className="p_developerLearnDetail">
        <h3>Overview</h3>
        <p>
                    Elastos Runtime can be thought of as an App Engine or a Virtual Machine(VM).
                    It is a runtime sandboxed closed environment that runs on top of existing OS, such as Android, iOS, Linux, etc.
                    It provides SDK for DApps to be running on top of RT. As for the DApp developers, they do not need to worry too much about the technical details of the layer underneath(blockchain layer).
                    They just need to call the RT.SDK. Building DApps will be much easier than before.
                    DApp developers will be using Cordova to develop their HTML5 applications that will be running on trinity browser. Non-Elastos apps can access the Elastos Smart Web via the RT SDK(C++ SDK) because android and iOS apps can call C++ SDK.
                    This is done to make it easier for existing mobile developers to integrate their existing mobile apps with Elastos.
                    Elastos Runtime runs on the user’s equipment to achieve a “reliable runtime environment.”
                    By developing Elastos DApp, independent developers can use (play) digital assets such as digital audio and video playback.
                    VM guarantees digital assets will run under blockchain control, providing users with the ability to consume and invest in digital content.
        </p>
        <h3>Two fundamental reasons why we need a runtime environment like Elastos Runtime</h3>
        <ol>
          <li>
                        Let’s first talk about how Java apps work on your windows or how your android apps work on your phone.
                        Every Java code runs inside a Java Virtual Machine hence, in any device that has Java installed, you can run Java apps easily and this Java Virtual Machine(JVM) acts like a sandbox.
                        However, this is not completely isolated at all. The problem arises when you are developing a game and you need a game engine or a multimedia codec or if you need maximum security.
                        This is where JVM falls short. Because, even if the actual Java code runs inside the Java Virtual Machine, in order to talk to the underlying hardware to utilize the resources the hardware provides, the code needs to talk to the kernel first because kernel is the interface that can talk to the hardware.
                        With that said, you cannot talk to the kernel directly from within this so called “sandbox” in Java Virtual machine. It has to use a bridge called Java Native Interface(JNI).
                        This is the main problem with Java. In order to do this, the code execution has to completely leave the VM first using the JNI which then helps you interact with the hardware.
                        This is the main reason that leads to man in the middle attacks and many virus attacks are based on this vulnerability.
                        This is where Elastos VM(C++ VM) shines. If we run everything in elastos runtime, the code never has to leave this sandbox because you can directly interact with the kernel using C programming language without ever leaving the VM.
                        This prevents the man in the middle attacks and many other viruses.
          </li>
          <li>
                        Another reason Elastos decided to have a C based VM has to do with the fact that you can run this VM in any devices out there.
                        This opens the door to couple of things: being able to run elastos runtime on top of any existing OS(like windows, ios, android, etc) and also being able to develop any kind of decentralized apps(DApps) as these apps have the full resources of the underlying hardware at their disposal.
                        At the moment, you’re unable to build any resource intensive apps(such as gives, movies apps, etc) using blockchain via ethereum or NEO or any other platforms that are available right now because all of their DApps will run using their main chain nodes where they’re only used for running smart contracts and nothing else.
                        So, with elastos, you’re free to make any sort of apps and they will scale very easily as these DApps run directly on the user’s device.
          </li>
        </ol>
      </div>
    )
  }
}
