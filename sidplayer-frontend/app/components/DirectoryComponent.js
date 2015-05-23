const React = require('react');
const AppDispatcher = require('../dispatcher/AppDispatcher.js');
const ActionTypes = require('../constants/RevellerConstants.js').ActionTypes;

class DirectoryComponent extends React.Component {
	constructor() {
		super();
	}
	render() {
		return (
			<div className="directory" onClick={this.handleDirectoryClick.bind(this)}>{this.props.directoryName}</div>
		)
	}
	handleDirectoryClick() {
		AppDispatcher.dispatch({
			type: ActionTypes.LOAD_DIRECTORY,
			path: this.props.path
		});
	}
}

DirectoryComponent.proptypes = {
	directoryName: React.PropTypes.string.isRequired
};

export default DirectoryComponent;