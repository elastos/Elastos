import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Table, Button } from 'antd'

export default class extends BaseComponent {
  ord_render () {
    const columns = [{
      title: 'Short title',
      dataIndex: 'title',
      key: 'title',
    }, {
      title: 'Short description',
      dataIndex: 'description',
      key: 'description',
    }, {
      title: 'Location',
      dataIndex: 'location',
      key: 'location',
    }, {
      title: 'Action',
      key: 'action',
      render: (text, record) => (
        <Button>Accept</Button>
      ),
    }]

    const mockData = {
      id: 1,
      title: 'Build a website',
      description: 'Description of the project',
      location: 'Online',
    }

    const tasks = this.props.tasks || []
    // Mock data
    tasks.forEach((task) => {
      task.location = 'Location mock'
      task.title = 'Title mock'
    })

    return (
      <Table rowKey={record => record._id} pagination={false} columns={columns} dataSource={tasks} />
    )
  }
}
