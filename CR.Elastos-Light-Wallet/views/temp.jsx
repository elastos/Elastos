const React = require('react');

module.exports = (props) => {
  const GuiToggles = props.GuiToggles;
  let restService;
  return (
    <table id="temp" className="bordered w750h520px">
      <tbody>
      <tr id="send-amount">
        <td className="black_on_white h20px darkgray_border">
          <div className="gray_on_white">Send Amount</div>
          <br />
          <input className='monospace' type="text" size="64" id="sendAmount" placeholder="Send Amount"></input>
        </td>
      </tr>
      <tr id="to-address">
        <td className="black_on_white h20px darkgray_border">
          <div className="gray_on_white">To Address</div>
          <br />
          <input style={{
                  fontFamily: 'monospace'
                }} type="text" size="64" id="sendToAddress" placeholder="Send To Address"></input>
        </td>
      </tr>
      <tr id="fees">
        <td className="black_on_white h20px darkgray_border">
          <div className="gray_on_white display_inline_block">Fees (in Satoshis)</div>
          <br />
          <input style={{
                  fontFamily: 'monospace'
                }} type="text" size="64" id="feeAmount" placeholder="Fees"></input>
          <p></p>
          <div className="white_on_black lightgray_border bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e)=> updateAmountAndFeesAndRenderApp()}>Estimated New Balance</div>
        </td>
      </tr>
      <tr id="change-node">
        <td className="black_on_white h20px darkgray_border">
          <div className="gray_on_white">Node URL</div>
          <br/>
          <input style={{
              fontFamily: 'monospace'
            }} type="text" size="64" id="nodeUrl" placeholder={restService}></input>
          <br/>
          <br/>
          <br/>
          <div className="white_on_gray bordered display_inline_block fake_button rounded padding_5px" onClick={(e) => showLogin()}>Cancel</div>
          <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => changeNodeUrl()}>Change Node URL</div>
          <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => resetNodeUrl()}>Reset Node URL TO Default</div>
        </td>
      </tr>
      </tbody>
    </table>
  );
}
