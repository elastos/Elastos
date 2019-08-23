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
  FormControl,
  ProgressBar
} from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";

import "../assets/css/fileupload.css";
import axios from "axios";
import { baseUrl } from "../utils/api";

class UserProfile extends Component {
  constructor() {
    super();
    this.state = {
      apiKey: "",
      prvKey: "",
      selectedFile: null,
      output: "",
      loaded: 0
    };

    this.handleClick = this.handleClick.bind(this);
  }

  changeHandler = event => {
    if (event.target.name === "apiKey") {
      this.setState({
        apiKey: event.target.value
      });
    } else if (event.target.name === "prvKey") {
      this.setState({
        prvKey: event.target.value
      });
    } else {
      this.setState({
        selectedFile: event.target.files[0]
      });
    }
  };

  uploadFileToServer() {
    let formdata = new FormData();
    formdata.append("file", this.state.selectedFile);

    const endpoint = "console/uploadAndSign";

    axios
      .post(baseUrl + endpoint, formdata, {
        onUploadProgress: ProgressEvent => {
          this.setState({
            loaded: (ProgressEvent.loaded / ProgressEvent.total) * 100
          });
        },
        headers: {
          api_key: this.state.apiKey,
          private_key: this.state.prvKey,
          "Content-Type":
            "multipart/form-data; boundary=--------------------------942705689964164935672351"
        }
      })
      .then(response => {
        this.setState({
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
            status: "FAILURE",
            output: error.request
          });
          console.log(error.request);
        } else {
          // Something happened in setting up the request that triggered an Error
          this.setState({
            status: "FAILURE",
            output: error.message
          });
          console.log("Error", error.message);
        }
      });
  }

  handleClick() {
    //TODO:
    //1.check for the api key
    this.uploadFileToServer();
  }

  render() {
    return (
      <div className="content">
        <Grid fluid>
          <Row>
            <Col md={6}>
              <Card
                title="File Upload"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <form method="post" action="#" id="#">
                          <ControlLabel>API Key</ControlLabel>
                          <FormControl
                            rows="3"
                            componentClass="textarea"
                            bsClass="form-control"
                            placeholder="Enter your API Key here"
                            name="apiKey"
                            value={this.state.apiKey}
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
                            value={this.state.prvKey}
                            onChange={this.changeHandler}
                          />
                          <br />

                          <ControlLabel>Upload Your File </ControlLabel>
                          <div className="form-group files">
                            <label />
                            <input
                              type="file"
                              name="file"
                              onChange={this.changeHandler}
                            />
                          </div>

                          <ProgressBar
                            striped
                            variant="success"
                            now={this.state.loaded}
                            label={`${Math.round(this.state.loaded, 2)}%`}
                          />
                          <br />
                          <Button
                            variant="primary"
                            size="lg"
                            onClick={this.handleClick}
                          >
                            Upload
                          </Button>
                        </form>
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
                    <Row>
                      <Col md={12}>
                        <ControlLabel>
                          Status : {this.state.status}
                        </ControlLabel>
                        <FormControl
                          rows="10"
                          componentClass="textarea"
                          bsClass="form-control"
                          name="output"
                          value={this.state.output}
                          readOnly
                        />
                      </Col>
                    </Row>
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
                            UploadAndSign API uploads the request file to hive
                            and signs the content of the file using DID
                            sidechain.
                          </p>
                        </FormGroup>
                        <SyntaxHighlighter
                          language="javascript"
                          style={gruvboxDark}
                        >
                          {`POST /api/1/console/uploadAndSign HTTP/1.1
Host: localhost:8888
Content-Type: multipart/form-data; boundary=--------------------------942705689964164935672351

headers:{
    "api_key":KHBOsth7b3WbOTVzZqGUEhOY8rPreYFM,
    "private_key":039C9EF3BD38C8E677BFH398R32F40A998F8872ADC23R32UHR89DE2A21631310E2F200E43B3,
}

request.files{
    "file":"sample.txt"
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
    "result": {
        "msg": "516D61527843366D62444B3546786F785757414C704D546A4B705662443938466A574A756D737469694439364A6D",
        "pub": "039C9EF3BD38C8E677B2F40A998F8872ADC9D9D5A89CE2A21631310E2F200E43B3",
        "sig": "F2827C10BBB1C065AB558828E134FD9FE5F080D63233344FEB7DA162EBF8EECA8BE6A099E584DA75E2EB4E6698F4E8491BB11657EFA33B21958B05D205C046C1",
        "hash": "QmaRxC6mbDK5FxoxWWALpMTjKpVbD98FjWJumstiiD96Jm"
    },
    "status": 200
}`}
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

    #reading the file content
    request_file = request.files['file']
    if not request_file:
      data = {"error message":"No file attached","status":404, "timestamp":getTime(),"path":request.url}
      return Response(json.dumps(data), 
        status=404,
        mimetype='application/json'
      )

    file_contents = request_file.stream.read().decode("utf-8")

    #upload file to hive
    api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.HIVE_ADD
    headers = {'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'}
    myResponse1 = requests.get(api_url_base, files={'file':file_contents}, headers=headers).json()
    if not myResponse1:
      data = {"error message":"File could not be uploaded","status":404, "timestamp":getTime(),"path":request.url}
      return Response(json.dumps(data), 
        status=404,
        mimetype='application/json'
      )


    #signing the hash key
    private_key = request.headers.get('private_key')
    api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SIGN
    headers = {'Content-type': 'application/json'}
    req_data =  {
                "privateKey":private_key,
                "msg":myResponse1['Hash']
            }
    myResponse2 = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
    myResponse2['result']['hash'] = myResponse1['Hash']
    return Response(json.dumps(myResponse2), 
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
