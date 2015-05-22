
class SidplayerController {
	constructor(url) {
		this.ws = new WebSocket(url);
		this.ws.onopen = () => {
			this.ls();
		}
		this.ws.onmessage = this.messageHandler.bind(this);
	}
	messageHandler(message) {
		let response = JSON.parse(message.data);
		let type = response.type;
		console.log(response);
		switch (type) {
			case 'ls':
				this.handleLsResponse(response.data);
				break;
		}
	}
	ls(args) {
		let command = {
			action: 'ls'
		};
		if (args) {
			command.argument = args
		}
		this.ws.send(JSON.stringify(command));
	}
	handleLsResponse(responseString) {
		console.log(responseString);
		let dirs = JSON.parse(responseString);
		console.log(dirs);
	}
}

export default SidplayerController;