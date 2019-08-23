import React, { Component } from "react";

import SyntaxHighlighter from "react-syntax-highlighter";
import { gruvboxDark } from "react-syntax-highlighter/dist/esm/styles/hljs";

import {
  Grid,
  Row,
  Col,
  FormGroup,
  ControlLabel,
  FormControl,
  Button
} from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";
import axios from "axios";
import { baseUrl } from "../utils/api";

class ApiKeygenerator extends Component {
  constructor() {
    super();
    this.handleClick = this.handleClick.bind(this);

    this.state = {
      isKeyGenerated: false,
      apiKey: ""
    };
  }

  getApiKeyFromServer() {
    const endpoint = "common/generateAPIKey";
    axios
      .get(baseUrl + endpoint, {
        //mode: "cors",
        params: {}
      })
      .then(response => {
        this.setState({
          isKeyGenerated: response.data.status === 200,
          apiKey: response.data["API Key"]
        });
        window.apiKey = this.state.apiKey;
        console.log(this.state.apiKey);
      })
      .catch(error => {
        // Error
        if (error.response) {
          // The request was made and the server responded with a status code
          // that falls out of the range of 2xx
          // console.log(error.response.data);
          // console.log(error.response.status);
          // console.log(error.response.headers);
          this.setState({
            status: "FAILURE",
            output: JSON.stringify(error.response.data, null, 2)
          });
        } else if (error.request) {
          // The request was made but no response was received
          // `error.request` is an instance of XMLHttpRequest in the
          // browser and an instance of
          // http.ClientRequest in node.js
          this.setState({
            output: error.request
          });
          console.log(error.request);
        } else {
          // Something happened in setting up the request that triggered an Error
          this.setState({
            output: error.message
          });
          console.log("Error", error.message);
        }
      });
  }

  handleClick() {
    //TODO:
    //Do we need to generate a new key every time the button gets clicked?
    this.getApiKeyFromServer();
  }

  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={12}>
              <Card
                title="Generate an API Key"
                content={
                  <Row>
                    <Col md={12}>
                      <Button
                        variant="primary"
                        size="lg"
                        onClick={this.handleClick}
                      >
                        Generate key
                      </Button>
                    </Col>
                    <Col md={6}>
                      {this.state.isKeyGenerated && (
                        <FormGroup>
                          <br />
                          <ControlLabel>API Key</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder=""
                            name="apiKey"
                            value={this.state.apiKey}
                            readOnly
                          />
                        </FormGroup>
                      )}
                    </Col>
                  </Row>
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
                            Generates a API Key of length 32 bit
                          </p>
                        </FormGroup>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`GET /api/1/common/generateAPIKey HTTP/1.1
Host: localhost:8888
`}
                        </SyntaxHighlighter>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`HTTP/1.1 200 OK
Vary: Accept
Content-Type: application/json

{
  "API Key": "KHBOsth7b3WbOTVzZqGUEhOY8rPreYFM", 
  "status": 200, 
  "timestamp": "2019-08-22 15:40:11 -0400", 
  "path": "http://localhost:8888/api/1/common/generateAPIKey"
}
`}
                        </SyntaxHighlighter>
                      </Col>
                    </Row>

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
                        <SyntaxHighlighter language="jsx" style={gruvboxDark}>
                          {`    stringLength = 32
    api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(stringLength))
    data = {"API Key":api_key,"status":200, "timestamp":getTime(),"path":request.url}
    return Response(json.dumps(data), 
        status=200,
        mimetype='application/json'
      )
                          `}
                        </SyntaxHighlighter>
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

export default ApiKeygenerator;


