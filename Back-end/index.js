const Koa = require('koa'),
  route = require('koa-route'),
  websockify = require('koa-websocket');

const mqtt = require('mqtt');
 
const fs = require('fs');

const app = websockify(new Koa());

const CAFile = fs.readFileSync('mqtt.crt');

const options = {username: '302CEM', password: 'n3fXXFZrjw', protocol: 'mqtts', ca: CAFile, host: 'mqtt.coventry.ac.uk', port: 8883};
const client = mqtt.connect('mqtt.coventry.ac.uk', options);
const port = 8888


client.on('connect', function() {
  client.subscribe('302CEM/lion/', function(err) {
    if(err) {
      console.log(err);
    }else{
      console.log('subscribed to broker!')
    }
  })
})

//Mysql
const mysql = require('mysql')
const db = mysql.createConnection({
    host: 'localhost',
    user: 'root',
    password: 'lion1234',
    database: 'mqttJS'
});
db.connect(function(err) {
  if(err) {
    console.log(err);
  }else{
    console.log('Database Connected')
  }
})

client.on('published', (packet)=>{
  message = packet.payload.toString()
  console.log(message)
  if(message.slice(0,1) != '{' && message.slice(0,4) != 'mqtt'){
    const dbStat = 'insert into Light_History set?'
    const data = {
      message: message
    }
    db.query(dbStat, data, (error, output)=>{
      if(error){
        console.log(error)
       } else {
          console.log(output)
        }
        
      })

    }
  })

//client.on('message', (topic, payload) => {
//  console.log(`Topic: ${topic} Message: ${payload}`);
//});
 
// Regular middleware
// Note it's app.ws.use and not app.use
app.ws.use(function(ctx, next) {
  // return `next` to pass the context (ctx) on to the next ws middleware
  return next(ctx);
});
 
// Using routes
app.ws.use(route.all('/live', function (ctx) {
  // `ctx` is the regular koa context created from the `ws` onConnection `socket.upgradeReq` object.
  // the websocket is added to the context on `ctx.websocket`.
  client.on('message', function(topic, message) {
    ctx.websocket.send(message.toString());
    console.log(`Topic: ${topic} Message: ${message}`);
  });
}));
 
app.listen(8888);
console.log(`listening on port ${port}`)
