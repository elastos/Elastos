const React = require('react');

module.exports = (props) => {
  const GuiToggles = props.GuiToggles;
  let restService;
  return (
    <table id="temp" className="bordered w750h520px">
      <tbody>
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
