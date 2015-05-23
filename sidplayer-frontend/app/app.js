const React = require('react');
const SidplayerAPI = require('./controllers/SidplayerAPI');
const sidplayer = new SidplayerAPI('ws://localhost:8080/ws');
const SidfilesStore = require('./stores/SidfilesStore.js');
const AppDispatcher = require('./dispatcher/AppDispatcher.js');
const ActionTypes = require('./constants/RevellerConstants.js').ActionTypes;

const Filelist = require('./components/FilelistComponent.js');

class RevellerApp extends React.Component {
	constructor() {
		super();
		this.changeHandler = this._onChange.bind(this);
		this.state = SidfilesStore.getAll();
	}
	componentDidMount() {
		SidfilesStore.addChangeListener(this.changeHandler);
	}
	componentWillUnmount() {
		SidfilesStore.removeChangeListener(this.changeHandler);
	}
	_onChange() {
		this.setState(SidfilesStore.getAll());
	}
	render() {
		return (
			<div className="reveller">
				<button className="backButton" onClick={this.back.bind(this)}>Back</button>
				<Filelist directories={this.state.directories} sidfiles={this.state.sidfiles} path={this.state.path} />
			</div>
		)
	}
	back() {
		let path = this.state.path.slice(0, this.state.path.length-1);
		if (path.length > 0) {
			AppDispatcher.dispatch({
				type: ActionTypes.LOAD_DIRECTORY,
				path: path
			})
		}
	}
}

React.render(<RevellerApp/>, document.getElementById('reveller'));