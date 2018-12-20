import React from 'react';
import StandardPage from '../../StandardPage';
import CVoteForm from '@/module/form/CVoteForm/Container';
import {Breadcrumb, Icon} from 'antd';
import './style.scss'

export default class extends StandardPage {
    constructor(p){
        super(p);
        this.state.data = null;
    }
    ord_renderContent(){
        return (
            <div className="p-cvote">
                <div className="d_box">
                    <CVoteForm edit={this.props.match.params.id} data={this.state.data} />
                </div>
            </div>
        );
    }

    ord_checkLogin(isLogin){
        if(!isLogin){
            this.props.history.replace('/login');
        }
    }
    async componentDidMount(){

        const data = await this.props.getData(this.props.match.params.id);
        this.setState({data});
    }

    goList(){
        this.props.history.push('/council');
    }
}
