const React = require('react');


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
		// set store to this directory
	}
}

DirectoryComponent.proptypes = {
	directoryName: React.PropTypes.string.isRequired
};

export default DirectoryComponent;