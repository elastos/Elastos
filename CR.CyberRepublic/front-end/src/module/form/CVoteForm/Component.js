import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {Form, Icon, Input, Button, Checkbox, Select, Row, Col, message, Steps, Modal, Switch, Tabs } from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { LANGUAGES } from '@/config/constant'

import './style.scss'

const FormItem = Form.Item
const TextArea = Input.TextArea
const TabPane = Tabs.TabPane
const Step = Steps.Step

class C extends BaseComponent {

    constructor(props) {
        super(props)

        this.state = {
            persist: true,
            loading: false,
            language: LANGUAGES.english// language for this specifc form only
        }

        this.isLogin = this.props.isLogin
        this.user = this.props.user
        this.onChangeLang = this.onChangeLang.bind(this)
    }

    ord_loading(f=false){
        this.setState({loading : f})
    }

    async handleSubmit(e) {
        e.preventDefault()

        const s = this.props.static
        this.props.form.validateFields(async (err, values) => {
            if (!err) {
                // console.log(' ===> ', values)

                const param = {
                    title: values.title,
                    title_zh: values.title_zh,
                    type: values.type,
                    notes: values.notes,
                    notes_zh: values.notes_zh,
                    motionId: values.motionId,
                    isConflict: values.isConflict,
                    proposedBy: values.proposedBy,
                    content: values.content,
                    content_zh: values.content_zh,
                    published: values.published === 'YES'
                }
                var x1 = []
                var x2 = []
                var x3 = []
                _.each(s.voter, (n)=>{
                    const name = n.value
                    x1.push(name+'|'+values['vote_'+name])
                    x2.push(name+'|'+values['reason_'+name])
                    x3.push(name + '|' + values['reason_zh_' + name])
                })
                param.vote_map = x1.join(',')
                param.reason_map = x2.join(',')
                param.reason_zh_map = x3.join(',')

                // console.log(param)
                this.ord_loading(true)
                if(this.props.edit){
                    try{
                        param._id = this.props.edit
                        await this.props.updateCVote(param)
                        message.success(I18N.get('from.CVoteForm.message.updated.success'))
                        this.ord_loading(false)
                        this.props.history.push('/council')
                    }catch(e){
                        message.error(e.message)
                        this.ord_loading(false)
                    }

                }
                else{
                    try{
                        await this.props.createCVote(param)
                        message.success(I18N.get('from.CVoteForm.message.create.success'))
                        this.ord_loading(false)
                        this.props.history.push('/council')
                    }catch(e){
                        message.error(e.message)
                        this.ord_loading(false)
                    }

                }

            }
        })
    }

    getInputProps(data) {

        const edit = this.props.edit
        const role = this.props.user.role
        const isCouncil = this.props.isCouncil

        const fullName = this.user.profile.firstName + ' ' + this.user.profile.lastName

        const publicReadonly = {}
        const publicDisabled = {}
        const councilNotOwnerReadOnly = {}
        const councilNotOwnerDisabled = {}

        if(!isCouncil){
            publicReadonly.readOnly = true
            publicDisabled.disabled = true
        }
        else{
            if(edit && (data.createdBy !== this.user.current_user_id || _.includes(['FINAL', 'DEFERRED'], data.status))){
                councilNotOwnerReadOnly.readOnly = true
                councilNotOwnerDisabled.disabled = true
            }
        }

        const secretaryDisabled = {readOnly: true}
        if (this.props.user.current_user_id === '5b4c3ba6450ff10035954c80') {
            delete secretaryDisabled.readOnly
        }

        // TODO: must delete
        // for test only
        // publicReadonly.readOnly=false
        // publicDisabled.disabled= false
        // councilNotOwnerReadOnly.readOnly= false
        // councilNotOwnerDisabled.disabled=false
        // secretaryDisabled.readOnly = false

        const s = this.props.static
        const {getFieldDecorator} = this.props.form

        const published_fn = getFieldDecorator('published', {
            initialValue : edit ? (data.published ? 'YES' : 'NO') : 'NO'
        })
        const published_el = (
            <Select {...publicDisabled} size="large">
                <Select.Option value={'NO'}>{I18N.get('from.CVoteForm.no')}</Select.Option>
                <Select.Option value={'YES'}>{I18N.get('from.CVoteForm.yes')}</Select.Option>
            </Select>
        )

        const title_fn = getFieldDecorator('title', {
            rules : [{required : true}],
            initialValue : edit ? data.title : ''
        })
        const title_el = (
            <Input {...publicReadonly} {...councilNotOwnerReadOnly} size="large" type="text" />
        )

        const title_zh_fn = getFieldDecorator('title_zh', {
            initialValue: edit ? data.title_zh : ''
        })
        const title_zh_el = (
            <Input {...publicReadonly} {...councilNotOwnerReadOnly} size="large" type="text" />
        )

        const type_fn = getFieldDecorator('type', {
            rules: [{required: true}],
            readOnly: true,
            initialValue: edit ? parseInt(data.type, 10) : ''
        })
        const type_el = (
            <Select size="large" {...publicDisabled} {...councilNotOwnerDisabled}>
                {/* <Select.Option key={-1} value={-1}>please select type</Select.Option> */}
                {
                    _.map(s.select_type, (item, i)=>{
                        return (
                            <Select.Option key={i} value={item.code}>{item.name}</Select.Option>
                        )
                    })
                }
            </Select>
        )

        const content_fn = getFieldDecorator('content', {
            rules : [{required : true}],
            initialValue : edit ? data.content : ''
        })
        const content_el = (
            <TextArea {...publicReadonly} {...councilNotOwnerReadOnly} rows={6}></TextArea>
        )
        const content_zh_fn = getFieldDecorator('content_zh', {
            initialValue: edit ? data.content_zh : ''
        })
        const content_zh_el = (
            <TextArea {...publicReadonly} {...councilNotOwnerReadOnly} rows={6}></TextArea>
        )

        const proposedBy_fn = getFieldDecorator('proposedBy', {
            rules : [{required : true}],
            initialValue : edit ? data.proposedBy : fullName
        })
        const proposedBy_el = (
            <Select {...publicDisabled} {...councilNotOwnerDisabled} size="large">
                {/* <Select.Option key={-1} value={-1}>please select</Select.Option> */}
                {
                    _.map(s.voter, (item, i)=>{
                        return (
                            <Select.Option key={i} value={item.value}>{item.value}</Select.Option>
                        )
                    })
                }
            </Select>
        )

        const motionId_fn = getFieldDecorator('motionId', {
            initialValue : edit ? data.motionId : ''
        })
        const motionId_el = (
            <Input {...publicReadonly} {...councilNotOwnerReadOnly} size="large" type="text" />
        )

        const vtt = {}
        debugger
        _.each(s.voter, (item)=>{
            const name = item.value

            let tmp = {}
            // if(edit && fullName !== name && data.createdBy !== this.user.current_user_id){
            if(fullName !== name){
                tmp.disabled = true
            }

            const fn = getFieldDecorator('vote_'+name, {
                initialValue : edit ? data.vote_map[name] : (fullName !== name ? '-1' : 'support')
            })
            const el = (
                <Select {...publicDisabled} {...tmp} size="large">
                    <Select.Option key={'-1'} value={'-1'}>please select</Select.Option>
                    {
                        _.map(s.select_vote, (item, i)=>{
                            return (
                                <Select.Option key={i} value={item.value}>{item.name}</Select.Option>
                            )
                        })
                    }
                </Select>
            )
            vtt['vote_'+name] = fn(el)
        })

        const vts = {}
        _.each(s.voter, (item)=>{
            const name = item.value

            let tmp = {}
            // if(edit && fullName !== name && data.createdBy !== this.user.current_user_id){
            if(fullName !== name){
                tmp.disabled = true
            }

            const fn = getFieldDecorator('reason_'+name, {
                initialValue : edit ? data.reason_map[name] : '',
                rules : [
                    {},
                    {
                        validator : (rule, value, callback)=>{
                            const form = this.props.form
                            const tmp = form.getFieldValue('vote_'+name)
                            if(tmp === 'reject' && !value){
                                callback('please input your reject reason')
                            }
                            else{
                                callback()
                            }

                        }
                    }
                ]

            })
            const el = (
                <TextArea {...publicReadonly} {...tmp} rows={4}></TextArea>
            )
            vts['reason_'+name] = fn(el)
        })

        const vts_zh = {}
        _.each(s.voter, (item)=>{
            const name = item.value

            let tmp = {}
            // if(edit && fullName !== name && data.createdBy !== this.user.current_user_id){
            if (fullName !== name) {
                tmp.disabled = true
            }

            const fn = getFieldDecorator('reason_zh_' + name, {
                initialValue: edit ? (data.reason_zh_map ? data.reason_zh_map[name] : '') : '',
                rules: [
                    {},
                    {
                        validator: (rule, value, cb) => {
                            const form = this.props.form
                            const tmp = form.getFieldValue('vote_' + name)
                            if (tmp === 'reject' && !value) {
                                cb('please input your reject reason')
                            }
                            else {
                                cb()
                            }

                        }
                    }
                ]

            })
            const el = (
                <TextArea {...publicReadonly} {...tmp} rows={4}></TextArea>
            )
            vts_zh['reason_zh_' + name] = fn(el)
        })
        const isConflict_fn = getFieldDecorator('isConflict', {
            initialValue : edit ? data.isConflict : 'NO'
        })
        const isConflict_el = (
            <Select {...publicDisabled} {...councilNotOwnerDisabled} size="large">
                <Select.Option value={'NO'}>{I18N.get('from.CVoteForm.yes')}</Select.Option>
                <Select.Option value={'YES'}>{I18N.get('from.CVoteForm.no')}</Select.Option>
            </Select>
        )

        const notes_fn = getFieldDecorator('notes', {
            initialValue : edit ? data.notes : ''
        })
        const notes_el = (
            <TextArea {...secretaryDisabled} rows={4}></TextArea>
        )
        const notes_zh_fn = getFieldDecorator('notes_zh', {
            initialValue: edit ? data.notes_zh : ''
        })
        const notes_zh_el = (
            <TextArea {...secretaryDisabled} rows={4}></TextArea>
        )

        return {
            published: published_fn(published_el),
            title : title_fn(title_el),
            title_zh: title_zh_fn(title_zh_el),
            type : type_fn(type_el),
            content : content_fn(content_el),
            content_zh: content_zh_fn(content_zh_el),
            proposedBy : proposedBy_fn(proposedBy_el),
            motionId : motionId_fn(motionId_el),
            ...vtt,
            ...vts,
            ...vts_zh,
            isConflict : isConflict_fn(isConflict_el),
            notes: notes_fn(notes_el),
            notes_zh: notes_zh_fn(notes_zh_el)
        }
    }

    togglePersist() {
        this.setState({persist: !this.state.persist})
    }

    onChangeLang(val) {
        return this.setState({language: this.state.language === 'en' ? 'zh' : 'en'})
    }
    ord_render() {
        const { language } = this.state
        let p = null
        if(this.props.edit && !this.props.data){
            return null
        }
        if(this.props.edit && this.props.data){
            p = this.getInputProps(this.props.data)
        }
        else{
            p = this.getInputProps()
        }
        const s = this.props.static
        const formItemLayout = {
            labelCol: {
                xs: {span: 24},
                sm: {span: 6}
            },
            wrapperCol: {
                xs: {span: 24},
                sm: {span: 12}
            },
        }

        return (
            <Form onSubmit={this.handleSubmit.bind(this)} className="c_CVoteForm">

                <h2>
                    {I18N.get('from.CVoteForm.proposal.title')}
                </h2>

                <h5>
                    {I18N.get('from.CVoteForm.proposal.content')}
                </h5>

                <Row>
                    <Col offset={6} span={12}>
                        {this.renderVoteStep(this.props.data)}
                    </Col>
                </Row>
                <br />
                <Tabs defaultActiveKey={LANGUAGES.english} onChange={(k) => console.log('changing tab: ' + k)}>
                    <TabPane tab={I18N.get('0301')} key={LANGUAGES.english}>
                        <FormItem style={{marginBottom:'12px'}} label={I18N.get('from.CVoteForm.label.publish')} {...formItemLayout}>
                            {p.published}
                        </FormItem>
                        <FormItem label={I18N.get('from.CVoteForm.label.title')} {...formItemLayout} style={{marginTop: '24px'}}>{ p.title }</FormItem>

                        <FormItem label={I18N.get('from.CVoteForm.label.type')} {...formItemLayout}>{p.type}</FormItem>

                        <FormItem label={I18N.get('from.CVoteForm.label.content')} {...formItemLayout}>{p.content}</FormItem>
                        <FormItem label={I18N.get('from.CVoteForm.label.proposedby')} {...formItemLayout}>{p.proposedBy}</FormItem>

                        <FormItem style={{'marginBottom':'30px'}} label={I18N.get('from.CVoteForm.label.motion')} help={I18N.get('from.CVoteForm.label.motion.help')} {...formItemLayout}>{p.motionId}</FormItem>

                        {_.map(s.voter, (item, i)=>{
                                const name = item.value
                                return (
                                    <FormItem key={i} label={`Online Voting by ${name}`} {...formItemLayout}>{p['vote_'+name]}</FormItem>
                                )
                        })}

                        {_.map(s.voter, (item, i)=>{
                                const name = item.value
                                return (
                                    <FormItem key={i} label={`Reasons from ${name} if against`} {...formItemLayout}>{p['reason_' + name]}</FormItem>
                                )
                        })}

                        <FormItem style={{'marginBottom':'12px'}} label={I18N.get('from.CVoteForm.label.conflict')} help={I18N.get('from.CVoteForm.label.conflict.help')} {...formItemLayout}>{p.isConflict}</FormItem>
                        <FormItem label={I18N.get('from.CVoteForm.label.note')} {...formItemLayout}>{p.notes}</FormItem>
                        <Row>
                            <Col offset={6} span={12}>
                                {this.props.isCouncil && this.renderSubmitButton()}
                                {this.props.isCouncil && this.renderFinishButton()}
                                {this.props.isCouncil && this.renderUpdateNoteButton()}
                            </Col>
                        </Row>
                    </TabPane>
                    <TabPane tab={I18N.get('0302')} key={LANGUAGES.chinese}>
                        <FormItem label={I18N.get('from.CVoteForm.label.title')} {...formItemLayout}>{ p.title_zh }</FormItem>
                        <FormItem label={I18N.get('from.CVoteForm.label.content')} {...formItemLayout}>{p.content_zh}</FormItem>
                            { _.map(s.voter, (item, i) => {
                                const name = item.value
                                return (
                                    <FormItem key={i} label={`Reasons from ${name} if against`} {...formItemLayout} >{p['reason_zh_' + name]}</FormItem>
                                )
                            })}
                        <FormItem label={I18N.get('from.CVoteForm.label.note')} {...formItemLayout} >{p.notes_zh}</FormItem>
                    </TabPane>
                </Tabs>
            </Form>
        )
    }

    renderUpdateNoteButton(){
        const edit = this.props.edit
        const role = this.props.user.role
        const data = this.props.data
        if(edit && this.isLogin && role === 'SECRETARY' && _.includes(['FINAL', 'DEFERRED'], data.status)){
            return (
                <FormItem style={{marginTop:40}}>
                    <Button loading={this.state.loading} onClick={this.updateNote.bind(this, data._id)} size="large" type="ebp" className="d_btn">
                        Update Notes
                    </Button>
                </FormItem>
            )
        }
        return null
    }

    updateNote(id){
        const notes = this.props.form.getFieldValue('notes')
        this.ord_loading(true)
        this.props.updateNotes({
            _id : id,
            notes
        }).then(()=>{
            message.success(I18N.get('from.CVoteForm.message.note.update.success'))
            this.ord_loading(false)
        }).catch((e)=>{
            message.error(e.message)
            this.ord_loading(false)
        })
    }

    renderSubmitButton(){
        const edit = this.props.edit
        const role = this.props.user.role
        const data = this.props.data
        if (!this.isLogin || !_.includes(['ADMIN', 'SECRETARY'], role)){
            return (
                <h4 style={{color:'#f00'}}>{I18N.get('from.CVoteForm.text.onlycouncil')}</h4>
            )
        }
        else{
            return (
                <FormItem>
                    <Button loading={this.state.loading} size="large" type="ebp" htmlType="submit" className="d_btn">
                        {edit ? I18N.get('from.CVoteForm.button.save') : I18N.get('from.CVoteForm.button.submit')}
                    </Button>
                </FormItem>
            )
        }
    }
    renderFinishButton(){
        const edit = this.props.edit
        const role = this.props.user.role
        const data = this.props.data
        if(edit && this.isLogin && role === 'SECRETARY' && data.status !== 'FINAL'){
            return (
                <FormItem style={{marginTop:40}}>
                    <Button loading={this.state.loading} onClick={this.finishClick.bind(this, data._id)} size="large" type="ebp" className="d_btn">
                        {I18N.get('from.CVoteForm.button.complete.proposal')}
                    </Button>
                </FormItem>
            )
        }
        return null
    }
    finishClick(id){
        Modal.confirm({
            title: I18N.get('from.CVoteForm.modal.title'),
            content: '',
            okText: I18N.get('from.CVoteForm.modal.confirm'),
            okType: 'danger',
            cancelText: I18N.get('from.CVoteForm.modal.cancel'),
            onOk: ()=>{
                this.ord_loading(true)
                this.props.finishCVote({
                    id : id
                }).then(()=>{
                    message.success(I18N.get('from.CVoteForm.message.proposal.update.success'))
                    this.ord_loading(false)
                    this.props.history.push('/council')
                }).catch((e)=>{
                    message.error(e.message)
                    this.ord_loading(false)
                })
            },
            onCancel(){
            }
        })
    }

    renderVoteStep(data){
        if(!this.props.edit){
            return null
        }

        const s = this.props.static
        let n = 0
        let en = 0
        let an = 0
        let status = 'process'
        let ss = data.status || 'processing...'
        _.each(s.voter, (item)=>{
            const name = item.value
            if (data.vote_map[name] === 'support'){
                n++
            }
            else if (data.vote_map[name] === 'reject'){
                en++
            }
            else {
                an++
            }
        })
        if(an > 0){

        }
        else if(en > 1){
            status = 'error'
            // ss = 'not pass'
        }

        if(n > 1){
            status = 'finish'
            // ss = 'pass'
        }

        const sy = {
            a : {
                width : '100%',
                border : '1px solid #cdd',
                height : 32,
                flex : 1,
                display : 'flex',
                background : '#eee'
            },
            b : {
                flex : 1,
                display : 'flex',
                borderRight : '1px solid #ccc'
            }
        }
        const fn = (step)=>{
            const xx = step - n
            if(n>=step){
                return {
                    background : '#009999'
                }
            }
            else if(en >= xx){
                return {
                    background : '#ff4d4f'
                }
            }
        }
        sy.a1 = _.extend(fn(1), sy.b)
        sy.a2 = _.extend(fn(2), sy.b)
        sy.a3 = _.extend(fn(3), sy.b)
        return (
            <div>
                <h4 style={{paddingBottom:'5px'}}>
                    {I18N.get('from.CVoteForm.label.voteStatus')} : <span className="cvoteStatus">{I18N.get(`cvoteStatus.${ss}`)}</span>
                </h4>
                <div style={sy.a}>
                    <div style={sy.a1}></div>
                    <div style={sy.a2}></div>
                    <div style={sy.a3}></div>
                </div>
                {/* <Steps current={status==='error'?en-1 : n-1} status={status}>
                    <Step title="" />
                    <Step title="" />
                    <Step title="" />
                </Steps> */}
            </div>

        )
    }
}

export default Form.create()(C)
