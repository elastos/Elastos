const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const page=props.page;
  const hideBanner = () => {
    GuiToggles.hideBanner(page);
  }

  return (
    <div id={page+'Banner'} className = {App.getBannerClass()}>
      {App.getBannerStatus()}
      <div className="bordered padding_5px display_inline bgcolor_gray_hover" onClick={(e) => hideBanner()}>Hide</div>
    </div>
  );
}
