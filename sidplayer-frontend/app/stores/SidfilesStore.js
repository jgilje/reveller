const EventEmitter = require('events');
const AppDispatcher = require('../dispatcher/AppDispatcher.js');
const ActionTypes = require('../constants/RevellerConstants.js').ActionTypes;

const CHANGE_EVENT = 'change';

let _isLoading = false;
let _sidfiles = [];
let _directories = [];
let _path = [];

class SidfilesStore extends EventEmitter {
	constructor() {
		super();
	}
	emitChange() {
		this.emit(CHANGE_EVENT);
	}
	addChangeListener(cb) {
		this.on(CHANGE_EVENT, cb);
	}
	removeChangeListener(cb) {
		this.removeEventListener(CHANGE_EVENT, cb);
	}
	getAll() {
		return {
			sidfiles: _sidfiles,
			directories: _directories,
			path: _path
		}
	}
	isLoading() {
		return _isLoading
	}
}

const sfs = new SidfilesStore();

sfs.dispatchToken = AppDispatcher.register((action) => {
	switch (action.type) {
		case ActionTypes.LOAD_DIRECTORY:
			_isLoading = true;
			sfs.emitChange();
			break;
		case ActionTypes.RECEIVE_DIRECTORY:
			_isLoading = false;
			_sidfiles = action.sidfiles;
			_directories = action.directories;
			_path = action.path;
			sfs.emitChange();
			break;
	}
});

export default sfs;