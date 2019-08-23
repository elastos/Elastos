import React, { Component } from "react";
import SyntaxHighlighter from "react-syntax-highlighter";
import { gruvboxDark } from "react-syntax-highlighter/dist/esm/styles/hljs";

import {
  Grid,
  Row,
  Col,
  FormGroup,
  Button,
  ControlLabel,
  FormControl
} from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";
import axios from "axios";
import { baseUrl } from "../utils/api";

class UserProfile extends Component {
  constructor() {
    super();
    this.state = {
      inputs: {
        hashKey: "",
        apiKey: ""
      },
      status: "",
      output: ""
    };

    this.handleClick = this.handleClick.bind(this);
  }

  changeHandler = event => {
    const key = event.target.name;
    const value = event.target.value;

    this.setState({
      inputs: {
        ...this.state.inputs,
        [key]: {
          ...this.state.inputs[key],
          value
        }
      },
      output: ""
    });
  };

  transferELA() {
    const endpoint = "console/transferELADemo";
    axios
      .get(baseUrl + endpoint, {
        headers: {
          api_key: this.state.inputs.apiKey.value,
          "Content-Type": "application/json;"
        }
      })
      .then(response => {
        this.setState({
          isTransferred: response.data.status === 200,
          status: "SUCCESS",
          output: JSON.stringify(response.data, null, 2)
        });
      })
      .catch(error => {
        // Error
        if (error.response) {
          // The request was made and the server responded with a status code
          // that falls out of the range of 2xx
          // console.log(error.response.data);
          //console.log(error.response.status);
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
          console.log(error.request);
        } else {
          // Something happened in setting up the request that triggered an Error
          console.log("Error", error.message);
        }
      });
  }

  handleClick() {
    //TODO:
    //Do we need to generate a new key every time the button gets clicked?
    this.transferELA();
  }

  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={6}>
              <Card
                title="Transfer ELA Demostration"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormGroup>
                          <ControlLabel>API Key</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Enter your API Key here"
                            name="apiKey"
                            value={this.state.inputs.apiKey.value}
                            onChange={this.changeHandler}
                          />
                          <br />
                          <Button
                            onClick={this.handleClick}
                            variant="primary"
                            size="lg"
                          >
                            Transfer ELA
                          </Button>
                        </FormGroup>
                      </Col>
                    </Row>
                    <div className="clearfix" />
                  </form>
                }
              />
            </Col>
            <Col md={6}>
              {this.state.output && (
                <Card
                  title="Output"
                  content={
                    <form>
                      <Row>
                        <Col md={12}>
                          <ControlLabel>
                            Status : {this.state.status}
                          </ControlLabel>
                          <FormControl
                            rows="7"
                            componentClass="textarea"
                            bsClass="form-control"
                            name="output"
                            value={this.state.output}
                            readOnly
                          />
                        </Col>
                      </Row>
                      <div className="clearfix" />
                    </form>
                  }
                />
              )}
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
                            TransferELADemo API creates a new wallet and
                            transfer 100 ELA from a pre-loaded wallet to the
                            newly created one. Returns the sender's address,
                            receiver's address, transaction id and status.
                          </p>
                        </FormGroup>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`GET /api/1/console/transferELADemo HTTP/1.1
Host: localhost:8888

headers:{
    "api_key":KHBOsth7b3WbOTVzZqGUEhOY8rPreYFM,
}    
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
    "sender": [
        {
            "address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
            "transferred_amount": "100"
        }
    ],
    "receiver": [
        {
            "privateKey": "84B1604D8662E274C03B352B951591849B9C20BB746AB19ECB63FC2B1249FD31",
            "publicKey": "034EB90D6BF5A70D7987EF5581A3F71722B3A41E3D0E2FE0E68FB1D025D112DD19",
            "address": "EbJomRVYUSX6ZaW4GaHvAVy2BLTQ7GrQCj"
        }
    ],
    "transaction_id": "11c08fbdb1961b7c1fb8fc06a2e1c4c7d309c220e12d8b6fc0cecb0ce671ab25",
    "status": 200
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
                          {`    api_key = request.headers.get('api_key')
    api_status = validate_api_key(api_key)
    if not api_status:
      data = {"error message":"API Key could not be verified","status":401, "timestamp":getTime(),"path":request.url}
      return Response(json.dumps(data), 
        status=401,
        mimetype='application/json'
      )

    #create a wallet
    api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_CREATE
    myResponse1 = requests.get(api_url_base).json()
    if myResponse1['status'] != 200:
      data = {"error message":"Wallet could not be created","status":404, "timestamp":getTime(),"path":request.url}
      return Response(json.dumps(data), 
          status=404,
          mimetype='application/json'
        )

    #transfer ELA
    api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_TRANSFER
    headers = {'Content-type': 'application/json'}
    req_data = {
              "sender":[
                  {
                      "address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWwef9095s",
                      "privateKey":"109a5fb2b7c7abd0f2fa90b0a2wefew5e27de7ewfwe768ab043r4a47a1dd25da1f68a8"
                  }
              ],
              "memo":"测试",
              "receiver":[
                  {
                      "address":myResponse1['result']['address'],
                      "amount":"100"
                  }
              ]
          }
    myResponse2 = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
    json_output =   {
              "sender":[
                    {
                      "address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
                      "transferred_amount":"100"
                    }
                  ],
                  "receiver":[
                  {
                    "privateKey":myResponse1['result']['privateKey'],
                    "publicKey":myResponse1['result']['publicKey'],
                      "address":myResponse1['result']['address']
                    }
                  ],
                  "transaction_id": myResponse2['result'],
                "status": myResponse2['status']
            }
    return Response(json.dumps(json_output), 
        status=myResponse2['status'],
        mimetype='application/json'
      )`}
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

export default UserProfile;
