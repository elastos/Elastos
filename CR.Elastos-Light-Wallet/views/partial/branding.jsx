const React = require('react');

module.exports = (props) => {
  return (
    <div id="branding" className="logo-info" >
      <img src="artwork/logo.svg" className="logoimage" height="41px" width="123px" onClick={(e) => GuiToggles.showHome()} />
     </div>
)
}



//     <div id="branding" className="bordered w250px h90px bgcolor_black_hover">
//       Branding
//     </div>
//   )
// }
