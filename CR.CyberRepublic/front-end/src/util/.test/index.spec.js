import {createContainer, constant} from "../index";

test('[method createContainer]', ()=>{
    const component = {a: 1};
    expect(createContainer(component, ()=>{}).a).toBe(1);
});

test('[method constant]', ()=>{
    const rs = constant('a', ['b']);

    expect(rs.b).toBe('a/b');
});