
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
		this.dispatchToken = AppDispatcher.register(action => {
			switch (action.type) {
				case ActionTypes.LOAD_DIRECTORY:
					console.log(action.path);
					let path = action.path.join('/');
					this.ls(path);
					break;
			}
		});
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
	ls(path) {
		let command = {
			action: 'ls'
		};
		if (path) {
			command.argument = path;
		}
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