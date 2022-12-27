import React from 'react'
import '../../style.scss'

export default class extends React.Component {

  render () {
    return (
      <div className="p_developerLearnDetail">
        <h3>Overview</h3>
        <p>
                    This is the traditional APP (i.e. Wechat, QQ, Taobao, and other mobile phone software).
                    These APPs can extend their capabilities by introducing the Elastos SDK, gaining typical blockchain abilities like identity authentication and trusted records.
        </p>
        <p>
                    Elastos provides a C++ SDK that can be used to develop native applications for android, ios devices, etc.
                    The only thing that’ll be missing from these native applications is that they won’t be running inside elastos runtime.
                    However, the rest of the features and capabilities that Elastos offers are available via the SDK.
                    That is why, any native applications that decide to use the SDK are called hybrid elastos applications.
                    They’re not pure elastos dapps because they are not run inside elastos runtime environment.
        </p>
      </div>
    )
  }
}
