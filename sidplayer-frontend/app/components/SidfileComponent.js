const React = require('react');
const AppDispatcher = require('../dispatcher/AppDispatcher.js');
const ActionTypes = require('../constants/RevellerConstants.js').ActionTypes;

class SidfileComponent extends React.Component {
	constructor() {
		super();
	}
	render() {
		return (
			<div className="sidfile" onClick={this.handleFileClick.bind(this)}>{this.props.filename}</div>
		)
	}
	handleFileClick() {
		console.log(this.prop.filename)
	}
}

SidfileComponent.proptypes = {
	filename: React.PropTypes.string.isRequired
};

export default SidfileComponent;