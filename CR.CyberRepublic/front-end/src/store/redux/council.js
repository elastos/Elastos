import BaseRedux from '@/model/BaseRedux'

class CouncilRedux extends BaseRedux {

    defineTypes () {
        return ['council']
    }

    defineDefaultState() {

        return {
            loading: false,
            tab: null,
            filter: {}
        };
    }
}

export default new CouncilRedux()
