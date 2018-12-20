import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
    Form,
    Icon,
    Input,
    PopConfirm,
    message,
    Button,
    Checkbox,
    Select,
    DatePicker,
    Row,
    Col,
    Upload,
    Cascader,
    Divider

} from 'antd'

import {upload_file} from "@/util";
import './style.scss'

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

        this.props.form.validateFields(async (err, values) => {

            if (!err) {
                if (!this.state.attachment_url) {
                    message.error('You must upload a video')
                    return
                }

                try {
                    await this.props.submitForm(values, this.state);
                } catch (err) {
                    console.error(err)
                    message.error('There was a problem submitting thing, please refresh and try again')
                }

            }
        })
    }

    constructor (props) {
        super(props)

        this.state = {
            attachment_url: null,
            attachment_loading: false,
            attachment_filename: '',
            attachment_type: '',

            removeAttachment: false
        }
    }

    getInputProps () {

        const {getFieldDecorator} = this.props.form

        // name
        const fullLegalName_fn = getFieldDecorator('fullLegalName', {
            initialValue: this.props.user.profile.firstName + ' ' + this.props.user.profile.lastName,
            rules: [
                {required: true, message: 'Please input an name'},
                {min: 6, message: 'name too short'}
            ]
        })
        const fullLegalName_el = (
            <Input size="large"/>
        )

        // name
        const location_fn = getFieldDecorator('location', {
            rules: [
                {required: true, message: 'Please input a location'},
                {min: 6, message: 'location too short, please include location and country'}
            ]
        })
        const location_el = (
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
                        <a>
                            {this.state.attachment_type === 'application/pdf' ?
                                <Icon type="file-pdf"/> :
                                <Icon type="file"/>
                            } &nbsp;
                            {this.state.attachment_filename}
                        </a>
                    ) : (
                        <Button loading={this.state.attachment_loading}>
                            <Icon type="upload" /> Click to upload
                        </Button>
                    )
                }
            </Upload>
        );

        return {
            fullLegalName: fullLegalName_fn(fullLegalName_el),
            location: location_fn(location_el),

            attachment: attachment_fn(attachment_el)
        }
    }

    async componentDidMount() {



    }

    ord_render () {
        const {getFieldDecorator} = this.props.form
        const p = this.getInputProps()

        const formItemLayout = {
            labelCol: {
                xs: {span: 24},
                sm: {span: 6},
            },
            wrapperCol: {
                xs: {span: 24},
                sm: {span: 12},
            },
        }

        const formItemNoLabelLayout = {
            wrapperCol: {
                xs: {span: 24},
                sm: {offset: 6, span: 12},
            },
        }


        // const existingTask = this.props.existingTask

        // TODO: terms of service checkbox\

        // TODO: react-motion animate slide left

        // TODO: description CKE Editor

        return <div className="c_anniversaryAppFormContainer">

            <Form onSubmit={this.handleSubmit.bind(this)}>
                <div>
                    <Row>
                        <Col xs={{span: 24}} md={{offset: 6, span: 12}}>className="left-align">

                            <h5>Dear Community Member,</h5>

                            <p>
                                Elastos is celebrating its One Year Anniversary this August and we would love it if you
                                could make a short 5-15 second video of yourself stating your name and wishing Elastos a
                                happy anniversary and stating where you're from.
                            </p>

                            <h4>Example Script</h4>

                            <p>
                                <i>Hi my name is [Name] and I'd like to wish Elastos a great 1 year anniversary from [City, Country]</i>
                            </p>

                            <Divider></Divider>

                            <p>
                                Submissions will be put together into an Elastos highlight reel to be shown at the Anniversary event.
                            </p>


                            <h5>Notes:</h5>

                            <p>
                                Submissions in any language are welcome, if your submission is not in English please
                                also include a text translation in English so we can add the appropriate subtitles.
                            </p>

                        </Col>
                    </Row>

                    {!this.props.is_login ?
                    <div className="center">
                        <br/>
                        You must be logged in to apply to come to the 2018 Anniversary Event<br/>
                        <br/>
                        <Button onClick={() => {
                            sessionStorage.setItem('loginRedirect', '/form/anniversaryVideo2018')
                            this.props.history.push('/login')
                        }}>Login</Button>
                        <Button onClick={() => this.props.history.push('/register')}>Register</Button>
                    </div> : <div>

                        <Divider>Short Info</Divider>

                        <Row>
                            <Col span={6} className="right-align static-field">
                                Email:
                            </Col>
                            <Col span={12} className="static-field content">
                                {this.props.user.email}
                            </Col>
                        </Row>
                        <FormItem label="Full Name" {...formItemLayout}>
                            {p.fullLegalName}
                        </FormItem>
                        <FormItem label="Location" {...formItemLayout}>
                            {p.location}
                        </FormItem>

                        <Divider></Divider>

                        <Divider>
                            Upload Your Video
                        </Divider>

                        <FormItem {...formItemNoLabelLayout}>
                            {p.attachment} <span style={{color: 'red'}}>*</span> required
                        </FormItem>

                        <FormItem wrapperCol={{xs: {span: 24, offset: 0}, sm: {span: 12, offset: 6}}}>
                            <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn">
                                Submit
                            </Button>
                        </FormItem>

                        <Row>
                            <Col offset={6} className="static-field content">
                                Please contact <a href="mailto:cyberrepublic@elastos.org">cyberrepublic@elastos.org</a> if you have any issues.
                            </Col>
                        </Row>
                    </div>}
                </div>
            </Form>
        </div>

    }

}
export default Form.create()(C)
