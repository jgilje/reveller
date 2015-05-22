
const SidfilesStore = require('../../stores/SidfilesStore.js');

class SidplayerController {
	constructor(url) {
		this.ws = new WebSocket(url);
		this.ws.onopen = () => {
			this.ls();
		}
		this.ws.onmessage = (message) => {
			this.messageHandler(message);
		};
	}
	messageHandler(message) {
		let response = JSON.parse(message.data);
		let type = response.type;
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
		let response = JSON.parse(responseString);
		let dirs = response.directories;
		let sidfiles = response.sidfiles;
		console.log(dirs, sidfiles);
	}
}

export default SidplayerController;