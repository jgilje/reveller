
const SidfilesStore = require('../../stores/SidfilesStore.js');

const ActionTypes = require('../../constants/RevellerConstants.js').ActionTypes;
const AppDispatcher = require('../../dispatcher/AppDispatcher.js');

class SidplayerAPI {
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
		AppDispatcher.dispatch({
			type: ActionTypes.LOAD_DIRECTORY
		});
		this.ws.send(JSON.stringify(command));
	}
	handleLsResponse(responseString) {
		let response = JSON.parse(responseString);
		let dirs = response.directories;
		let sidfiles = response.sidfiles;
		let path = response.path.split('/');
		AppDispatcher.dispatch({
			type: ActionTypes.RECEIVE_DIRECTORY,
			directories: dirs,
			sidfiles: sidfiles,
			path: path
		});
	}
}

export default SidplayerAPI;