import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import I18N from '@/I18N'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import DeleteSvgIcon from '@/module/common/DeleteSvgIcon'
import EditSvgIcon from '@/module/common/EditSvgIcon'

class TeamInfoList extends BaseComponent {
  handleDelete = index => {
    this.props.onDelete(index)
  }

  handleEdit = index => {
    this.props.onEdit(index)
  }

  ord_render() {
    const { list, editable } = this.props
    const visible = editable === false ? editable : true
    return (
      <StyledTable>
        <StyledHead>
          <StyledRow>
            <th style={{ width: '15%' }}>
              {I18N.get('suggestion.plan.teamMember')}
            </th>
            <th style={{ width: '22%' }}>{I18N.get('suggestion.plan.role')}</th>
            <th>{I18N.get('suggestion.plan.responsibility')}</th>
            <th>{I18N.get('suggestion.plan.moreInfo')}</th>
            {visible && (
              <th style={{ width: 110 }}>
                {I18N.get('suggestion.plan.action')}
              </th>
            )}
          </StyledRow>
        </StyledHead>
        <tbody>
          {list &&
            list.map((item, index) => item && (
              <StyledRow key={index}>
                <td>{item.member}</td>
                <td>{item.role}</td>
                <td>
                  <MarkdownPreview
                    content={item.responsibility}
                    style={{ p: { margin: '1em 0' } }}
                  />
                </td>
                <td>
                  <MarkdownPreview
                    content={item.info}
                    style={{ p: { margin: '1em 0' } }}
                  />
                </td>
                {visible && (
                  <td>
                    <EditSvgIcon
                      type="edit"
                      onClick={this.handleEdit.bind(this, index)}
                      style={{ marginRight: 22, cursor: 'pointer' }}
                    />
                    <DeleteSvgIcon
                      type="delete"
                      onClick={this.handleDelete.bind(this, index)}
                      style={{ cursor: 'pointer' }}
                    />
                  </td>
                )}
              </StyledRow>
            ))}
        </tbody>
      </StyledTable>
    )
  }
}

TeamInfoList.propTypes = {
  onDelete: PropTypes.func,
  onEdit: PropTypes.func,
  list: PropTypes.array,
  editable: PropTypes.bool
}

export default TeamInfoList

const StyledTable = styled.table`
  margin-top: 16px;
  margin-bottom: 48px;
  width: 100%;
  font-size: 13px;
  table-layout: fixed;
`
const StyledHead = styled.thead`
  > tr {
    background: #0f2631;
  }
  th {
    line-height: 18px;
    padding: 16px;
    color: #fff;
  }
`
const StyledRow = styled.tr`
  width: 100%;
  background: #f2f6fb;
  > td {
    line-height: 18px;
    padding: 8px 16px;
    color: #000;
    overflow-wrap: break-word;
    > button {
      margin: 0 4px;
    }
  }
`
