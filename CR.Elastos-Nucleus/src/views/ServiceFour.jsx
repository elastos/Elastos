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

class UserProfile extends Component {
  constructor() {
    super();

    this.state = {
      inputs: {
        prvKey: "",
        message: "",
        hashKey: "",
        pubKey: "",
        sign: "",
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

  verifyAndShowMessage() {
    const endpoint = "console/verifyAndShow";
    axios
      .post(
        baseUrl + endpoint,
        {
          msg: this.state.inputs.message.value,
          pub: this.state.inputs.pubKey.value,
          sig: this.state.inputs.sign.value,
          hash: this.state.inputs.hashKey.value
        },
        {
          headers: {
            api_key: this.state.inputs.apiKey.value,
            private_key: this.state.inputs.prvKey.value,
            "Content-Type": "application/json;"
          }
        }
      ).then(response => {
                  this.setState({
                    status: "SUCCESS",
                    output: response.data
                  });
          })
          .catch((error) => {
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
                  })
              } else if (error.request) {
                  // The request was made but no response was received
                  // `error.request` is an instance of XMLHttpRequest in the
                  // browser and an instance of
                  // http.ClientRequest in node.js
                  this.setState({
                      status: "FAILURE",
                      output: error.request
                  })
                  console.log(error.request);
              } else {
                  // Something happened in setting up the request that triggered an Error
                  this.setState({
                      status: "FAILURE",
                      output: error.message
                  })
                  console.log('Error', error.message);
              }


          });

  }

  handleClick() {
    //TODO:
    //1.check for the api key
    this.verifyAndShowMessage();
  }

  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={6}>
              <Card
                title="Verify And Show File Content"
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
                          <ControlLabel>Private Key</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Enter your Private Key here"
                            name="prvKey"
                            value={this.state.inputs.prvKey.value}
                            onChange={this.changeHandler}
                          />
                          <br />
                          <ControlLabel>Message</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Enter the message hash here"
                            name="message"
                            value={this.state.inputs.message.value}
                            onChange={this.changeHandler}
                          />
                          <br />
                          <ControlLabel>Public Key</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            name="pubKey"
                            placeholder="Enter your public key here"
                            value={this.state.inputs.pubKey.value}
                            onChange={this.changeHandler}
                          />
                          <br />
                          <ControlLabel>Signature</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            name="sign"
                            placeholder="Enter your signature here"
                            value={this.state.inputs.sign.value}
                            onChange={this.changeHandler}
                          />
                          <br />
                          <ControlLabel>Hash Key of File</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            name="hashKey"
                            placeholder="Enter your hash key of file here"
                            value={this.state.inputs.hashKey.value}
                            onChange={this.changeHandler}
                          />
                          <br />
                          <Button
                            onClick={this.handleClick}
                            variant="primary"
                            size="lg"
                          >
                            Verify
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
                  title="Message content"
                  content={
                    <form>
                      <Row>
                        <Col md={12}>
                          <ControlLabel>Status : {this.state.status}</ControlLabel>
                          <FormControl
                            rows="5"
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
                            VerifyAndShow API verifies the message with the DID sidechain. If the verification succeeds, then it shows the content stored from the elastos hive. 
                          </p>
                        </FormGroup>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`POST /api/1/console/verifyAndShow HTTP/1.1
Host: localhost:8888
Content-Type: application/json

headers:{
    "api_key":KHBOsth7b3WbOTVzZqGUEhOY8rPreYFM,
    "private_key":039C9EF3BD38C8E677BFH398R32F40A998F8872ADC23R32UHR89DE2A21631310E2F200E43B3,
}

request.body
{
    "msg": "516D61527843366D62444B3546786F785757414C704D546A4B705662443938466A574A756D737469694439364A6D",
    "pub": "039C9EF3BD38C8E677B2F40A998F8872ADC9D9D5A89CE2A21631310E2F200E43B3",
    "sig": "F2827C10BBB1C065AB558828E134FD9FE5F080D63233344FEB7DA162EBF8EECA8BE6A099E584DA75E2EB4E6698F4E8491BB11657EFA33B21958B05D205C046C1",
    "hash": "QmaRxC6mbDK5FxoxWWALpMTjKpVbD98FjWJumstiiD96Jm"
}
`}
                        </SyntaxHighlighter>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`HTTP/1.1 200 OK
Vary: Accept
Content-Type: Text

Hello World
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

    #verify the hash key
    api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_VERIFY
    headers = {'Content-type': 'application/json'}
    req_data = request.get_json()
    signed_message = req_data['msg']
    file_hash = req_data['hash']
    json_data = {
            "msg": req_data['msg'],
            "pub": req_data['pub'],
              "sig": req_data['sig']
          }
    myResponse1 = requests.post(api_url_base, data=json.dumps(json_data), headers=headers).json()
    if not myResponse1['result']:
      data = {"error message":"Hask key could not be verified","status":404, "timestamp":getTime(),"path":request.url}
      return Response(json.dumps(data), 
        status=404,
        mimetype='application/json'
      )

    #verify the given input message using private key
    private_key = request.headers.get('private_key')
    api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SIGN
    headers = {'Content-type': 'application/json'}
    req_data =  {
                "privateKey":private_key,
                "msg":req_data['hash']
            }
    myResponse2 = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
    if myResponse2['result']['msg'] != signed_message:
      data = {"error message":"Hash Key and messsage could not be verified","status":401, "timestamp":getTime(),"path":request.url}
      return Response(json.dumps(data), 
        status=401,
        mimetype='application/json'
      )

    #show content
    api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.SHOW_CONTENT + "{}"
    myResponse = requests.get(api_url_base.format(file_hash))
    return Response(myResponse, 
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

export default UserProfile;
