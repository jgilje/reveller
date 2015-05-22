const React = require('react');
const SidplayerController = require('./controllers/SidplayerController');
const sidplayer = new SidplayerController('ws://localhost:8080/ws');

class RevellerApp extends React.Component {
	constructor() {
		super();
	}
	render() {
		return (
			<div className="yolo">lolorama</div>
		)
	}
}

React.render(<RevellerApp/>, document.getElementById('reveller'));