/*!

=========================================================
* Light Bootstrap Dashboard React - v1.3.0
=========================================================

* Product Page: https://www.creative-tim.com/product/light-bootstrap-dashboard-react
* Copyright 2019 Creative Tim (https://www.creative-tim.com)
* Licensed under MIT (https://github.com/creativetimofficial/light-bootstrap-dashboard-react/blob/master/LICENSE.md)

* Coded by Creative Tim

=========================================================

* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

*/
import React, { Component } from "react";
import { Nav, NavDropdown, MenuItem } from "react-bootstrap";

class AdminNavbarLinks extends Component {
  render() {
    return (
      <div>
        <Nav>
          <NavDropdown eventKey={2} title="Dropdown" id="basic-nav-dropdown">
            <MenuItem href="/serviceone" eventKey={2.1}>
              API Service 1
            </MenuItem>
            <MenuItem href="/servicetwo" eventKey={2.2}>
              API Service 2
            </MenuItem>
            <MenuItem href="/servicethree" eventKey={2.3}>
              API Service 3
            </MenuItem>
            <MenuItem href="/servicefour" eventKey={2.4}>
              API Service 4
            </MenuItem>
            <MenuItem href="/servicefive" eventKey={2.5}>
              API Service 5
            </MenuItem>
          </NavDropdown>
        </Nav>
        <Nav pullRight>
          <NavDropdown
            eventKey={2}
            title="English"
            id="basic-nav-dropdown-right"
          >
            <MenuItem eventKey={2.1}>English</MenuItem>
            <MenuItem eventKey={2.2}>Spanish</MenuItem>
            <MenuItem eventKey={2.3}>French</MenuItem>
            <MenuItem eventKey={2.4}>Romanian</MenuItem>
            <MenuItem eventKey={2.5}>Italian</MenuItem>
          </NavDropdown>
          <NavDropdown
            eventKey={2}
            title="Account"
            id="basic-nav-dropdown-right"
          >
            <MenuItem eventKey={2.1}>Action</MenuItem>
            <MenuItem eventKey={2.2}>Another action</MenuItem>
            <MenuItem eventKey={2.3}>Something</MenuItem>
            <MenuItem eventKey={2.4}>Another action</MenuItem>
            <MenuItem eventKey={2.5}>Something</MenuItem>
            <MenuItem divider />
            <MenuItem eventKey={2.5}>Separated link</MenuItem>
          </NavDropdown>
        </Nav>
      </div>
    );
  }
}

export default AdminNavbarLinks;
