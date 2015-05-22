const EventEmitter = require('events');

const CHANGE_EVENT = 'change';

class SidfilesStore extends EventEmitter {
	constructor() {
		super();
	}
	emitChange() {
		this.emit(CHANGE_EVENT);
	}
}

export default new SidfilesStore();