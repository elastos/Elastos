import React, { Component } from "react";
import {
  Grid,
  Row,
  Col,
  FormGroup,
  ControlLabel,
  FormControl
} from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";
import CustomButton from "components/CustomButton/CustomButton.jsx";

class UserProfile extends Component {
  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={6}>
              <Card
                title="Input Section"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormGroup controlId="formControlsTextarea">
                          <ControlLabel>Input text</ControlLabel>
                          <FormControl
                            rows="5"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Enter infromation here"
                          />
                        </FormGroup>
                      </Col>
                    </Row>
                    <CustomButton text="Submit" pullRight bsStyle="info" fill />
                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
            <Col md={6}>
              <Card
                title="Output Section"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormGroup controlId="formControlsTextarea">
                          <ControlLabel>Output text</ControlLabel>
                          <FormControl
                            rows="5"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Display output here"
                          />
                        </FormGroup>
                      </Col>
                    </Row>
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
