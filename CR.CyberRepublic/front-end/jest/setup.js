jest.mock('react-redux', ()=>{
    return {
        connect(){
            return (component)=>{
                return component;
            };
        }
    };
});

jest.mock('react-router', ()=>{
    return {
        withRouter(component){
            return component;
        }
    };
});