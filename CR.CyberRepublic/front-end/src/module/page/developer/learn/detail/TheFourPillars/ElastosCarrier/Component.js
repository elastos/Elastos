import React from 'react'
import '../../style.scss'

export default class extends React.Component {

  render () {
    return (
      <div className="p_developerLearnDetail">
        <h3>Overview</h3>
        <p>
                    Elastos Carrier is a completely decentralized P2P network service platform.
                    For Elastos, it is an important support infrastructure for decentralized application development and operation.
                    It is the Elastos P2P Network Platform and is part of the architecture diagram.
        </p>
        <h3>Features of Elastos Carrier</h3>
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
                        Everything on Elastos is blacklisted by default.
                        In order for any two devices or any two users or any two apps to talk to each other, they have to be added to each other’s friends list first.
          </li>
        </ol>
        <h3>Carrier Programming Basic Concepts</h3>
        <ul>
          <li>
            <strong>Whisper</strong>
: Represents a carrier node object. It is used to control the initialization, start and stop of nodes
          </li>
          <li>
            <strong>Friend</strong>
: A friend, represents a node to license another node. For example, if one node sends data to another node, it must first establish a friend relationship. Otherwise, they cannot communicate. For friends, you can add, delete, send messages, etc. A friend-based message communication is a limited-length, unreliable communication mechanism similar to a UDP packet.
          </li>
          <li>
            <strong>Session</strong>
: If you want to establish a long connection with high time efficienty and high reliability data transmission channel between the two nodes, you can choose to use the session mode. Initialization, requests and confirmation requests, etc are available for a session.
          </li>
          <li>
            <strong>Stream</strong>
: After the session is successfully established, the nodes transfer data by reading and writing from and to a stream.
          </li>
          <li>
            <strong>Channel</strong>
: This is a multiplexing implementation for the same session which is a wrapper around the use of a session.
          </li>
        </ul>
        <h3>How does Elastos Carrier work?</h3>
        <p>
                    As the base platform for Elastos, the carrier itself is at a very initial level, not directly exposed to application developers.
                    Ultimately, the entire platform will package the carrier within Elastos RT's operating environment and let RT expose the entire carrier function interface.
                    Although the carrier now provides an API, it's still at the intermediate state.
                    Of course, it also supports developers to develop some applications, do some development based on the API of the current carrier platform.
                    But the final form will be integrated within the RT. Right now, Elastos has its own internal team dedicated to the integration of RT, which integrates all communication capabilities of the carrier within the RT.
                    When doing the development, people only need to focus on RT's API. All features of the carrier can be accessed by using the RT's API.
        </p>
        <p>
                    The foundation of the carrier's current technical architecture is based on the DHT-centric, decentralized distributed network.
                    On top of it, a friend to friend basic communication network was made, which we provide several layers of functional interfaces.
                    First, we have the relationship between nodes and nodes. On top of it, the message will be provided.
                    On top of that, continuous data transmission, loss transmission, datagram transmission, and higher layer transmission semantics will be provided.
        </p>
        <p>
                    So the carrier's basic features are its own functionality in the framework of communication infrastructure.
                    Is the carrier involved with RTC of Elastos's RT? In fact, this is an inverted relationship.
                    Elastos RT will provide the RTC ability, but this RTC ability ultimately relies on the carrier to provide the foundation support.
                    It means RT will have RTC, but the RTC eventually comes down on the carrier implementation, which means that the RTC is implemented with the carrier's communication channel.
                    This is an inverted relationship. The carrier itself is involved with RTC, but the carrier's communication capabilities are injected into RT, which enables RT to achieve remote RTC.
        </p>
        <p>
                    At present, the carrier contains several levels of capability: First, the carrier contains support for the association between nodes, and all nodes on the carrier are in a DHT network.
                    However, nodes and nodes are relatively isolated. To establish the communication between nodes and nodes, first of all, we need to establish the relationship between the nodes. So the carrier has a set of APIs.
                    Helping establish relationships between the nodes is a bit like making real-world friends, or like your contacts in Wechat.
                    For A and B to communicate, A and B have to be friends.
                    The carrier can support establishing the relationships between nodes with the basic API.
                    That's what the API does. On this basis, the carrier also provides a message-oriented API, which means once a relationship is established between the nodes, the most basic message can be used.
                    The messages are not connected. They were done through the network. If A and B are friends, they can send messages to each other at any time.
        </p>
        <p>
                    A can send messages to B directly and vice versa.
                    The message doesn't need to be connected between A and B. the DHT network can serve as an intermediary to achieve the message transmission between A and B.
                    That's the case for messages.
                    On top of the message, we also provide products facing big data transmission.
                    For example, besides messages between A and B, big chunks of data also need to be transmitted between applications.
                    Big chunks of data are kind of communication mechanisms like the socket.
                    The carrier also provides the API of the session to help establish data transmission similar to the UDP datagram.
                    In the carrier system, we call it stream.
        </p>
        <p>
                    The session call ability provided by the carrier actually has two tiers.
                    One is a mode similar to UDP datagram; The other mode is similar to TCP streaming.
                    Both models are implemented on top of the stream. You can check the box and either select the datagram mode or streaming mode.
                    The two modes are identical in terms of interface, but they have two different working mechanisms.
                    One is simulation TCP, the other is simulation UDP. On top of this, we also provide data encryption and decryption, multiplexing, and interface forwarding upper semantics, to make it easier for carrier developers to provide application support and make it easier to use.
        </p>
        <p>
                    The carrier is a completely decentralized network.
                    It is not operated by some intermediate or a group of central servers.
                    Therefore the carrier itself, as a network, will not store any user information.
                    For example, messages that the user sends, communication between nodes, relationships between friends are all stored in the node itself.
                    In other words, the data of the node is right here. The carrier or intermediate servers will not touch or save any user data.
        </p>
      </div>
    )
  }
}
