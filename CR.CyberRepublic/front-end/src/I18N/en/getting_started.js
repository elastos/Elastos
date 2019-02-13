export default {
  part1: `<h3>What is Elastos?</h3>
            <p>Elastos is the safe and reliable internet of the future. Built utilizing the blockchain, this technological breakthrough provides the first completely safe environment on the web where decentralized applications are detached from the internet while also permitting full scalability to billions of users. Elastos enables the generation of wealth through ownership and exchange of your data and digital assets.</p>
            <p>Elastos is not a blockchain project but rather a network operating system project powered by blockchain technology so in that sense, Elastos is not directly competing with any other blockchain projects. It can work together with them to form this new ecosystem where the decentralized applications run directly on the device instead of running on the blockchain along with decentralized peer to peer network to transfer assets in a completey closed sandboxed environment, thereby solving the three pillar issues that are prevelant in the internet of today - security, scalibility and decentralization.</p>
            <p>Elastos uses internet as the base-layer infrastructure rather than an application where decentralized applications are forced to never directly connect to the internet and only interact with Elastos runtime(which in turn acts as a middle layer, then connecting to the internet). Within the elastos ecosystem, everything from devices to virtual machines to users to DApps have a DID(Decentralized ID) issued by the blockchain and everytime the application requests any sort of internet access, the ID has to first be verified using the blockchain which in turn eliminates many of the man-in-the-middle attacks, virus attacks, etc</p>
            <br />
            <a href="https://www.elastos.org/wp-content/uploads/2018/White%20Papers/elastos_whitepaper_en.pdf?_t=1526235330" target="_blank">Read Elastos Whitepaper</a>
            <br/>
            <a href="https://www.elastos.org/wp-content/uploads/2018/White Papers/elastos_sidechain_whitepaper_v0.3.0.8_EN.pdf?_t=1526918471" target="_blank">Read Elastos Mainchain - Sidechain Whitepaper</a>
            <br/>`,
  part2: `<h3>Brief Summary</h3>
                <ul>
                    <li>
                        <strong>Bitcoin</strong> = Trustworthy Ledger: Bitcoin introduced the power of decentralised ledger technology to the world,
                        showing how we don’t need financial institutions to transact value.
                        Bitcoin's purpose is a digital currency, with the intent to become electronic cash.
                        Bitcoin is great; however, it is extremely outdated.
                        Bitcoin has very slow transaction times, expensive transaction fees and uses Proof of Work mining which is vastly uneconomical.
                        Thanks to Bitcoin, the 2nd generation cryptocurrencies were born.
                    </li>
                    <li>
                        <strong>Ethereum</strong> = Trustworthy Ledger + Smart Contracts:
                        Ethereum, as well as many others, are second generation cryptocurrencies.
                        Ethereum was one of the first cryptocurrencies to introduce ‘smart contracts’ and the concept of decentralised apps.
                        Smart contracts put the trust of contracts in the trust of code. For example, If I bought a TV from an online merchant,
                        the payment would only clear once the TV had arrived and I was satisfied with it.
                        They can be coded to included things like ’14-day money back guarantee’ and every other element that is in a normal contract.
                        Drastically improving efficiency and breeds a new generation of trust.
                        Ethereum is a great project, however the scalability of smart contracts that are hosted on the Ethereum network is limited.
                        A game called ‘CryptoKitties’ caused massive network congestion and that is just one of many decentralised applications that use smart contracts.
                    </li>
                    <li>
                        <strong>Elastos</strong> = Trustworthy Ledger + Smart Contracts + Monetizable Dapps and Digital Assets:
                        Elastos is one of many new cryptocurrencies that are build on a third generation blockchain, focusing on a few main elements to combat the scalability issues that arise with 2nd generation cryptocurrencies:
                        <ol>
                            <li>Storage and Speed</li>
                            <li>Bugs within the smart contracts(security vulnerability)</li>
                            <li>Cost</li>
                            <li>Deletion of redundant data</li>
                            <li>Security</li>
                        </ol>
                        <p>
                            The issue with second generation smart contracts is that they have to run solely on the blockchain, which causes network congestion and high transaction costs.
                            Decentralized applications for Elastos are run by blockchain technology but can be built on current Operating Systems (IOS, Android and Linux).
                            Elastos understands the importance of ease of use for both the consumers and the producers.
                            The integration of multiple well known coding languages make development relativity easier than other blockchains.
                            They also support Android and Apple Operating Systems, which most other blockchains do not.
                        </p>
                        <p>
                            Elastos is not just the new internet, but the entire smart economy.
                            Focusing on digital assets, monetizing computing power and spare storage, financially incentive trade of digital assets all combined with the highest level of security and the removal of the middleman.
                            With speed, security and minimal cost at the heart of Elastos, it is bound to scale over time. Elastos is a third generation blockchain technology that tackles the issues we have with second generation blockchain projects such as Ethereum.
                        </p>
                    </li>
                </ul>
                <h3>The internet has nothing to do with computing. Computing has nothing to do with the internet</h3>
                <p>
                    Rong Chen often says that the internet has nothing to do with computing.
                    Computing has nothing to do with the internet.
                    He doesn't mean the network doesn't exist but rather there's a separation between computing from the network and communication.
                    Elastos proposed a long time ago that they should only focus on the computing logic, not which device the service was located at or which server it should connect to.
                    It's not what the application should be focusing on.
                </p>
                <p>
                    Because the entire system needs consistency, it is dangerous for the system if it is allowed to write directly to the location of a system.
                    For example, Elastos accesses the resource through the ID of the resource, not through specifying an IP address to access the computer.
                    The Elastos architecture includes P2P networks. P2P refers to the relationship between entities to entities, not physical computers to computers.
                    The ID of a P2P network represents an individual or an entity, not a computer. Today, we can use our ID to communicate with someone on our cellphones.
                    Tomorrow, even if I change a cellphone, we can still communicate with the same ID.
                    So, specific applications should not care about which cellphone you use or how you access it.
                </p>`,
  part3: `<h3>Brief Summary</h3>
                <ul>
                    <li>Through the merged mining with Bitcoin, the safety and reliability are ensured by strong Bitcoin computing power through Proof of Work. This mining strategy saves resources and avoids repeated consumption.</li>
                    <li>Because Elastos structure is built by having a main chain and sidechains, it avoids main chain being overloaded, and leads to easy routing and flexible extension, increasing the possibility for the Elastos to become basis for future internet applications.</li>
                    <li>The blockchain is a trust zone and a reliable internet database. Smart contracts are merely the saving process of this reliable database. Only the valuable data that needs to be notarized is recorded on the blockchain; only the data processing that needs computation reliability requires smart contracts.</li>
                </ul>`,
  part4: `<h3>Merged Mining</h3>
                <p>
                    The Elastos blockchain utilizes merged mining with Bitcoin, the process by which consensus is reached on both chains simultaneously.
                    In this case, the Bitcoin blockchain works as the parent blockchain to Elastos, with the Elastos chain as its auxiliary blockchain.
                    The mining pools will deploy merged mining code and miners will submit proof of work to both blockchains at the same time.
                    Energy consumption does not increase with merged mining, and will be equal to the energy consumed for mining either alone.
                    Through this mechanism, the Elastos blockchain has an extremely strong guarantee of computing power and will then be able to provide blockchain innovations at a global scale.
                    It makes full use of existing Bitcoin computing resources in addition to being environmentally friendly.
                </p>
                <p>
                    Elastos implements merged mining with Bitcoin.
                    The strategy of merged mining saves resources and avoids repeated consumption.
                    Miner submits Proof of Work(PoW) to both Bitcoin and Elastos and enjoys profits of mining competition without extra consumption of computing power.
                    The consensus mechanism for Elastos is AuxPoW+DPoS where the mining reward distribution is 70% for AuxPoW + DPoS(miners + stakers) and 30% for Ecosystem Application Reward(aka Cyber Republic Fund).
                    Tokens for apps built on Elastos can be published on sidechains.
                    These tokens may participate in two-way asset transfer across the main chain and sidechains.
                </p>
                Additional benefits of merged mining include:
                <ul>
                    <li>The transfer of trust over multiple chains. The Elastos main chain is merged mined along with the Bitcoin main chain. This merged mining characteristic can be extended to Elastos sidechains as long as the sidechain adopts the same Proof of Work consensus. Thus, layers of chains can be merged mined recursively, which establishes a hierarchy of trust among chains.</li>
                    <li>Isolated nodes. An auxiliary blockchain, or sidechain, dependent on merged mining does not need a consensus of multiple nodes. In the extreme case, one chain only needs one node and does not diminish the reliability of the ledger information on the main chain or other chains. No other blockchain consensus algorithm has this kind of advantage.</li>
                </ul>
                <a href="https://medium.com/@kiran.pachhai/elastos-did-the-backbone-of-the-new-internet-26182108bf6a" target="_blank">Read an article on ID Sidechain and the benefits of merged mining</a>`,

  part5: `<h3>Overview of Main chain and Sidechains in the elastos ecosystem</h3>
                <p>
                    One of the highlights of the design is that it makes use of a blend of mainchains and sidechains to improve the computational ability of blockchain technology.
                    The mainchain only executes simple transactions and payment transfers while the sidechain handles smart contracts to support services and applications.
                    With this, structure scalability and security can be accomplished.
                </p>
                <p>
                    Similar to the Operating System on a mobile device, users need a trustworthy location to store important data.
                    The Elastos blockchain works as this trust zone for the entire network Operating System.
                    The Elastos blockchain applies main and sidechain solutions to facilitate the smart economy and a healthy decentralized application environment.
                    This means that every application can create individual sidechains. The Elastos blockchain provides built-in, complete, easy-to-use sidechain support.
                    They will also be customizable, allowing clients to pick a different consensus method depending on the use case.
                </p>
                <p>
                    Any system built with blockchain technology has less computing power than a traditional computer, and so will not be able to meet the various requirements of internet applications (such as video games or streaming a high-definition movie).
                    This is a fundamental reason why blockchains still cannot be applied on the internet at a large scale.
                    The Elastos team recognizes this fact, and therefore believes that blockchain development should not rely solely on the main chain for computation.
                    Elastos aims to scale their blockchain system by providing support for sidechains, which will help meet requirements for running applications with high IOPS(Input Output Operations Per Second).
                    The Elastos main chain will be responsible for the small but very important role of trading and transferring ELA, thus providing stability to the blockchain system.
                    Elastos wants to avoid bloating the main chain with unnecessary smart contracts.
                    Instead, only major infrastructure upgrades will take place on the main chain.
                    All other smart contracts can be implemented on sidechains, enabling scalability.
                </p>
                <p>
                    This kind of hierarchical and structured design philosophy will pave the way for a future blockchain paradigm, such as the aforementioned development from stand-alone computation to a distributed one.
                    This is a key innovation in blockchain technology, and more important than the partial technology of singular consensus algorithms and chains.
                    The team will implement basic services as sidechains for global and public use.
                    These services include ID generation, token distribution, digital asset trading, and fast payment systems.
                    These basic services, all important infrastructural components, are part of the Elastos Smart Web.
                </p>
                <p>
                    In addition, the team will also provide support for third-party sidechain development.
                    Transactions are the most important part of the interface between the main and sidechains.
                    The transaction procedure for sending tokens from the main chain to a sidechain is equivalent to sending from a user account on the main chain to a multi-signature address corresponding to the sidechain.
                    The process automatically checks that the sidechain can identify the transaction and deposit the equivalent value of sidechain tokens to the sidechain account.
                </p>
                <a href="https://www.elastos.org/wp-content/uploads/2018/White Papers/elastos_sidechain_whitepaper_v0.3.0.8_EN.pdf?_t=1526918471" target="_blank">Read Elastos Sidechain Whitepaper</a>
                <h3>Main chain, sidechains and friendchains</h3>`,
  part6: `<p>
                    The Elastos main chain uses the arbitrator’s joint signature and SPV(Simplified Payment Verification) mechanism to guarantee the security of the transfer with the sidechain.
                    The main chain token holders(ELA holders) jointly elect a certain number of arbitrators.
                    The arbitrator is responsible for signing the token withdrawals from the sidechain to the main chain.
                    Each sidechain node synchronizes all of the block headers of the main chain.
                </p>
                <p>
                    The Elastos sidechain can use any consensus mechanism.
                    Currently, the Elastos team has already developed a POW consensus based sidechain that can connect with the main chain to complete SPV and DPOS based deposit and withdrawal operations.
                    This POW based sidechain can use the computing power of the main chain to ensure its own security.
                    What this essentially means is that if DApps choose to implement a POW based consensus mechanism, they can all piggyback on ELA merged mining hashpower that it inherits from the strong and resilient bitcoin network.
                </p>
                <p>
                    In addition, through the cross-chain technology, Elastos can also complete mutual transfers with other blockchain systems that issue their own token.
                    This kind of blockchain that can transfer funds with Elastos is called a “friend chain” and atomic swaps are possible between main chain and friendchains.
                </p>
                <h5>Asset transfer from main chain to sidechain</h5>`,
  part7: `<ol>
                    <li>User wants to convert their 5 ELAs to STOKENs</li>
                    <li>Initiate the transfer of 5 ELAs from main chain address U to the main chain address S(the sidechain address in main chain)</li>
                    <li>Transaction #1 is created with 5 ELAs and also the actual sidechain address u is attached to the transaction</li>
                    <li>Transaction #1 is transferred from main chain address U to main chain address S</li>
                    <li>Arbitrator on duty waits until Transaction #1 is put into a block by main chain miner and waits until sufficient confirmations</li>
                    <li>Arbitrator on duty then obtains Transaction #1 through its SPV(Simplified Payment Verification) module</li>
                    <li>Arbitrator on duty obtains the sidechain address u and constructs Transaction #2 that contains SPV Proof Path of Transaction #1 along with 5 STOKENs</li>
                    <li>Sidechain miner packages Transaction #2 and puts it into the sidechain's block</li>
                    <li>With the consensus method this sidechain has, it waits for sufficient confirmations</li>
                    <li>User gets 5 STOKENs on their sidechain address</li>
                </ol>
                <h5>Asset transfer from sidechain to main chain</h5>`,
  part8: `<ol>
                    <li>User wants to convert 5 STOKENs to ELAs</li>
                    <li>Initiate the transfer of 5 STOKENs from sidehain address u to the main chain address U</li>
                    <li>Transaction #1 is created with 5 STOKENs and also the actual main chain address U is attached to the transaction</li>
                    <li>Transaction #1 is transferred from main chain address U to main chain address S</li>
                    <li>Arbitrator on duty waits until Transaction #1 is put into a block by sidechain miner and waits until sufficient confirmations</li>
                    <li>Arbitrator on duty then obtains Transaction #1 from its own sidechain node(every arbitrator has 1 node dedicated to each sidechain that exists on elastos ecosystem)</li>
                    <li>Arbitrator on duty obtains the main address U and constructs Transaction #2 that contains Transaction #1 hash along with 5 ELAs</li>
                    <li>Arbitrator on duty broadcasts the Transaction #2 to all the rest of the arbitrator nodes for signatures. Each arbitrator signs Transaction #2 and send back the signature to the arbitrator on duty</li>
                    <li>Arbitrator on duty waits to receive over 2/3rd of the arbitrator signatures and then submits Transaction #2 along with the received signatures to the main chain</li>
                    <li>Main chain miner packages Transaction #2 and puts it into the main chain's block</li>
                    <li>With the PoW consensus method of the main chain, it waits for sufficient confirmations</li>
                    <li>User gets 5 ELAs on their main chain address</li>
                </ol>
                <h5>Friendchains of Elastos</h5>
                <p>
                    When you convert N number of ELAs to a sidechain token, all you’re doing is locking x number of ELAs on the main chain and then unlocking y number of tokens on the sidechain.
                    Each sidechain address has a corresponding ELA address on the main chain for this very reason.
                    However, there are other use cases where the blockchain is not native to elastos ecosystem such as NEO, ETH, etc. These are known as friendchains.
                </p>
                <p>Elastos support for friendchain is separated into a two-stage process:</p>
                <ol>
                    <li>The first stage supports the cross-chain atomic transaction between the friendchain and the Elastos main chain. This type of transaction is peer to peer and requires the parties to negotiate exchange rates and create mutual and constrained atomic exchange transactions.</li>
                    <li>The second stage will be based on decentralized exchanges, supporting the free exchange of the main chain and friendchain tokens. Users will no longer need to create exchange transactions from user to user like in the atomic-swap example.</li>
                </ol>
                <h3>Asset Transfer between main chain and friendchains</h3>
                <p>
                    The first stage of atomic swap will be achieved using hash lock mechanism and will work as described below:
                </p>`,
  part9: `<ol>
                    <li>Alice wants to sell 5 ELAs with Bob in exchange for 25 FTOKENs because the current market rate is 1 ELA = 5 FTOKENs</li>
                    <li>Alice generates a random number x and uses it to get hash(x) that is then encoded within Transaction #1 along with 5 ELAs</li>
                    <li>Alice puts Transaction #1 from her address EA to Elastos main chain E</li>
                    <li>Bob sees Transaction #1 on Elastos main chain E, constructs Transaction #2 with 25 FTOKENs along with hash(x) on Friendchain F</li>
                    <li>Alice sees Transaction #2 on Friendchain F and uses the number x along with her private key on Friendchain E to unlock this transaction. The 25 FTOKENs are then transferred to Alice</li>
                    <li>Bob sees that Transaction #2 has been unlocked and retrieves x</li>
                    <li>Bob signs Transaction #1 with his private key on Elastos main chain E along with the number x. This unlocks Transaction #1 for Bob and finally 5 ELAs are transferred to his address on Elastos main chain E</li>
                </ol>
                <a href="https://medium.com/@kiran.pachhai/elastos-architecture-the-main-chain-sidechains-and-friendchains-3727ef477d8e" target="_blank">Read an article on the main chain, sidechains and friendchains</a>`,
  part10: `<h3>What is CAR?</h3>
                <p>CAR means The Component Assembly Runtime (CAR). It is Java with machine code. It is COM with metadata.</p>
                <p>
                    It is a component-oriented programming model and also a programming idea, and it is described by special standards divided in two parts: specification and implementation.
                    The specification part tells how to define the components and how to communicate among components.
                    Following the specification, any language can be used. Thus, CAR can be implemented in many ways.
                    Elastos operating system implements the CAR core services.
                </p>
                <p>
                    CAR components provide service using interface, component interface needs the metadata to describe the component so that other users can know how to use the service.
                    Metadata describe the relationship between services and calls. With this description, calling between different components becomes possible, and members of the long-range, inter-process communication can be properly carried out.
                    The major solved problems by CAR component technology are: component from different sources can interoperate; components upgrade but affect no other component; component is independent of the programming language; transparency of component operating environment
                </p>
                <p>
                    The programming idea with CAR is the essence technology in Elastos, it runs through the entire technology system implementation.
                    In Elastos both the operating system kernel and the component library provided by Elastos platform are implemented using CAR technology.
                    Closely integrated of the operating system kernel with CAR technology runtime environment can provide strong support to Elastos architecture.
                </p>
                <p>
                    By doing this, you're able to use a CAR component to establish communication between a program written in C/C++ and one in HTML5/JS - being able to talk without confirming to one programming language.
                    This leads to convenience for the developer as well as portability.
                    This means that all the DApps can be run on a C++ virtual machine no matter what language the DApp is written in and this also means that there's no need of a bridge like a JNI that is vulnerable to many man-in-the-middle-attacks, virus attacks, etc.
                    In addition, this also lets an elastos DApp use the full capability of the hardware as the program is able to directly talk to the hardware for things like game engines, multimedia codec, etc thereby enabling developers to build true decentralized applications powered by the blockchain technology which is also very secure and scalable at the same time.
                </p>
                <h3>CAR Language</h3>
                <p>This focuses on the syntax of CAR, such as classes, interfaces, methods, parameters, inheritance, constructors, and so on.</p>
                <a href="https://github.com/elastos/Elastos.RT/blob/master/Docs/CAR_Language.md" target="_blank">Source</a>
                <h3>How to write a CAR component?</h3>
                <p>This focuses on the use of the CAR component and how to implement the inheritance relationship as well as the use of some of the macros defined.</p>
                <a href="https://github.com/elastos/Elastos.RT/blob/master/Docs/How_To_Write_A_Car_Component.md" target="_blank">Source</a>`,
  part11: `<h3>Overview</h3>
                This is material for any developer who has some experience with programming languages like Javascript/NodeJS, HTML5, Java, Swift, C++, Golang, etc and wants to get started with various github projects that are open sourced by Elastos.
                This includes the core development of the product such as the Elastos Runtime environment, Elastos Blockchain, creating and integrating ELA into wallets, or creating applications using the SDK if you're a DApp developer.
                In order to make it very easy for any developer(even to someone who's not very familiar with blockchain) Elastos provides an SDK that makes it easy to start developing a decentralized application integrated with blockchain technology without having to fully learn about how blockchain works underneath.
                <h3>The Four Pillars of Elastos Ecosystem</h3>
                <h5>Blockchain and Smart Contracts</h5>
                As the operating system’s trusted zone, the blockchain can implement “trust”.
                The Elastos main chain uses Bitcoin’s POW mechanism to ensure the reliability of data transmission through joint mining with Bitcoin.
                At the same time, Elastos provides services and extends third-party applications through its side chains.
                <h5>Elastos Carrier</h5>
                Elastos Carrier is a completely decentralized P2P network service platform.
                For Elastos, it is an important support infrastructure for decentralized application development and operation.
                It is the Elastos P2P Network Platform part of the architecture diagram.
                <h5>Elastos Runtime</h5>
                Elastos Runtime runs on the user’s equipment to achieve a “reliable runtime environment.”
                By developing Elastos DApp, independent developers can use (play) digital assets such as digital audio and video playback.
                VM guarantees digital assets will run under blockchain control, providing users with the ability to consume and invest in digital content.
                <h5>Elastos SDK</h5>
                This is the traditional APP (i.e. Wechat, QQ, Taobao, and other mobile phone software).
                These APPs can extend their capabilities by introducing the Elastos SDK, gaining typical blockchain abilities like identity authentication and trusted records.
                <h5>Some of the features of Elastos</h5>
                <ul>
                    <li>The Elastos public chain is clean and simple, and hidden from third-party applications and services.</li>
                    <li>Elastos prevents overload of the main chain by having a few predefined sidechains built into the Elastos Carrier platform.</li>
                    <li>Elastos promotes the property rights of digital content. Elastos has the capability to issue tokens for digital assets or applications and to establish the ownership of digital content through smart contracts.</li>
                    <li>Elastos Runtime runs on the OS of customers’ mobile devices. Apps are free to run and their performance is comparable to existing mobile apps. Elastos supports traditional programming languages, making it relatively easy to write code. Elastos also supports popular programming frameworks.</li>
                    <li>The separation of apps from the network ensures that digital content won’t be leaked.</li>
                    <li>Even when Elastos apps are running on operating systems such as iOS, Android and Windows, the local OS won’t be able to sabotage the property rights of digital assets. The value of digital assets is preserved.</li>
                    <li>For non-Elastos apps such as Android or iOS apps, users can access the Elastos Smart Web through the Elastos SDK. Users can log into non-Elastos apps using their Elastos Smart Web ID. Users can also keep their non-Elastos app data in their Elastos cloud storage.</li>
                    <li>Both Elastos smart contracts and Elastos Dapps run on the Elastos Smart Web. This creates a closed platform and avoids the necessity of moving on and off the blockchain. This closed platform creates a special economic zone where users can feel secure while trading digital assets. This enables a closed cycle of production, transaction, and consumption that is necessary for creating wealth.</li>
                </ul>`,
}
