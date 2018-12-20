import React from 'react';
import {upload_file} from "@/util"
import { Col, Row, Upload, Button, Icon, message, Popconfirm, Divider } from 'antd'
import SubmissionService from '@/service/SubmissionService'

export default function() {

    const submissionService = new SubmissionService()


    const removeAttachment = async () => {
        this.setState({
            attachment_loading: false,
            attachment_url : null,
            attachment_type: '',
            attachment_filename: '',

            removeAttachment: true
        })

        await submissionService.update(this.props.submission._id, {
            attachment: null,
            attachmentType: '',
            attachmentFilename: ''
        })

        message.success('File removed')
    }

    const p_attachment = {
        showUploadList: false,
        customRequest :(info)=>{
            this.setState({
                attachment_loading: true
            });
            upload_file(info.file).then(async (d)=>{
                const url = d.url;
                this.setState({
                    attachment_loading: false,
                    attachment_url : url,
                    attachment_type: d.type,
                    attachment_filename: d.filename,

                    removeAttachment: false
                });

                // do the upload immediately
                try {
                    await submissionService.update(this.props.submission._id, {
                        attachment: url,
                        attachmentType: d.type,
                        attachmentFilename: d.filename
                    })

                    message.success('Receipt uploaded successfully')

                } catch (err) {
                    console.error(err)

                    this.setState({
                        attachment_loading: false,
                        attachment_url : null,
                        attachment_type: '',
                        attachment_filename: ''
                    })

                    message.success('There was a problem uploading this file, please try again')
                }
            })
        }
    };

    return <div>
        <Row>
            <Col>
                <h4 className="center">
                    {this.props.submission.title}
                </h4>
            </Col>
        </Row>
        {this.props.page !== 'ADMIN' &&
        <div className="c_anniversaryAppFormContainer">
            <Row>
                <Col offset={6} span={12} className="left-align">

                    <Row>
                        <Col span={8}>
                            <h4>Location:</h4>
                        </Col>
                        <Col span={16}>
                            Shangri-La Hotel, Chiang Mai, Thailand
                        </Col>
                    </Row>
                    <Row>
                        <Col span={8}>
                            <h4>Event Dates:</h4>
                        </Col>
                        <Col span={16}>
                            Aug 24 - 27, 2018
                        </Col>
                    </Row>
                    <Row>
                        <Col span={8}>
                            <h4>Registration Deadline:</h4>
                        </Col>
                        <Col span={16}>
                            <b>July 18, 2018 - 11:59pm PDT</b>
                        </Col>
                    </Row>

                    <h5>Dear Community Member,</h5>

                    <p>
                        Elastos is hosting a One Year Anniversary Event in Thailand and you are invited as a selected
                        community member. While you are responsible for paying for your own airfare, <u>you will receive an
                        reimbursement in ELA equal to the cost of airfare for coming and will be provided hotel accommodations.</u>
                    </p>

                    <h4>Itinerary</h4>

                    <ul className="itinerary">
                        <li><h5 style={{paddingTop: 0}}>August 24: <span style={{color: 'rgba(0, 0, 0, 0.85)'}}>Arrival and Check-In</span></h5></li>
                        <li>
                            <h5>August 25: <span style={{color: 'rgba(0, 0, 0, 0.85)'}}>Anniversary of Elastos - Main Event</span></h5>

                            <div style={{marginLeft: '5%'}}>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 08:30 - 08:55
                                    </Col>
                                    <Col span={18}>
                                        Conference Sign In
                                    </Col>
                                </Row>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 09:00 - 11:30
                                    </Col>
                                    <Col span={18}>
                                        Main Event / Ceremony
                                    </Col>
                                </Row>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 11:30 - 13:30
                                    </Col>
                                    <Col span={18}>
                                        Buffet Lunch
                                    </Col>
                                </Row>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 14:00 - 17:00
                                    </Col>
                                    <Col span={18}>
                                        dApp Meetups & Presentations
                                    </Col>
                                </Row>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 18:00 - 20:00
                                    </Col>
                                    <Col span={18}>
                                        Dinner
                                    </Col>
                                </Row>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 20:30 - 22:00
                                    </Col>
                                    <Col span={18}>
                                        European Developer Community Webcast
                                    </Col>
                                </Row>
                            </div>
                        </li>
                        <li>
                            <h5>August 26: <span style={{color: 'rgba(0, 0, 0, 0.85)'}}>Community Building & Chiang Mai Tour</span></h5>

                            <div style={{marginLeft: '5%'}}>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 09:30 - 10:30
                                    </Col>
                                    <Col span={18}>
                                        America Developer Community Webcast
                                    </Col>
                                </Row>
                                <Row>
                                    <Col span={6} className="strong-text">
                                        - 11:00 - 18:30
                                    </Col>
                                    <Col span={18}>
                                        Chiang Mai Town Tour
                                    </Col>
                                </Row>
                            </div>
                        </li>
                        <li>
                            <h5>August 27: <span style={{color: 'rgba(0, 0, 0, 0.85)'}}>Hotel Check-Out</span></h5>
                        </li>
                    </ul>

                    <br/>

                    <h4>Details</h4>

                    <p>
                        We look forward to hosting our team and selected community members to celebrate our first year and to
                        look forward to many more to come. We value our community with a passion and would love to see you
                        attend and celebrate Elastos in style.
                    </p>

                    <br/>
                    <span style={{fontWeight: 600}}>
                                    You are required to do the following:
                                </span><br/>
                    - Book your own flight, you will be compensated in ELA and a receipt will be required.<br/>
                    - Hotels will be provided by us and reserved under the name provided on this form<br/>
                    <br/>
                    <h4>Rules</h4>
                    - Flight must be a round-trip economy class flight at a reasonable cost<br/>
                    - ELA exchange rate is determined at time of 12pm local time in Thailand on the day of 25th August, 2018<br/>
                    - You must arrive in Chiang Mai by August 24, the official event is on the 25th<br/>
                    - You are responsible for your own Visa requirements based on your nationality<br/>
                    <u>- You must not represent yourself as an employee of Elastos, your purpose of travel to Thailand must be for tourism only</u><br/>
                    - If for whatever reason - except medical emergencies with proof - you will not receive reimbursement if you do not attend the event<br/>
                    - Airfare receipt due by Aug 25, 12pm local time in Thailand for reimbursement
                </Col>
            </Row>

            <br/>

            <Divider>
                Your Registration - You Can Always Add/Modify the Airfare Receipt
            </Divider>
        </div>}

        <Row>
            <Col className="col-label" span={8}>
                Email
            </Col>
            <Col span={12}>
                {this.props.submission.email}
            </Col>
        </Row>
        <Row>
            <Col className="col-label" span={8}>
                Full Legal Name
            </Col>
            <Col span={12}>
                {this.props.submission.fullLegalName}
            </Col>
        </Row>
        <Row>
            <Col className="col-label" span={8}>
                Wallet Address
            </Col>
            <Col span={12}>
                {this.props.submission.createdBy.profile.walletAddress}
            </Col>
        </Row>
        <Row>
            <Col className="col-label" span={8}>
                Airfare Receipt
            </Col>
            <Col span={12}>
                {this.state.attachment_url ? (
                    <div>
                        <a target="_blank" href={this.state.attachment_url}>
                            {this.state.attachment_filename}
                        </a>

                        &nbsp;

                        <Popconfirm title="Are you sure you want to remove this receipt?" placement="top" okText="Yes" onConfirm={removeAttachment.bind(this)}>
                            <Icon type="delete" style={{cursor: 'pointer'}}/>
                        </Popconfirm>
                    </div>
                ) : (
                    <Upload name="attachment" {...p_attachment}>
                        <Button loading={this.state.attachment_loading}>
                            <Icon type="upload" /> Click to upload
                        </Button>
                    </Upload>
                )}
            </Col>
        </Row>
    </div>

}
