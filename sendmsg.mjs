import fetch from "node-fetch";
fetch("https://discord.com/api/v8/channels/777288013981155339/messages",{
	method: 'post',
	body: JSON.stringify({
		content: "PONG!"
	}),
	headers: {'Content-Type': 'application/json',
				'Authorization':'Bot NzU2MTQwMDMyOTc2NDg2NDQw.GT9jTi.e7xhPkI3T9B7g8hBXQLK53Q-AGnJq12P7napGY' }
}).then(console.log);