const React = require('react');

module.exports = (props) => {
  const guiToggles = props.guiToggles;
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
  </tbody>
</table>
);
}
