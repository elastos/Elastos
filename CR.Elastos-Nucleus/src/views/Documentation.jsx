import React, { Component } from "react";
import Iframe from 'react-iframe'
import {Col, Grid, Row} from "react-bootstrap";

class Documentation extends Component{

    render(){
        return(
        <div class="container">
        <Grid fluid>
          <Row>
            <Col md={12}>
                <Iframe url="http://localhost:8888/api/"
                width= "100%"
                height="750px"
                position="relative"
                frameBorder={0}
                />
            </Col>
          </Row>
        </Grid>
        </div>
        )

    }
}

export default Documentation

