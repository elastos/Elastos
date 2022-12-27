
/**
 * win: Browser Window to inject traffic-light
 * options: 
 *   Object: style: traffic-light style //TODO
 *   Function: shouldAddTrafficLight (url) => trueOrFlase. determine inject traffic or not 
 *   Function: shouldAddDragBar (url) => trueOrFlase. determine inject drag bar or not 
 * return EventEmitter 
 *
 * Events: 
 *   'minimize'
 *   'maximize'
 *   'close'
 *   'fullscreen'
 **/
const {BrowserWindow, ipcMain} = require('electron')
const fs = require('fs')
const path = require('path')
const EventEmitter = require('events');

function cssJS() {
    return `
        var style = document.createElement('style');
        style.type = 'text/css';
        style.innerHTML = \`
            #drag-bar {
                -webkit-app-region:no-drag; position: fixed; 
                height: 24px;
                width: 58px;
                top: 14px;
                left: 18px;
                user-select: none;
                z-index: 1;
            }
            #traffic-light {
                display: flex;
                justify-content: space-between;
                width: 52px;
                position: fixed;
                z-index: 2;
                left: 20px;
                top: 20px;
            }
            [data-type=close] {
                background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAAAXNSR0IArs4c6QAAAh1JREFUSA21lsFOwkAQhme2YElBExE0Gj2qD+LB4AlPXoyv4AN49gF8BePFk5wkHnwQ8ajRKIqJQkOBdp1/dYlWkCB1Lttud7+Z3Z39p0y/2G2ptKwpKHOkyzJsTWtdxHBmrktT04orTG5luVq9Rf8g40Gd9c3NxYA6h8S0k3Fd7WUy2XQqRY6jzPAwjKjb65HfbrfaQcCk6dSlqYPixcV9nPfDwV1poxzp8DjnZd2ZnDfF/AGNT7TvWkf02vQ7Tb8VKHb2lqqXFfsN7TcHN1sb+yrSh8X8bDadTn0dN/K52+1RvfHSihQfrJxfHtkJfQeIXPb4ZKGQzzqOY7+P1YZhSA9PjZac0a5diXFg9pyDq/n83PS4kccjwEoeG89vrnbXcSZmg3Gg2PNJ4XAGBlgmSeSdkYqk21dL8wVv1IHGox32joO/e3zyiTPrCnkuqSi5/Xu2DIMN6gcLTLAVh3pb8twbNHCSPjDBVpKoq7hESZthClvh+tsbmqQTMMFObuOHRKcgXNCWpA1MsGUddA3hSto+mTWlHT4TVfSTdgClhZwr6LlIrhxIctsEFmQcbGWKheg5JDepVRiWMME2WYRiAT2HUE1qYIAFJljGgVE9KRbQc0juXw1zwUDhsdWtfw+g3ygW0PO/rARzMBcMWwsQaL/g2Kj/tWRaJ7GiTxCuIUXfRwaOVfStE7T93xZRRYjit98WuaC4Q6N+W94ByM4xdG0oqUYAAAAASUVORK5CYII=')
            }
            #traffic-light:hover [data-type=close] {
                background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAAAXNSR0IArs4c6QAAAvpJREFUSA29VkFPE1EQnrfbsrUtBYFCNFSIiXjxD0iMpiGxqSc8eTH+BQ9GL9UbxIQbifEHGC+e7EnSA1EucCCaeBQvAhWUCkKhdRe6fc638tbdpWtTE32HfbvvzXwzb+Z7MyvoD6Oczw9LsiZFU06y2JiUMg1xIUSFpxWpiaIgozg8N1fGeqshWi1WcrkzFh1Ok6BbMcOQ8VgsEY1ESNc1R9y2m3TUaFDdNGumZQmS9MKgrkK6VNoM4p0wsJGfmGxK+1kynjBSyXiXEL9Ag4rqW8omVQ/qhwf1mqUJ/c7Zufmi2sPsM7B+Y+Ku1pTT6b7TiWg04pVr+3501KDKzvdaUxOFzKv5WaXgGoDnHOPnQwN9CV3X1X5Hs23b9PXbTo1zdFudxDHgxFxYHwb7+rs79TzoAU6ytbO9b0jjInLiBBgJRcy94E8Wl+lR6TXtm1YQw/2umqYj83Rp2V0DBrAckvCqBiqCLUioK8UvG9V9ev9liwohRgBeKL1xZDaqB15VcrAYE9gaeM5UZG772fLg2jid60nRp929E0YU+CrvjfSm6P7Vyz4DwAImsPV750cfp7qTY+C5d8T4qOMjGXpb3qTVvSq9+7xJV0YzZDH/4bkCn7qepZ5TMa+q8y5IRM26GRPlXHZtaKA/E4m0Zs7uD5MecpjW2Mhobw97RW3BYaHRAKO21zVcf3VDHdOBRy97N5XLuuFq57lSByaw/YFXu4FZYzIzt91VPj5pnm93o8ULy4kKakvYCCY0LPFBfWACm89BH1G4Wo0gOBLqDVcYhYF1jLmiSV285KpYDxpoBQ62BHMSZgSVFuWcw2sUueRyQvxhmllYCmVL0MjMwqLPP2ChjANbc5oF13OUXK/UYDJBl4bSFMZzZQQykPUOB4sxgf1/ip1T9bhZoJ6j5P7tgC4w0HhUd3PvAeo3mgXqOUpupwM60AWG6gXA+H17jhH/actUXgeaPnHTj4c0/ToY2FHTV0Ywu78ttrzJZ73g+23hC4o71O635ScefdGiSCyMOgAAAABJRU5ErkJggg==')
            }
            [data-type=min] {
                background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAAAXNSR0IArs4c6QAAAhxJREFUSA21ls9S01AUh++5SQtJBlmJoh11YWF8m7pyxgFfwQfomgfwFYRxxpV9GwdwoU4VhRUySaBNcrxfSjpQoBUaz6J/Jsn3u/fcc34nYqZEf+d5K1PbEaMdEV3TwtzndrHmSFX21EjPl6LX2vjSvwkj1134uv1i1RbDLWPlVRD4GoVe1GiI8bzR7XmuZpipieM8TtNMTKEfC9voPtv8fDDJuyLw/cNaRzN9f2/JX1hebjbdaqeG25U5Ph4M/pxkZ+LLmyev93oXH7gk8G27/dbxtlZWFqPmwgzyRYr7PTgrzOHhaez0uk83999Vl8cCrFxy3Vl9FERVKqqb/vWb1B38TGP1ZKPaSSlAzkWz3YcPFpduu/JJcXby6/fpiYq/zpmUeeBAyfm8cMRgwCqLxP0XSrFQ2W09DsNZBzq52pv+c/D9H0liRdctdR6EPrVdW8CCCduJFC+jwAtro5+DXO+EsK1Ta9NEdUfDF7LStrT/Xcty2qJgwq4x8xNy50khRUc0SN2RO6+CTYr2h8P6BTBDHNeq2E9xmid17wCnxc4tfp4mGQdSW8DCxmHbclg4P8dy61IoWY4Ju6wihgV+jlHNGzBgwYRVCuB6DAv8fJ6K4lkYsKrpNu4D/Nutv4uf32UnPMOzMKpZwA6ueMR/HZkoEpeGvnNFzPDaoZ/kCRV4q6E/khh9jl9bnCtiXHgLV+hQGpQemvXa8hdr6zhi7opTzQAAAABJRU5ErkJggg==')
            }
            #traffic-light:hover [data-type=min] {
                background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAAAXNSR0IArs4c6QAAAkpJREFUSA21ls1S1EAQx6cnswtJCjiJoitglQvlk6jlZT1ZZYGv4APsmQfwFYSyypN7s3wSC/AA1CJfJ6CSwG4ybf8DSYXwsbAb+5Kv6V/P9Ez/O6TusO7ay0bMukWKW0S8wFY9wnDS6oiZNlhRx5DtNJb+dG/D0E0ftlZfzWjbX1GaPriuYd9z/FqNlONcDE8SVv2YVRAkQRTFpCx/t7rWnl/+vVfmXQuw822hxTF/nZwwY1NT9brM9k6TVanj417v5DQ+J0OfZj9udIoOVwJsrzY/C29lenrcr48NIBcpct87t+rw8CyQeO255c0v2ec8AGZOCa/NPHX9LBXZoPtekbq9v1HADi1lK0kDIOfE8fqTx+MTD515OThWsn9wdspkFrEnaR6wocj5qHAEAwOs9JDIM+EoWqb1xjPPG7Sh5dne9oyN7+6GoSZe1DjnrmdwtiszsMAEW4LY977reJXRL0FSOx7YWqI1UURVW80QstI0KP/isfz1c0vtbJ8MFW92blK9fjuf+oIJtimT2LLi8st7PsM3t8ukGFnGkRTIc6Mv3rx59yIfM8pNIloFtpZlbPb7hcijUAu+EEMormbSP4IoCQvfKrmF0kLONfQ8CmNsSGUGFmQcbJ02C9FzSG5VEVKWMMFO6xfNAnoOoRrVwAALTLDSAFA9NAvoOSR3WIMvGGBl3S1XIOi3zL8NPR9mJfCBLxhZL8BEr2nEf22ZWWquNH1RRYjhjU0/TEKcwAc1/SwIrvlvi6gihAvagveoUBQoamjQb8s/UsFGN/X8miYAAAAASUVORK5CYII=')
            }
            [data-type=fullscreen] {
                background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAAAXNSR0IArs4c6QAAAjRJREFUSA21ls9u00AQxmfWdhxs/kQgVFVQuFB6ohfOvEOjHnpBvAIP0HMfgEcAcemhSl6ES7lQygUKigABUYudurZ32G+rjZK0IaRdVrKcdXZ/39i7880y/aU92Fm9WypZE1Lmkoek5bYdrvg7E79n0t1Ic/fD+u7naRg+74/7O48XdVBtkchGmMQSpXHKUUAqVHa4rjTpk5qqvMjMZbR4W9Xh5sf1N71J3hmBpc6jNRZ+FV1vxlErbbA6M2SMIVqo7Gcn5eFxISzPDtpvu6MDxmbf66w+J6at5kIrDeJwdNzM33VR0fHXfkZCm5/auy/chKGAjZz5dXLnZsrB6adwg/71LrWm/MvPTESeujexAvjmwuVec7F1bd7IJ8Xtm/T6RyzRCtbEhooFxTe/LBxiYIBlN4npM7ZioWgvXbqVzFrQyWin9bHw2cGPPNa0orDPwysx+YJDFCwwwVYmidrR1TiZFs1Fn4dpnIBtBGQZSeS7qUZgdqwsK6S/y1CfIpYJtk/oKEvktKfIGBe8xXdD0oFt1pv3pax9860ZwnGNgO6Uv4vctwKcFnau4OfVoCAkh68GFmwcbGWLhfFzWK4vAcsyTLDtLkKxgJ/DqC7bwAALTLCsAFwPxQJ+blf/giqYaxmG5arbMIUPt7+9u7GxkJVHgydBs9GYN/kQ+aD3yxYcUwteuhiHBcc9+K8l04mMFX3jijDDc4t+VuTYgXMVfSeC+8ixpQ3jmji27COHZh1b/gAKKDsWzDNC7QAAAABJRU5ErkJggg==')
            }
            #traffic-light:hover [data-type=fullscreen] {
                background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAAGXcA1uAAAAAXNSR0IArs4c6QAAAnlJREFUSA21VT1vGkEQze6RDxE5nYtEdmeLIgXU8R9IaVcuQGCltuRUbqjTuIuSOgkIrLiyf0DaNOmgs5IKRXKBLEXBPssIuLy3ulnvwh1ckHwSzOzMvPnYmblTD/BUq9X98Xh8psgopT5QqCkhY59arXbDgwITiVQLYzT8c7U8R1F0TQixTyhwH8UDwl4h7FNRAPFWeLrr4fdNBKpSqWwHQXAqAtJGo6G8tBxl6KXoKPJpitBkNV2HiSFwJqG1PkHaj0Tm0AGMn/Es9d2mGDoYc1OfWUUP0nVPM+egcVs/5+hnVFLcX2hWZrRTAgzBCwMQOXr1CbXs4pwXGWgIw41Wq3VBmQcol8truVzuC+Rb+PWR7lGz2fwI3j4WgOIT0wJoOJlMdhHBzKQBTI+NdecwSGuHIB3nbWfMsfFYNpWCoFQqHYM+9LQJB1xGUCwWLzlM7o0kmN6JADpMm747K59b/V/AdwJC30n6aTQa7XHnN9JNPM2g3W7/Drrd7hWq/4OCXntq54DmXaPjZtZsp7FAz7HFv2Dn3loI4xMYvxG8BYiAlGA06h0HEYAINLGxjAydolOMT10G1PXlBcCUHEBJx4kOXWASz4CQ11HBe9GbAHG55xAu3DkBLqADNKvAilS8YnTu3uUCfCZ1iLEoaDSqcQ/OmUGevjXu+1WmfJYwom9uQn8JbFZIn2/hI1hnXresnjlR9B10Op0f2DSu50uUFGR1MM8OjofQf8W41u0exJ+21rI7IAGZOZau4r2zRUnKQOh+E4aPESzpO+qaG54Zw/YWs18Vx2JkKxCBS7kjCLYN2Q4cbIKuxnp+kvgFPYXTM77VYvkM+QeCh/+ggmWf8AAAAABJRU5ErkJggg==')
            }
            #traffic-light [data-type] { 
                width: 12px;
                height: 12px;
                display: flex;
                align-items: center;
                justify-content: center;
                background-size: 12px;
            }
        \`
        document.getElementsByTagName('head')[0].appendChild(style);
    `
}
function dragBarJS(id) {
    return `
        let div = document.createElement('div')
        div.setAttribute('id', 'drag-bar')
        div.ondblclick = (e) => {
            if(e.target && e.target.nodeName.toUpperCase == 'SPAN') { 
            }
          window.require('electron').ipcRenderer.send('electron-traffic-light:${id}.toggleMax');
        };
        document.body.append(div)
    `
}
function trafficLightJS(id) {
    return `
        const trafficLight = document.createElement('div');
        trafficLight.setAttribute('id', 'traffic-light')
        trafficLight.innerHTML = \`
            <div data-type="close"></div>
            <div data-type="min"></div>
            <div data-type="fullscreen"></div>
        \`
        document.body.append(trafficLight)
        trafficLight.onclick = (e) => {
            if(e.target && e.target.dataset && ~['close', 'min'].indexOf(e.target.dataset.type)) {
                window.require('electron').ipcRenderer.send('electron-traffic-light:${id}.' + e.target.dataset.type);
            }
        }
    `
}

// TODO:
// * README
// * test
// * fullscreen then change icon 
// * options + fullscreen = max
// * blur => icon change to  grey
// * options: 
//     trafficlight: 
//       minmizable, maximizable, closable 
//       line-height, with

module.exports = function inject(win, options={}) {
    if(!win || !(win instanceof BrowserWindow)) return;
    const trafficLightEmitter = new EventEmitter();
    const { webContents } = win
    let bounds = options.bounds || win.getBounds()
    
    webContents.on('dom-ready', e => {
        let currentUrl = webContents.getURL()
        webContents.executeJavaScript(cssJS())
        if(!options.shouldAddTrafficLight || options.shouldAddTrafficLight(currentUrl)) {
            webContents.executeJavaScript(trafficLightJS(win.id))
        }

        if(!options.shouldAddDragBar || options.shouldAddDragBar(currentUrl)) {
            webContents.executeJavaScript(dragBarJS(win.id))
        }
    })
    
    // ipcHandle
    ipcMain.on(`electron-traffic-light:${win.id}.toggleMax`, () => {
        if(win.isFullScreen()) {
            win.setFullScreen(false);
            win.setBounds(bounds, true);
        } else if(win.isMaximized()) {
            // mainWindow.maximize();
            win.setBounds(bounds, true);
        } else {
            win.maximize()
        }
    })

    ipcMain.on(`electron-traffic-light:${win.id}.fullscreen`, () => {
        win.setFullScreen(!win.isFullScreen());
        if(win.isFullScreen()) {
            trafficLightEmitter.emit('fullscreen')
            win.setFullScreen(false);
        } else {
            trafficLightEmitter.emit('reset-fullscreen')
            win.setFullScreen(true);
        }
    })

    ipcMain.on(`electron-traffic-light:${win.id}.min`, () => {
        win.minimize()
        trafficLightEmitter.emit('set-minimize')
    })

    ipcMain.on(`electron-traffic-light:${win.id}.close`, () => {
        if (win.isFullScreen()) {
            win.setFullScreen(false);
            win.setBounds(bounds, true);
        } else {
            win.close();
        }
        trafficLightEmitter.emit('set-close')
    })

    return trafficLightEmitter
}