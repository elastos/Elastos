import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import CVoteForm from '@/module/form/CVoteForm/Container'
import I18N from '@/I18N'

import { Container } from './style'

export default class extends BaseComponent {
  ord_render() {
    const { onCreate, onCancel } = this.props
    return (
      <Container>
        <CVoteForm header={I18N.get('from.CVoteForm.button.add')} onCreate={onCreate} onCancel={onCancel} />
      </Container>
    );
  }
}
