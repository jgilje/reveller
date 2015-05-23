const keymirror = require('keymirror');

export default {
	ActionTypes: keymirror({
		GET_DIRECTORY: null,
		LOADING_DIRECTORY: null,
		RECEIVE_DIRECTORY: null
	})
}