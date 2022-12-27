const React = require('react');

module.exports = (props) => {
  const GuiToggles = props.GuiToggles;
  const onLinkClick = props.onLinkClick;

  return (
        <footer>

          <a className="exit_link paddingright_5px scale-hover" target="_blank" href="https://twitter.com/ElastosInfo" onClick={(e) => onLinkClick(e)}><img src="artwork/tw.svg" height="32px" width="32px" /> </a>
          <a className="exit_link paddingright_5px scale-hover" target="_blank" href="https://www.facebook.com/elastosorg/" onClick={(e) => onLinkClick(e)}><img src="artwork/fb.svg" height="32px" width="32px" /></a>
          <a className="exit_link scale-hover" target="_blank" href="https://t.me/elastosgroup" onClick={(e) => onLinkClick(e)}><img src="artwork/telegram.svg" height="32px" width="32px" /></a>
          <p id="logout-text" className="dark-hover cursor_def" onClick={(e) => GuiToggles.showLanding()}>Logout <img src="artwork/logout.svg" className="logout-image"/> </p>


        </footer>
  )
}