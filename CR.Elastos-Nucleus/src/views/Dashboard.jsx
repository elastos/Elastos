import React, { Component } from "react";
import { Grid, Row, Col, FormGroup } from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";
import CustomButton from "components/CustomButton/CustomButton.jsx";

class Dashboard extends Component {
  createLegend(json) {
    var legend = [];
    for (var i = 0; i < json["names"].length; i++) {
      var type = "fa fa-circle text-" + json["types"][i];
      legend.push(<i className={type} key={i} />);
      legend.push(" ");
      legend.push(json["names"][i]);
    }
    return legend;
  }
  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={12}>
              <Card
                title="About Us"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormGroup controlId="formControlsTextarea">
                          <p>
                            <span className="category" />
                            - Elastos Console: The prototype version of Elastos Console contains around 6 services where developers can go to the console website to interact with. 
                            <br /><br />
                            - Each service has a different page complete with full interactivity, documentation and samples of code in various languages that developers can easily “plug-and-play” into their own applications to quickly get started in integrating Elastos technology. 
                            <br /><br />
                            - The Console also contains documentation for every single endpoint exposed via Elastos Smartweb Services. 
                            <br /><br />
                            - The Elastos Console is the main product coming out of GMU collaboration as this will continue to mature over the next 6 months to 1 year before fully supporting not just GMU net, but also having the ability to easily deploy applications to mainnet(Eg. Deploying apps for Trinity, deploying to eth sidechain mainnet, or even interacting with other university nets around the world). 
                            <br /><br />
                            - The goal is to make Elastos Console one stop shop for all the developers who are interested in developing using Elastos Technology.

                          </p>
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

export default Dashboard;
