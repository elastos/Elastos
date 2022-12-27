import React from 'react'
import Trinity from './trinity.png'
import App from './app.png'
import Wallet from './wallet.png'
import Wallet2 from './wallet2.png'
import Wallet3 from './wallet3.png'
import Ditto from './ditto.png'
import Ditto2 from './ditto2.png'
import Ditto3 from './ditto3.png'

import '../../style.scss'

export default class extends React.Component {

  render () {
    return (
      <div className="p_developerLearnDetail">
        <h3>Elastos E2E Demo App: The Tech</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/lSVyhhkqAP4" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
        <p>
                    The first ever Elastos End to End Demo that was showcased at ___ might not look very pretty from the outside but when you dig deeper into the tech side of things, there’s a lot going on and we’re going to dissect all of the technology used in this demo.
                    From the outside, the demo just looks like a regular mobile app and an end user doesn’t even know how the communication is being done underneath nor how the blockchain is being used to tie everything together.
                    This is how it should be.
                    A regular end consumer only cares about the app they’re using.
                    They want the interface to be as friendly and as intuitive as possible and Elastos plans to do exactly that by letting developers create a user friendly blockchain based app.
                    It’s all about mass adoption at the end of the day.
        </p>

        <h5>Elastos Browser (Codename: Trinity Browser)</h5>
        <a href="https://github.com/elastos/Elastos.Trinity" target="_blank">Repo: https://github.com/elastos/Elastos.Trinity</a>
        <img src={Trinity} />
        <p>
                    Elastos Browser can currently only be built for Android devices but it’ll be supported in other platforms in the future such as iOS, mac, windows, etc.
                    Trinity is a project name that aims to implement a cross-platform application that can run on Android, iOS, Windows, Mac, etc.
                    It is the entrance from the classical internet into the elastos internet.
        </p>
        <p>
                    This is a regular mobile app that you can download from app store or directly install it onto your phone.
                    This app is used to load and run DApps written using Cordova/Ionic framework.
                    In this app, the complete Elastos Runtime feature is implemented which realizes sandbox isolation between the host system and Elastos Runtime, and also sandbox isolation between DApps which can provide a safe and reliable environment to run on.
                    Digital assets and property rights are protected.
        </p>
        <img src={App} />
        <p>
                    On this browser, the open source Chromium project is used as the technical framework for rendering and running and then the Elastos framework, sandbox mechanism, access rights management, disabling of traditional internet communication methods(eg http/https) and Component Assembly Runtime(CAR) components are added.
                    It also provides other functions to implement a standard elastos runtime where the HTML5 Elastos DApp is run.
                    Some of the features of Trinity browser are as follows:
        </p>
        <ol>
          <li>Virtual File System: Each app has an encrypted virtual file system built in. All DApp and DApp file data is stored in this virtual file system. This ensures that data cannot be accessed externally.</li>
          <li>Isolation between DApps: In the virtual file system, the access path permissions are set according to the ID of the DApp and the mechanism for setting file access rights is dependent on the user.</li>
          <li>Runtime isolation: It supports a separate process for each application, leveraging the natural isolation mechanism of the process along with access control.</li>
          <li>Disabling of http/https protocols: The browser completely disables http/https protocols so applications are not allowed to send any network packets out to the internet and instead will need to pass on their request to elastos carrier</li>
        </ol>
        <h5>Dittobox (Personal Cloud Drive)</h5>
        <a href="https://github.com/elastos/Elastos.DittoBox.Server" target="_blank">Dittobox Server Repo: https://github.com/elastos/Elastos.DittoBox.Server</a>
        <br />
        <a href="https://github.com/elastos/Elastos.DittoBox.Android" target="_blank">Dittobox Android Repo: https://github.com/elastos/Elastos.DittoBox.Android</a>
        <img src={Ditto} />
        <p>
                    Dittobox can be thought of as your own personal cloud drive that can be used to store your personal files, digital assets, etc that can later be accessed via Elastos Carrier via a mobile/desktop app.
                    In the end to end demo, a dittobox server could be run in a docker container from your own computer however, you can also run it directly on a raspberry pi computer that sits behind your router.
                    And then using elastos carrier, you’re able to access all the files from this dittobox server from anywhere in the world.
        </p>
        <img src={Ditto2} />
        <p>
                    Elastos Carrier doesn’t provide any account and password functionality, only access authentication.
                    In other words, when accessing a deployed personal cloud service through a mobile phone, the relationship between nodes has to be established through Carrier API.
                    The mobile phone and the cloud drive service have to be paired through a “pairing code”.
                    It is set by the user while deploying the service.
                    Once pairing is complete, it is now possible to access the personal cloud drive from the client side.
                    The user then has to provide the correct cloud drive login info to actually access the drive.
                    The authentication is therefore two fold. First is the pairing authentication and the cloud driver user account authentication.
        </p>
        <img src={Ditto3} />
        <p>
                    The Dittobox feature on the end to end demo demonstrates that all traditional http/https based application can be refactored to elastos carrier apps running over carrier network.
                    Being an elastos carrier web app, the app server can be deplolyed without requirement of direct network accessibility.
                    For example, through elastos carrier network, you can deploy dittobox server at local network at your home and access the service from anywhere in the world.
        </p>
        <a href="https://github.com/elastos/Elastos.RT.SDK.Wallet.Android" target="_blank">Wallet SDK Repo: https://github.com/elastos/Elastos.RT.SDK.Wallet.Android</a>
        <br />
        <a href="https://github.com/elastos/Elastos.ORG.Wallet.Mobile" target="_blank">Elastos Mobile Wallet Repo: https://github.com/elastos/Elastos.ORG.Wallet.Mobile</a>
        <img src={Wallet} />
        <p>
                    The end to end demo also contains an in-built wallet that is able to store the mainchain currency, ELA, and also able to transfer ELA to ID sidechain that is used to power different services and apps within the elastos smartweb.
                    You can also create a new wallet or import from an existing wallet. Note that this is not connected to the main elastos blockchain and is rather connected to the test blockchain network.
                    The Wallet component encapsulates elastos SPV that can be used to create wallet objects, create and send transactions, and more.
        </p>
        <img src={Wallet2} />
        <p>
                    This is a pretty early version of what kind of wallet you can build on Elastos.
                    First of all, it contains a way to store your ELA.
                    Secondly, there’s an additional functionality called “IdChain” and this also stores ELA.
                    If you haven’t checked out the documentation on ID Sidechain, please do so at
          {' '}
          <a href="https://medium.com/@kiran.pachhai/elastos-did-the-backbone-of-the-new-internet-26182108bf6a" target="_blank">https://medium.com/@kiran.pachhai/elastos-did-the-backbone-of-the-new-internet-26182108bf6a</a>
.
                    The issuance of IDs is free using ID Sidechain on the Elastos internet however the transactions are not free. For any transactions associated with the ID sidechain, this is merged mined with Elastos main chain and hence requires some transaction fees in ELA to be put into the chain.
                    Applications and services will be using ID sidechain so whenever they require some ELA payment that is needed by the ID sidechain, the elastos wallet automatically reduces the appropriate ELA amount from this “IdChain” sidechain wallet.
                    This is the main functionality of having a sub-wallet for ID chain.
        </p>
        <img src={Wallet3} />
        <p>
                    In the future, the mobile wallet may very well be able to store not just ELA but also additional sidechain tokens in the same wallet.
                    This might include all the Elastos sidechain tokens as well.
        </p>
        <h5>Elastos Runtime</h5>
        <p>
                    Each app in Trinity browser is run inside its own sandboxed environment and this sandboxed environment is Elastos Runtime.
                    Elastos Runtime can be thought of as an App Engine or a Virtual Machine.
                    It runs on top of existing OS such as android, ios, linux, etc and provides SDK for DApps to be running on top of it.
                    Elastos Runtime is a development framework and the component library provided by this framework is implemented using Component Assembly Runtime(CAR).
        </p>
        <p>
                    CAR components provide service using interface and component interface needs the metadata to describe the component so that other users can knwo how to use the service.
                    Metadata describes the relationship between services and calls.
                    With this description, calling between different components becomes possible, and members of the long-range, inter-process communication can be properly carried out.
                    The major problems solved by CAR component technology are: component from different sources can inter-operate, components upgrade but affect no other component, component is independent of the programming language and component operating environment is transparent.
                    By doing this, you’re able to use a CAR component to establish communication between a program written in C/C++ and one in HTML5/JS — being able to talk without confirming to one programming language.
        </p>
        <h5>Elastos Carrier</h5>
        <p>
                    On Trinity browser, http/https protocols are completely disabled and because of this, applications have no access to the internet at all.
                    So, in order to access the outside world, apps have to talk to elastos carrier and carrier takes over all network traffic on application’s behalf.
                    Elastos Carrier is a completely decentralized peer to peer network service platform.
                    Some of the features of Elastos Carrier are:
        </p>
        <ol>
          <li>
                        On Elastos, apps do not have direct access to the internet.
                        So, Elastos carrier is used to solve the issue when an app needs to communicate to the outside world.
                        Usually, solving this problem requires deploying a central server for data transfer.
                        But, Elastos Carrier implements a centerless direct connection communication scheme based on P2P communication technology.
          </li>
          <li>
                        Elastos Carrier provides cross-network access capabilities.
                        For example, any two app nodes can be in different subnets.
                        One might be at a home wifi environment and another might be at a corporate wifi environment.
                        Using Elastos Carrier, apps can communicate with each other directly using an “address or DID” string of each other by first confirming the authorization by adding each other as “friends”.
                        Everything on Elastos is blacklisted by default. In order for any two devices or any two users or any two apps to talk to each other, they have to be added to each other’s friends list first.
          </li>
        </ol>
        <p>
                    Elastos Carrier is a Friend to Friend basic communication network built on DHT(Distributed Hash Table) decentralization and distributed network technology.
                    The “Peer to Peer” in Carrier means node-to-node.
                    The bootstrap nodes that connect to the network via fixed IP addresses is the basic network architecture used by Carrier for networking.
                    They help smart devices installed with the Carrier SDK to connect to Carrier network.
                    These smart devices then become peer nodes and aggregate across the P2P network.
                    Bootstrap Nodes provide relay functions to peer nodes, but do not function at the application level.
                    This means that no friend relationship exists between these two types of nodes.
                    Peer nodes on the other hand, participate in application-level functions and data/message exchange between them requires a friend relationship to be established.
                    Since Carrier network architecture is friend-to-friend based, as long as no friend relationship is established, no direct communication between nodes is possible, even if they are on the same DHT network, making it harder for hackers to launch DDOS atacks.
                    This improves the security factor in Carrier network.
        </p>
        <p>
                    It’s because of this reason that Elastos Carrier can be integrated with any existing devices or servers such as a dittobox server that may reside behind the router but the applications can still connect to this dittobox server via carrier from anywhere in the world.
                    This means that you can host your own microwebsites for free using elastos carrier network and share files among friends completely peer to peer so there are no big corporations storing your data.
                    In other words, you truly own your own data and you can do whatever you want with it.
                    However, in cases where you may want to serve your website to millions of customers, your home bandwidth may not be enough so in such cases, you will need to pay some ELA to pay for services such as IPFS on the elastos network.
                    The IPFS part is still being actively developed so it’s not part of the end to end demo however it should be available in the near future.
        </p>
        <h3>Build The App Yourself</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/8cp_K4TF3ZM" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
      </div>
    )
  }
}
