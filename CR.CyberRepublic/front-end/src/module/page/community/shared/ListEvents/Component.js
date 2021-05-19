import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Link } from 'react-router-dom'
import { Row, Col } from 'antd'
import I18N from '@/I18N'

export default class extends BaseComponent {
  ord_render () {
    const mockData = {
      name: 'Paris Meetup - 18 May 2018',
      description: 'The greatest scientists are artist too',
      thumbnail: 'http://18.218.149.20:3001/assets/images/task_thumbs/12.jpg',
      date: '2018.05.18',
      time: '24:00',
      hour: '12 hour',
      url: 'http://example.com'
    }

    const events = this.props.events || []
    return (
      <Row>
        {events.map((event, index) => {
          return (
            <Col span={8} key={index}>
              <div className="wrap-event">
                <div className="wrap-image">
                  <img src={event.thumbnail || mockData.thumbnail}/>
                </div>
                <div className="event-title">{event.name}</div>
                <div className="event-description">{event.description}</div>
                <div className="event-meta">
                  {mockData.date}
                  {' '}
/
                  {' '}
                  {mockData.time}
                  {' '}
/
                  {' '}
                  {mockData.hour}
                  {' '}
                  <Link to={`/task-detail/${event._id}`}>{I18N.get('community.link.toevent')}</Link>
                </div>
              </div>
            </Col>
          )
        })}
      </Row>
    )
  }
}
