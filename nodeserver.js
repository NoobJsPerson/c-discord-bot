const { WebSocketServer } = require("ws");
const wss = new WebSocketServer({ port: 443 });

wss.on('connection', function connection(ws) {
	ws.send(JSON.stringify({
		heartbeat_interval: 4125,
		garbage: "noone cares"
	}))
	ws.on('message', function incoming(message) {
		const obj = JSON.parse(message.toString());
		console.log(obj);
		ws.send(JSON.stringify({op:11}))
	});
	setTimeout(() => ws.send(JSON.stringify({message:"Hello, World!"})),10000)
});  