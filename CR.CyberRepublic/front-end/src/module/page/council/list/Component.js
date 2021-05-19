import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import './style.scss'

export default class extends BaseComponent {

  ord_render() {
    return (
      <div className="p_councilList">
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_content">
              {this.renderList()}
            </div>
          </div>
        </div>
      </div>
    )
  }

  renderList() {
    const listData = this.getListData()
    const p_list = {
      itemLayout: 'horizontal',
      // size : 'small',
      // pageSize : 10,
      dataSource: listData,
      header: (<h2 style={{padding: 0}}>{I18N.get('council.0002')}</h2>)
      // renderItem : ()=>{

      // }
    }

    return (
      <div className="d_list">

        <List {...p_list} renderItem={item => (
          <List.Item key={item.title} >
            <List.Item.Meta
              title={(
                <a href="#" onClick={this.toDetail.bind(this, `/council/detail/${item.id}`)} className="f_h4">
#
                  {item.id}
                  {' '}
-
                  {item.title}
                </a>
)}
              description={item.description}
            />
            <div style={{position: 'relative', top: 20}}>{item.date}</div>
          </List.Item>
        )}
        />

        <h4 style={{marginTop: 24}}>
          {I18N.get('council.list.proposals')}
          {' '}
          <a href="mailto:council@cyberrepublic.org">council@cyberrepublic.org</a>
        </h4>
      </div>
    )
  }

  getListData() {
    return [
      {
        id: '1',
        title: I18N.get('council.list.1'),
        description: I18N.get('council.desc.1'),
        date: '09/13/2018'
      },
      {
        id: '2',
        title: I18N.get('council.list.2'),
        description: I18N.get('council.desc.2'),
        date: '09/13/2018'
      },
      {
        id: '3',
        title: I18N.get('council.list.3'),
        description: I18N.get('council.desc.3'),
        date: '09/13/2018'
      },
      {
        id: '4',
        title: I18N.get('council.list.4'),
        description: I18N.get('council.desc.4'),
        date: '09/13/2018'
      },
      {
        id: '5',
        title: I18N.get('council.list.5'),
        description: I18N.get('council.desc.5'),
        date: '09/13/2018'
      }
    ]
  }

  toDetail(path) {
    this.props.history.push(path)
  }
}
