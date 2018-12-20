import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
    Form,
    Icon,
    Input,
    Popconfirm,
    message,
    Button,
    Checkbox,
    Select,
    DatePicker,
    Row,
    Col,
    Upload,
    Spin,
    Divider

} from 'antd'

import {upload_file} from "@/util";
import './style.scss'
import moment from 'moment/moment'

const FormItem = Form.Item
const TextArea = Input.TextArea

/**
 * This is form for members to apply to come to the 2018 Anniversary Event - Chiang Mai
 *
 * TODO: this will later allow them to apply to be a vote candidate
 */
class C extends BaseComponent {

    handleSubmit (e) {
        e.preventDefault()

        this.props.form.validateFields((err, values) => {

            if (!err) {
                this.props.submitForm(values, this.state);
            }
        })
    }

    constructor (props) {
        super(props)

        this.state = {

            rulesAgreed: false,

            // init as loading while we check for an existing submission
            loading: true,

            _id: null,

            attachment_url: null,
            attachment_loading: false,
            attachment_filename: '',
            attachment_type: '',

            removeAttachment: false,

            passport_url: null,
            passport_loading: false,
            passport_filename: '',
            passport_type: '',

            removePassport: false
        }
    }

    async componentDidMount() {

        const existingSubmission = await this.props.getExistingSubmission()

        if (!existingSubmission) {
            await this.setState({
                loading: false
            })
            return
        }

        this.setState({

            rulesAgreed: true,
            loading: false,

            _id: existingSubmission._id,

            fullLegalName: existingSubmission.fullLegalName,

            attachment_url: existingSubmission.attachment,
            attachment_filename: existingSubmission.attachmentFilename,
            attachment_type: existingSubmission.attachmentType,

            removeAttachment: false,

            passport_url: existingSubmission.passportUpload,
            passport_filename: existingSubmission.passportFilename,
            passport_type: existingSubmission.passportUploadType,

            removePassport: false
        })

        this.props.form.setFieldsValue({readRules: true})
    }

    async removeAttachment() {
        await this.setState({
            attachment_loading: false,
            attachment_url : null,
            attachment_type: '',
            attachment_filename: '',

            removeAttachment: true
        })

        // message.success('File removed')
    }

    async removePassport() {
        await this.setState({
            passport_loading: false,
            passport_url : null,
            passport_type: '',
            passport_filename: '',

            removePassport: true
        })

        // message.success('Passport file removed')
    }

    getInputProps () {

        const {getFieldDecorator} = this.props.form

        // I have read the rules - TODO: disable if
        const readRules_fn = getFieldDecorator('readRules', {
            rules: [
                {required: true, message: 'You must agree to the rules and requirements'},
            ]
        })
        const readRules_el = (
            <Checkbox/>
        )

        // name
        const fullLegalName_fn = getFieldDecorator('fullLegalName', {
            initialValue: this.state.fullLegalName || this.props.user.profile.firstName + ' ' + this.props.user.profile.lastName,
            rules: [
                {required: true, message: 'Please input an name'},
                {min: 6, message: 'name too short'}
            ]
        })
        const fullLegalName_el = (
            <Input size="large"/>
        )

        // walletAddress
        const walletAddress_fn = getFieldDecorator('walletAddress', {
            initialValue: this.props.user.profile.walletAddress || '',
            rules: [
                {required: true, message: 'Please input a wallet address'},
                {len: 34, message: 'address length error'}
            ]
        })
        const walletAddress_el = (
            <Input size="large"/>
        )

        const attachment_fn = getFieldDecorator('attachment', {
            rules: []
        });
        const p_attachment = {
            showUploadList: false,
            customRequest :(info)=>{
                this.setState({
                    attachment_loading: true
                });
                upload_file(info.file).then((d)=>{
                    const url = d.url;
                    this.setState({
                        attachment_loading: false,
                        attachment_url : url,
                        attachment_type: d.type,
                        attachment_filename: d.filename,

                        removeAttachment: false
                    });
                })
            }
        };
        const attachment_el = (
            <Upload name="attachment" {...p_attachment}>
                {
                    this.state.attachment_url ? (
                        <a target="_blank" href={this.state.attachment_url}>
                            {this.state.attachment_type === 'application/pdf' ?
                                <Icon type="file-pdf"/> :
                                <Icon type="file"/>
                            } &nbsp;
                            {this.state.attachment_filename}
                        </a>
                    ) : (
                        <Button className="mobileBtn" loading={this.state.attachment_loading}>
                            <Icon type="upload" /> Upload Flight Receipt
                        </Button>
                    )
                }
            </Upload>
        );

        const passport_fn = getFieldDecorator('passport', {
            rules: []
        });
        const p_passport= {
            showUploadList: false,
            customRequest :(info)=>{
                this.setState({
                    passport_loading: true
                });
                upload_file(info.file).then((d)=>{
                    const url = d.url;
                    this.setState({
                        passport_loading: false,
                        passport_url : url,
                        passport_type: d.type,
                        passport_filename: d.filename,

                        removePassport: false
                    });
                })
            }
        };
        const passport_el = (
            <Upload name="passport" {...p_passport}>
                {
                    this.state.passport_url ? (
                        <a target="_blank" href={this.state.passport_url}>
                            {this.state.passport_type === 'application/pdf' ?
                                <Icon type="file-pdf"/> :
                                <Icon type="file"/>
                            } &nbsp;
                            {this.state.passport_filename}
                        </a>
                    ) : (
                        <Button className="mobileBtn" loading={this.state.passport_loading}>
                            <Icon type="upload" /> Upload Photo of Passport
                        </Button>
                    )
                }
            </Upload>
        );

        return {
            readRules: readRules_fn(readRules_el),

            fullLegalName: fullLegalName_fn(fullLegalName_el),
            walletAddress: walletAddress_fn(walletAddress_el),

            attachment: attachment_fn(attachment_el),
            passport: passport_fn(passport_el)
        }
    }

    ord_render () {
        const {getFieldDecorator} = this.props.form
        const p = this.getInputProps()

        const formItemLayout = {
            labelCol: {
                xs: {span: 24},
                md: {span: 8},
            },
            wrapperCol: {
                xs: {span: 24},
                md: {span: 12},
            }
        }

        const formItemNoLabelLayout = {
            wrapperCol: {
                xs: {span: 24},
                md: {offset: 6, span: 12},
            },
        }

        const textLayout = {
            xs: {
                span: 24
            },
            md: {
                offset: 6,
                span: 12
            }
        }


        // const existingTask = this.props.existingTask

        // TODO: terms of service checkbox\

        // TODO: react-motion animate slide left

        // TODO: description CKE Editor

        return <div className="c_anniversaryAppFormContainer">

            <Form onSubmit={this.handleSubmit.bind(this)}>
                <div>
                    <Divider>Details and Rules</Divider>
                    <Row>
                        <Col {...textLayout} className="left-align">

                            <h5 style={{color: '#47aaa7'}}>
                                If you did not receive an invitation to fill out this form directly via email,<br/>
                                you are not on the invitiation list and please do not apply.
                            </h5>

                            <Row>
                                <Col span={8}>
                                    <h4>Location:</h4>
                                </Col>
                                <Col span={16} style={{paddingLeft: 15}}>
                                    Shangri-La Hotel, Chiang Mai, Thailand
                                </Col>
                            </Row>
                            <Row>
                                <Col span={8}>
                                    <h4>Event Dates:</h4>
                                </Col>
                                <Col span={16} style={{paddingLeft: 15}}>
                                    Aug 24 - 27, 2018
                                </Col>
                            </Row>
                            {/*
                            <Row>
                                <Col span={8}>
                                    <h4>Registration Deadline:</h4>
                                </Col>
                                <Col span={16} style={{paddingLeft: 15}}>
                                    <b>July 18, 2018 - 11:59pm PDT</b>
                                </Col>
                            </Row>*/}
                            <Row>
                                <Col span={8}>
                                    <h4>Airfare/Passport Upload Deadline:</h4>
                                </Col>
                                <Col span={16} style={{paddingLeft: 15}}>
                                    <b style={{color: '#E3170D'}}>Aug 9, 2018 - 12:00pm CST (Beijing)</b>
                                </Col>
                            </Row>

                            <br/>

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
                                            <Col span={18} style={{paddingLeft: 15}}>
                                                Conference Sign In
                                            </Col>
                                        </Row>
                                        <Row>
                                            <Col span={6} className="strong-text">
                                                - 09:00 - 11:30
                                            </Col>
                                            <Col span={18} style={{paddingLeft: 15}}>
                                                Main Event / Ceremony
                                            </Col>
                                        </Row>
                                        <Row>
                                            <Col span={6} className="strong-text">
                                                - 11:30 - 13:30
                                            </Col>
                                            <Col span={18} style={{paddingLeft: 15}}>
                                                Buffet Lunch
                                            </Col>
                                        </Row>
                                        <Row>
                                            <Col span={6} className="strong-text">
                                                - 14:00 - 17:00
                                            </Col>
                                            <Col span={18} style={{paddingLeft: 15}}>
                                                dApp Meetups & Presentations
                                            </Col>
                                        </Row>
                                        <Row>
                                            <Col span={6} className="strong-text">
                                                - 18:00 - 20:00
                                            </Col>
                                            <Col span={18} style={{paddingLeft: 15}}>
                                                Dinner
                                            </Col>
                                        </Row>
                                        <Row>
                                            <Col span={6} className="strong-text">
                                                - 20:30 - 22:00
                                            </Col>
                                            <Col span={18} style={{paddingLeft: 15}}>
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
                                            <Col span={18} style={{paddingLeft: 15}}>
                                                America Developer Community Webcast
                                            </Col>
                                        </Row>
                                        <Row>
                                            <Col span={6} className="strong-text">
                                                - 11:00 - 18:30
                                            </Col>
                                            <Col span={18} style={{paddingLeft: 15}}>
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
                            - Airfare receipt due by Aug 9, 12pm CST (Beijing time) for reimbursement at the event
                        </Col>
                    </Row>

                    <br/>

                    {this.state.loading ?
                        <div className="center"><Spin size="large"/></div> :
                        (!this.props.is_login ?
                        <div className="center">
                            <br/>
                            <span class="strong-text">You must be logged in to apply to come to the 2018 Anniversary Event</span><br/>
                            <br/>
                            <Button onClick={() => {
                                sessionStorage.setItem('loginRedirect', '/form/anniversary2018')
                                this.props.history.push('/login')
                            }}>Login</Button>
                            <Button onClick={() => this.props.history.push('/register')}>Register</Button>
                        </div> : <div>

                            {!this.state._id &&
                            < FormItem {...formItemNoLabelLayout}>
                                {p.readRules} <b>I have read and agree to the rules <span style={{color: 'red'}}>*</span> </b>
                            </FormItem>
                            }

                            <Divider>Application</Divider>
                            <Row>
                                <Col xs={{span: 24}} md={{span: 6}} className="right-align static-field">
                                    Email:
                                </Col>
                                <Col xs={{span: 24}} md={{span: 12}} className="static-field content">
                                    {this.props.user.email}
                                </Col>
                            </Row>
                            <FormItem label="Full Legal Name" {...formItemLayout}>
                                {p.fullLegalName}
                            </FormItem>

                            <Divider></Divider>

                            {this.props.user.profile.walletAddress ?
                                <div>
                                    <Row>
                                        <Col {...textLayout} >
                                            Please ensure this is your wallet address, this is managed under your profile
                                        </Col>
                                    </Row>
                                    <Row>
                                        <Col xs={{span: 24}} md={{span: 6}} className="right-align static-field">
                                            ELA Address:
                                        </Col>
                                        <Col xs={{span: 24}} md={{span: 12}}  className="static-field content">
                                            <b>{this.props.user.profile.walletAddress}</b>
                                        </Col>
                                    </Row>
                                </div> :
                                <div>
                                    <Row>
                                        <Col offset="6" span="12">
                                            Please note this wallet address will be added to your profile
                                        </Col>
                                    </Row>
                                    <FormItem label="ELA Wallet Address" {...formItemLayout}>
                                        {p.walletAddress}
                                    </FormItem>
                                </div>
                            }

                            <Divider>
                                Upload Your Airfare Receipt & Photo of Passport Info Page
                            </Divider>

                            <Row>
                                <Col {...textLayout} className="left-align">
                                    If you do not upload it now you will need to upload it by Aug 9th
                                    <br/>
                                    <br/>
                                </Col>
                            </Row>

                            {this.state.attachment_url ?
                                <Row>
                                    <Col offset={6} span={12}>
                                        <a target="_blank" href={this.state.attachment_url}>
                                            Airfare: {this.state.attachment_filename}
                                        </a>

                                        &nbsp;

                                        <Popconfirm title="Are you sure you want to remove this?" placement="top" okText="Yes" onConfirm={this.removeAttachment.bind(this)}>
                                            <Icon type="delete" style={{cursor: 'pointer'}}/>
                                        </Popconfirm>
                                    </Col>
                                </Row> :

                                <div>
                                    <FormItem {...formItemNoLabelLayout}>
                                        {p.attachment}
                                    </FormItem>
                                </div>
                            }

                            <br/>

                            {this.state.passport_url ?
                                <Row>
                                    <Col offset={6} span={12}>
                                        <a target="_blank" href={this.state.passport_url}>
                                            Passport: {this.state.passport_filename}
                                        </a>

                                        &nbsp;

                                        <Popconfirm title="Are you sure you want to remove this?" placement="top" okText="Yes" onConfirm={this.removePassport.bind(this)}>
                                            <Icon type="delete" style={{cursor: 'pointer'}}/>
                                        </Popconfirm>
                                    </Col>
                                </Row> :

                                <div>
                                    <FormItem {...formItemNoLabelLayout}>
                                        {p.passport}
                                    </FormItem>
                                </div>
                            }

                            <Divider/>

                            <FormItem wrapperCol={{xs: {span: 24, offset: 0}, md: {span: 12, offset: 6}}}>
                                <Button loading={this.props.loading} type="ebp" htmlType="submit" className="mobileBtn d_btn">
                                    {this.state._id ? 'Save Updates' : 'Submit'}
                                </Button>
                            </FormItem>

                            <Row>
                                <Col {...textLayout} className="static-field content">
                                    Please contact <a href="mailto:cyberrepublic@elastos.org">cyberrepublic@elastos.org</a> if you have any issues.
                                </Col>
                            </Row>
                        </div>
                        )
                    }
                </div>
            </Form>
        </div>

    }

}
export default Form.create()(C)
