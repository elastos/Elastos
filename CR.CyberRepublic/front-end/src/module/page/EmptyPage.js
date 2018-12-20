import React from 'react';
import BasePage from '@/model/BasePage';

export default class extends BasePage {
    ord_renderPage() {
        return (
            <div className="p_emptyPage">
                {this.ord_renderContent()}
            </div>
        );
    }

    ord_renderContent() {
        return null;
    }
}
