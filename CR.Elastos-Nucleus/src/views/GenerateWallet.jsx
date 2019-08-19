import React, { Component } from "react";
import Dropzone from "react-dropzone";
import SyntaxHighlighter from "react-syntax-highlighter";
import { docco } from "react-syntax-highlighter/dist/esm/styles/hljs";

import { Grid, Row, Col, FormGroup } from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";
import CustomButton from "components/CustomButton/CustomButton.jsx";

class UserProfile extends Component {
  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={12}>
              <Card
                title="Upload Files"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <Dropzone onDrop={acceptedFiles => console.log("test")}>
                          {({ getRootProps, getInputProps }) => (
                            <section>
                              <div {...getRootProps()}>
                                <input {...getInputProps()} />
                                <p>
                                  Drag 'n' drop some files here, or click to
                                  select files
                                </p>
                              </div>
                            </section>
                          )}
                        </Dropzone>
                      </Col>
                    </Row>
                    <CustomButton
                      text="Upload"
                      path="/admin/generatewallet"
                      pullRight
                      bsStyle="info"
                      fill
                    />
                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
          </Row>
          <Row>
            <Col md={12}>
              <Card
                title="Documentation"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormGroup controlId="formControlsTextarea">
                          <p>
                            <span className="category" />
                            Lorem ipsum dolor sit amet, consectetuer adipiscing
                            elit, sed diem nonummy nibh euismod tincidunt ut
                            lacreet dolore magna aliguam erat volutpat. Ut wisis
                            enim ad minim veniam, quis nostrud exerci tution
                            ullam corper suscipit lobortis nisi ut aliquip ex ea
                            commodo consequat. Duis te feugi facilisi. Duis
                            autem dolor in hendrerit in vulputate velit esse
                            molestie consequat, vel illum dolore eu feugiat
                            nulla facilisis at vero eros et accumsan et iusto
                            odio dignissim qui blandit praesent luptatum zzril
                            delenit au gue duis dolore te feugat nulla facilisi.
                          </p>
                        </FormGroup>
                      </Col>
                    </Row>
                    <CustomButton
                      text="Show Content"
                      path="/admin/transactions"
                      pullRight
                      bsStyle="info"
                      fill
                    />
                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
          </Row>
          <Row>
            <Col md={12}>
              <Card
                title="Code Snippet"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <SyntaxHighlighter language="javascript" style={docco}>
                          {`import React from 'react';
import ReactDOM from "react-dom";

import { BrowserRouter, Route, Switch, Redirect } from "react-router-dom";

import "bootstrap/dist/css/bootstrap.min.css";
import "./assets/css/animate.min.css";
import "./assets/sass/light-bootstrap-dashboard-react.scss?v=1.3.0";
import "./assets/css/demo.css";
import "./assets/css/pe-icon-7-stroke.css";

import AdminLayout from "layouts/Admin.jsx";

ReactDOM.render(
  <BrowserRouter>
    <Switch>
      <Route path="/admin" render={props => <AdminLayout {...props} />} />
      <Redirect from="/" to="/admin/dashboard" />
    </Switch>
  </BrowserRouter>,
  document.getElementById("root")
);`}
                        </SyntaxHighlighter>
                      </Col>
                    </Row>
                    <CustomButton
                      text="Show Content"
                      path="/admin/transactions"
                      pullRight
                      bsStyle="info"
                      fill
                    />
                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
          </Row>
        </Grid>
      </div>
    );
  }
}

export default UserProfile;
