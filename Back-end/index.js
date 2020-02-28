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
  client.subscribe('302CEM/lion/#', function(err) {
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
 
// Regular middleware
// Note it's app.ws.use and not app.use
app.ws.use(function(ctx, next) {
  // return `next` to pass the context (ctx) on to the next ws middleware
  return next(ctx);
});


// Using routes
app.ws.use(route.all('/live', function (ctx) {
  // 'ctx' is the regular koa context created from the 'ws' onConnection 'socket.upgradeReq' object.
  // the websocket is added to the context on 'ctx.websocket'.
  ctx.websocket.on('message', function(message) {
    // do something with the message from client
    console.log("sending to broker")
    console.log(message);
    client.publish('302CEM/lion/esp32/led_control', message)
  });
  
  // Finally working - please DO NOT CHANGE!
  client.on('message', function(topic, message) {
    ctx.websocket.send(message.toString());
     
    message = message.toString()
    var data = JSON.parse(message)
    console.log(message)
    const dbStat = 'insert into Light_History (brightness) VALUES ( "'+data.value+'" )'
    data = {
      message: message
    }
    db.query(dbStat, data, (error, output)=>{
      if(error){
        console.log(error)
      } else {
        console.log(output)
      }        
    })
  }) 

  console.log(`[Socket] Topic: ${topic} Message: ${message}`);
 
}));
 
app.listen(8888);
console.log(`listening on port ${port}`)
