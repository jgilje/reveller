const React = require('react');

const DirectoryComponent = require('./DirectoryComponent.js');
const Sidfile = require('../components/FilelistComponent.js');

class FilelistComponent extends React.Component {
	constructor() {
		super();
	}
	render() {
		let dirs = this.props.directories.map((d,i) => {
			return <DirectoryComponent directoryName={d} key={i}/>
		});
		return (
			<div className="filelist">
				{dirs}
			</div>
		)
	}
}

FilelistComponent.proptypes = {
	directories: React.PropTypes.arrayOf(React.PropTypes.string),
	sidfiles: React.PropTypes.arrayOf(React.PropTypes.string),
	path: React.PropTypes.arrayOf(React.PropTypes.string)
};

FilelistComponent.defaultProps = {
	directories: [],
	sidfiles: [],
	path: []
};

export default FilelistComponent;