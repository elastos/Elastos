import Base from './Base'
import * as s3 from 's3'
import * as uuid from 'uuid'
import * as fs from 'fs'

let s3_client: any = undefined

export default class extends Base {
    private config: any
    protected init(){
        this.config = {
            bucket : process.env.S3_BUCKET,
            region : process.env.S3_REGION
        }
        if(!s3_client){
            this.initS3Client()
        }
    }

    private initS3Client(){
        s3_client = s3.createClient({
            maxAsyncS3: 20,     // this is the default
            s3RetryCount: 3,    // this is the default
            s3RetryDelay: 1000, // this is the default
            multipartUploadThreshold: 20971520, // this is the default (20 MB)
            multipartUploadSize: 5242880, // 5 MB
            s3Options: {
                accessKeyId: process.env.AWS_ACCESS_KEY,
                secretAccessKey: process.env.AWS_ACCESS_SECRET,
                region: this.config.region,
                // endpoint: 's3.yourdomain.com',
                // sslEnabled: false
                // any other options are passed to new AWS.S3()
                // See: http://docs.aws.amazon.com/AWSJavaScriptSDK/latest/AWS/Config.html#constructor-property
            }
        })

    }

    /*
    * @param - file, req.files.file.
    * */
    public async saveFile(file){
        const file_name = uuid.v4()+'_'+file.name
        const path = process.cwd()+'/.upload/'

        if(!fs.existsSync(path)){
            fs.mkdirSync(path)
        }

        const res = await file.mv(path+file_name)

        // TODO: add retry
        const uploader = s3_client.uploadFile({
            localFile : path+file_name,
            s3Params : {
                ACL:'public-read',
                Bucket : this.config.bucket,
                Key : file_name
            }
        })

        return new Promise((resolve, reject)=>{
            // On S3 error
            uploader.on('error', (err)=>{
                // On error print the error to the console and send the error back as the response
                console.error('unable to upload:', err.stack)
                reject(err)
            })
            // On S3 success
            uploader.on('end', ()=>{
                // Removing file from server after uploaded to S3
                fs.unlinkSync(path+file_name)

                // get public url
                const url = s3.getPublicUrl(this.config.bucket, file_name, this.config.region)
                resolve(url)
            })
        })
    }

}
