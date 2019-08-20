import React, { Component } from "react";
import Dropzone from "react-dropzone";
import SyntaxHighlighter from "react-syntax-highlighter";
import { gruvboxDark } from "react-syntax-highlighter/dist/esm/styles/hljs";

import { Grid, Row, Col, FormGroup } from "react-bootstrap";

import { Card } from "components/Card/Card.jsx";

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
                        <Dropzone
                          onDrop={acceptedFiles => console.log("test1")}
                        >
                          {({ getRootProps, getInputProps }) => (
                            <section>
                              <div {...getRootProps()}>
                                <input {...getInputProps()} />
                                <p>Select or Drop your file here</p>
                              </div>
                            </section>
                          )}
                        </Dropzone>
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
                title="Documentation"
                content={
                  <form>
                    <Row>
                      <Col md={12}>
                        <FormGroup controlId="formControlsTextarea">
                          <p>
                            <span className="category" />
                            UploadAndSign API uploads the request file to hive and signs the content of the file using DID sidechain.
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
    "api_key":564732BHU,
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
