export default {
    bootstraps : [
      { ipv4: "13.58.208.50",  port: "33445", publicKey: "89vny8MrKdDKs7Uta9RdVmspPjnRMdwMmaiEW27pZ7gh" },
      { ipv4: "18.216.102.47", port: "33445", publicKey: "G5z8MqiNDFTadFUPfMdYsYtkUDbX5mNCMVHMZtsCnFeb" },
      { ipv4: "18.216.6.197",  port: "33445", publicKey: "H8sqhRrQuJZ6iLtP2wanxt4LzdNrN2NNFnpPdq1uJ9n2" },
      { ipv4: "52.83.171.135", port: "33445", publicKey: "5tuHgK1Q4CYf4K5PutsEPK5E3Z7cbtEBdx7LwmdzqXHL" },
      { ipv4: "52.83.191.228", port: "33445", publicKey: "3khtxZo89SBScAMaHhTvD68pPHiKxgZT6hTCSZZVgNEm" }
    ],
  
    CARRIER_CB_NAMES : [
      // "onIdle",
      "onConnection",
      "onReady",
      "onSelfInfoChanged",
      "onFriends",
      "onFriendConnection",
      "onFriendInfoChanged",
      "onFriendPresence",
      "onFriendRequest",
      "onFriendAdded",
      "onFriendRemoved",
      "onFriendMessage",
      "onFriendInviteRequest", // TODO
    ],

    STREAM_CB_NAMES : [
      "onSessionRequest",
      "onStateChanged",
      "onStreamData",
      "onChannelOpen", // TODO
      "onChannelOpened",  // TODO
      "onChannelClose",  // TODO
      "onChannelData",  // TODO
      "onChannelPending",  // TODO
      "onChannelResume",  // TODO
    ],

    CONNECTION_STATUS : {
      'CONNECTED' : 0,
      'DISCONNECTED' : 1
    },
    PRESENCE_STATUS : {
      'ONLINE' : 0,
      'AWAY' : 1,
      'BUSY' : 2
    },
    STREAM_TYPE : {
      'AUDIO' : 0,
      'VIDEO' : 1,
      'TEXT' : 2,
      'APPLICATION' : 3,
      'MESSAGE' : 4
    },
    STREAM_STATE : {

      Initialized: 1,
      TransportReady: 2,
      Connecting: 3,
      Connected: 4,
      Deactivated: 5,
      Closed: 6,
      Error: 7
    },
    STREAM_MODE : {
      /**
       * Compress option, indicates data would be compressed before transmission.
       * For now, just reserved this bit option for future implement.
       */
      COMPRESS: 1,
      /**
       * Encrypt option, indicates data would be transmitted with plain mode.
       * which means that transmitting data would be encrypted in default.
       */
      PLAIN: 2,
      /**
       * Relaible option, indicates data transmission would be reliable, and be
       * guranteed to received by remote peer, which acts as TCP transmission
       * protocol. Without this option bitwised, the transmission would be
       * unreliable as UDP transmission protocol.
       */
      RELIABLE: 4,
      /**
       * Multiplexing option, indicates multiplexing would be activated on
       * enstablished stream, and need to use multipexing APIs related with channel
       * instread of APIs related strema to send/receive data.
       */
      MULTIPLEXING: 8,
      /**
       * PortForwarding option, indicates port forwarding would be activated
       * on established stream. This options should bitwise with 'Multiplexing'
       * option.
       */
      PORT_FORWARDING: 16
    }
    
  };