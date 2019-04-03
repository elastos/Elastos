import React from 'react'
import '../../style.scss'

export default class extends React.Component {

  render () {
    return (
      <div className="p_developerLearnDetail">
        <h3>Overview</h3>
        <p>
                    As the operating system’s trusted zone, the blockchain can implement “trust”.
                    The Elastos main chain uses Bitcoin’s POW mechanism to ensure the reliability of data transmission through merge mining with Bitcoin.
                    At the same time, Elastos provides services and extends third-party applications through its side chains.
        </p>
        <h3>Multi-layer blockchain architecture</h3>
        <p>
                    Elastos employs a main chain - sidechain structure.
                    The main chain is only ever used for simple ELA payments and acts as a trustworthy ledger with the most basic transactions.
                    The sidechains are used to extend the functionality of the elastos ecosystem.
                    Everything including smart contracts will run as sidechains.
                    For example, one of the first sidechains, ID sidechain, is used to issue IDs to devices, users and applications on the new internet Elastos is building.
                    Another sidechain maybe a digital asset smart contract sidechain. Another sidechain may be ethereum smart contract sidechain.
                    Another sidechain may be NEO smart contract sidechain. Any existing smart contracts may be ported to Elastos to work as a sidechain and after that, the applications built for elastos may be able utilize any of these smart contracts or they may even build their own sidechains to their own liking.
        </p>
        <p>
                    The sidechain may process hundreds of thousands of transactions per block and may even have a very high TPS.
                    Only the hash of each block from each sidechain is stored on the elastos main chain to establish trust.
                    By decoupling the main chain from sidechains greatly eliminates the many congestion and scalibility issues many of the public blockchain projects face today.
                    This means that the TPS of the elastos main chain is not an issue as it’s only used to establish trust. The applications themselves never interact with the elastos main chain.
                    Instead, they only interact with their own sidechains.
                    So, in this way, applications may even choose to create their own consensus method that’s catered to their needs and build a sidechain for themselves.
        </p>
        <h3>Summary</h3>
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
      </div>
    )
  }
}
