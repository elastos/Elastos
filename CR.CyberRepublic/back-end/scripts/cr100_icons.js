const s3 = require('s3')
const fs = require('fs')

const config = {
    bucket : "ebp-staging-files/cr100",
    region : "us-west-1"
}

const s3_client = s3.createClient({
    maxAsyncS3: 20,     // this is the default
    s3RetryCount: 3,    // this is the default
    s3RetryDelay: 1000, // this is the default
    multipartUploadThreshold: 20971520, // this is the default (20 MB)
    multipartUploadSize: 5242880, // 5 MB
    s3Options: {
        accessKeyId: "",            // <--- FILL THIS
        secretAccessKey: "",        // <--- FILL THIS
        region: config.region,
        // endpoint: 's3.yourdomain.com',
        // sslEnabled: false
        // any other options are passed to new AWS.S3()
        // See: http://docs.aws.amazon.com/AWSJavaScriptSDK/latest/AWS/Config.html#constructor-property
    }
});

function readFiles(dirname, onFileContent, onError) {
    fs.readdir(dirname, function(err, filenames) {
        if (err) {
            onError(err);
            return;
        }
        filenames.forEach(function(filename) {
            onFileContent(filename);
        });
    });
}

async function upload(file_name) {
    console.log('name:', file_name)
    // TODO: add retry
    const uploader = s3_client.uploadFile({
        localFile : 'cr100/' + file_name,
        s3Params : {
            ACL:'public-read',
            Bucket : config.bucket,
            Key : file_name
        }
    });

    return new Promise((resolve, reject)=>{
        // On S3 error
        uploader.on('error', (err)=>{
            // On error print the error to the console and send the error back as the response
            console.error("unable to upload:", err.stack);
            reject(err);
        });
        // On S3 success
        uploader.on('end', ()=>{

            // get public url
            const url = s3.getPublicUrl(config.bucket, file_name, config.region);
            resolve(url);

            console.log('uploaded', url)
        });
    });
}


(async () => {
    readFiles('cr100/', function(filename) {
        console.log('uploading file', filename, '...')
        upload(filename)
    }, function(err) {
        console.log(err)
        throw err;
    }
    );
})()