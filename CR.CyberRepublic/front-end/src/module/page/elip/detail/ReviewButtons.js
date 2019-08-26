import React from 'react'
import { Button, Modal, Input } from 'antd'
import I18N from '@/I18N'
const { TextArea } = Input
import { ELIP_STATUS } from '@/constant'

class ReviewButtons extends React.Component {
  constructor(p) {
    super(p)
    this.state = {
      visibale: false,
      reason: ''
    }
  }

  showModal = () => {
    this.setState({ visible: true })
  }

  handleCancel = e => {
    this.setState({ visible: false })
  }

  handleOk = async () => {
    const { onSubmit } = this.props
    const { reason } = this.state
    await onSubmit({ reason, status: ELIP_STATUS.REJECTED })
    this.setState({ reason: '', visible: false })
  }

  handleChange = e => {
    this.setState({ reason: e.target.value })
  }

  render() {
    return (
      <div>
        <Button
          className="cr-btn cr-btn-danger"
          style={{ marginRight: 10 }}
          onClick={this.showModal}
        >
          {I18N.get('elip.button.reject')}
        </Button>
        <Button className="cr-btn cr-btn-primary">
          {I18N.get('elip.button.approve')}
        </Button>
        <Modal
          title={I18N.get('elip.modal.reason')}
          visible={this.state.visible}
          onOk={this.handleOk}
          onCancel={this.handleCancel}
          okText={I18N.get('elip.modal.confirm')}
          cancelText={I18N.get('elip.modal.cancel')}
        >
          <TextArea
            onChange={this.handleChange}
            value={this.state.reason}
            autosize={{ minRows: 6 }}
          />
        </Modal>
      </div>
    )
  }
}

export default ReviewButtons
