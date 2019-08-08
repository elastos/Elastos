import React, { Component } from "react";
import { Grid, Row, Col, FormGroup } from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";
import Button from "components/CustomButton/CustomButton.jsx";

class UserProfile extends Component {
  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={12}>
              <Card
                title="Generate Wallet"
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
                    <Button bsStyle="info" pullRight fill type="submit">
                      Enter
                    </Button>
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
