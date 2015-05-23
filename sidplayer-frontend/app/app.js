const React = require('react');
const SidplayerAPI = require('./controllers/SidplayerAPI');
const sidplayer = new SidplayerAPI('ws://localhost:8080/ws');
const SidfilesStore = require('./stores/SidfilesStore.js');

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
				<Filelist directories={this.state.directories} sidfiles={this.state.sidfiles} path={this.state.path} />
			</div>
		)
	}
}

React.render(<RevellerApp/>, document.getElementById('reveller'));