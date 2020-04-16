const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const openDevTools = props.openDevTools;


  const changeNodeUrl = () => {
    App.changeNodeUrl();
    GuiToggles.hideHomeMenu();
  }

  const changeNetwork = (event) => {
    App.setRestService(event.target.value);
    App.refreshBlockchainData();
  }

  return (
    // <div id="homeMenu" className="color_white bgcolor_gray display_inline_block">
    <div id="homeMenu">
      <select value={App.getCurrentNetworkIx()} name="network" style={{background: "inherit"}} onChange={(e)=> changeNetwork(e)}>
        <option value="0">{App.REST_SERVICES[0].name}</option>
        <option value="1">{App.REST_SERVICES[1].name}</option>
        <option value="2">{App.REST_SERVICES[2].name}</option>
      </select>
      <div className="display_inline_block">Change Node</div>
      <div className="display_inline_block">
        <input className="display_inline" type="text" size="32" id="nodeUrl" style={{background: "inherit"}} placeholder={App.getRestService()}></input>
        <div className="bordered padding_5px display_inline bgcolor_gray_hover" onClick={(e) => changeNodeUrl()}>Change</div>
      </div>
      <div className="bordered padding_5px display_inline bgcolor_gray_hover" onClick={(e) => App.resetNodeUrl()}>Reset</div>
      <div className="bordered padding_5px display_inline bgcolor_gray_hover" onClick={(e) => openDevTools()}>Dev Tools</div>
      <div className="bordered padding_5px display_inline bgcolor_gray_hover" onClick={(e) => GuiToggles.hideHomeMenu()}>Cancel</div>
      <div id="homeMenuClose" className="bordered padding_5px display_inline bgcolor_gray" title="menu" onClick={(e) => GuiToggles.hideHomeMenu()}>
        <img className="centered_img" src="artwork/more-horizontal.svg" />
      </div>
    </div>
  );
}

