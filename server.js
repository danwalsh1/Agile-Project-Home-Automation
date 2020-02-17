var server = require('http')

server.createServer(koa).listen(9000);

function koa(request, response) {

    response.writeHead(200, {'Content-Type': 'text/plain'});

    response.end('Its a fucking server')
}