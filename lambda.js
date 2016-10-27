const AWS = require('aws-sdk');
const SNS = new AWS.SNS();

exports.handler = (event, context, callback) => {
    const data = event.data;
    const message = `The Kitty says, "${data}"`;
    console.log(message);

    SNS.publish({
        Message: decodeURI(message),
        TopicArn: "arn:aws:sns:us-west-2:819664798373:coblr-photon"
    }, function(){
        callback(null, {'kittySays': data});
    });
};