const React = require('react');
const electron = require('electron');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const openDevTools = props.openDevTools;
  const page=props.page;

  const remote = electron.remote;

 
  const Version = () => {
    return remote.app.getVersion();
  }  


  const changeNodeUrl = () => {
    App.changeNodeUrl();
    GuiToggles.hideMenu(page);
  }

  const changeNetwork = (event) => {
    App.setRestService(event.target.value);
    App.refreshBlockchainData();
  }

  const hideMenu = () => {
    GuiToggles.hideMenu(page);
  }

  return (
    <div id={page+'Menu'}>
      <div className="display_inline_block gradient-font marginleft_25px"><Version/> </div>
      <select value={App.getCurrentNetworkIx()} className="dark-hover menu-selector" name="network" style={{background: "inherit"}} onChange={(e)=> changeNetwork(e)}>
        <option value="0">{App.REST_SERVICES[0].name}</option>
        <option value="1">{App.REST_SERVICES[1].name}</option>
      </select>
      <div className="display_inline_block">Change Node</div>
      <div className="display_inline_block menu-change-div">
        <input className="display_inline menu-change-input" type="text" size="28" id="nodeUrl" style={{background: "inherit"}} placeholder={App.getRestService()}></input>
        <div className="display_inline dark-hover menu-change-btn cursor_def" onClick={(e) => changeNodeUrl()}>Change</div>
      </div>
      <div className="padding_5px display_inline dark-hover br10 cursor_def" onClick={(e) => App.resetNodeUrl()}>Reset</div>
      <div className="padding_5px display_inline dark-hover br10 cursor_def" onClick={(e) => openDevTools()}>Dev Tools</div>
      {/* <div className="padding_5px display_inline dark-hover br10 cursor_def" onClick={(e) => hideMenu()}>Cancel</div> */}
      <div id={page+'MenuClose'} className="padding_5px display_inline dark-hover br10 marginright_20px" title="close" onClick={(e) => hideMenu()}>
        <img src="artwork/menuclose.svg" />
      </div>
    </div>
  );
}
