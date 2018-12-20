import _ from 'lodash';

/*
* request api method
*
* @param
* opts.path
* opts.method
* opts.header
* opts.data
* opts.cache
* opts.success
* opts.error
*
* TODO: add limit to qry
* TODO: doesn't this exist already somewhere?
* */
export const api_request = (opts = {})=>{

    const apiToken = sessionStorage.getItem('api-token');
    const headers = {};
    if(apiToken){
        headers['api-token'] = apiToken;
    }

    let server_url = process.env.SERVER_URL;
    opts = _.merge({
        method : 'get',
        headers,
        cache: 'no-cache',
        data : {},
        success : null,
        error : null,
        path : ''
    }, opts);
    server_url += opts.path;

    const method = opts.method.toLowerCase();
    const option = {
        headers : {
            'Content-Type': 'application/json',
            ...opts.headers
        },
        cache: opts.cache,
        method : opts.method,
        signal: opts.signal,
        mode: 'cors'
    };

    if(method === 'post' && option.headers['Content-Type'] === 'multipart/form-data'){
        const formData = new FormData();
        _.each(opts.data, (v, k)=>{
            formData.append(k, v);
        });
        option.body = formData;

        delete option.headers['Content-Type'];
    }
    else if(method !== 'get' && method !== 'head'){
        option.body = JSON.stringify(opts.data);
    }
    else{
        server_url += '?';
        _.each(opts.data, (value, key)=>{
            server_url += `${key}=${encodeURIComponent(value)}&`
        });
        server_url = server_url.replace(/&$/, '');
    }

    return fetch(server_url, option).then((response)=>{
        try {
            if (response.status === 200) {
                // fetch success
                return response.json()
            }
            else {
                throw new Error(response.statusText);
            }
        } catch(err) {
        }

    }).then((data)=>{
        if(data.code > 0){
            // return data correct
            opts.success && opts.success(data.data, data);
            return data.data;
        }
        else{
            opts.error && opts.error(data);
            throw new Error(data.error);
        }
    })
};

/*
*
* example
    upload_file(file, {
        error(e){
            console.error(e)
        }
    }).then((url)=>{
        console.log(url);
    });
*
* */
export const upload_file = async (fileObject, opts={})=>{

    try{
        const url = await api_request({
            path : '/api/upload/file',
            method : 'post',
            headers : {
                'Content-Type' : 'multipart/form-data'
            },
            data : {
                file : fileObject
            },
            compress: true
        });

        return {
            filename: fileObject.name,
            type: fileObject.type,
            ...url
        };
    }catch(e){
        opts.error && opts.error(e);
        throw e;
    }

};

