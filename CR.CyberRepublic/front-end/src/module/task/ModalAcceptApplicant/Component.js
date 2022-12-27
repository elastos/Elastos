import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Modal, Select, Button, Input } from 'antd'
import config from '@/config'
import _ from 'lodash'

const TextArea = Input.TextArea

const FormItem = Form.Item

export default Form.create()(
  class C extends BaseComponent {

    ord_render () {
      const {onCancel, onCreate, visible, taskCandidate, acceptedCnt, acceptedMax} = this.props
      if (!taskCandidate) {
        return <div/>
      }

      // check if acceptedCnt = acceptedMax
      const allowAccept = acceptedCnt < acceptedMax

      const footerModal = (
        <div>
          {allowAccept && <Button onClick={onCreate} type="primary" className="ant-btn-ebp">Accept Applicant</Button>}
          <Button onClick={onCancel}>{allowAccept ? 'Cancel' : 'Close'}</Button>
        </div>
      )

      return (
        <Modal
          visible={visible}
          title={allowAccept ? 'Accept Applicant' : 'Max Applicants Accepted'}
          footer={footerModal}
          okText="Create"
          onCancel={onCancel}
          onOk={onCreate}
        >
          {allowAccept ? (
            <div>
              <p>
                            Are you sure you want to accept this applicant?
                <br/>
                <br/>
                {_.capitalize(taskCandidate.type)}
: &nbsp;
                <span className="strong-text">
                  {taskCandidate.type === 'USER' ? taskCandidate.user.username : taskCandidate.team.name}
                </span>
              </p>

              <h5>Application Message</h5>

              <p>
                {taskCandidate.applyMsg ? taskCandidate.applyMsg : 'no message given'}
              </p>
            </div>
          ) : (
            <span className="strong-text">
                        You cannot accept any more candidates for this task
            </span>
          )}
        </Modal>
      )
    }
  },
)
