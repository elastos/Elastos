const React = require('react');

module.exports = (props) => {
  const guiToggles = props.guiToggles;
return (
<table id="login" className="bordered w750h520px">
  <tbody>
    <tr>
      <td className="bordered w250px h20px ta_center va_top">
      // showGenerateNewPrivateKey
      // showGenerateNewMnemonic
      // showLogin
        <button className="bgcolor_black_hover"  onClick={(e) => guiToggles.showGenerateNewPrivateKey()}>
        </button>
      </td>
      <td className="bordered w250px h20px ta_center va_top">
        <button className="bgcolor_black_hover"  onClick={(e) => guiToggles.showGenerateNewMnemonic()}>
        </button>
      </td>
      <td className="bordered w250px h20px ta_right va_top">
        <button className="bgcolor_black_hover"  onClick={(e) => guiToggles.showLogin()}>
        </button>
      </td>
    </tr>
  </tbody>
</table>
);
}
