const React = require('react');

const Directory = require('./DirectoryComponent.js');
const Sidfile = require('../components/SidfileComponent.js');

class FilelistComponent extends React.Component {
	constructor() {
		super();
	}
	render() {

		let dirs = this.props.directories.map((d,i) => {
			return <Directory directoryName={d} path={this.props.path} key={i}/>
		});
		let files = this.props.sidfiles.map((f,i) => {
			return <Sidfile filename={f} key={i} />
		});
		return (
			<div className="filelist">
				{dirs}
				{files}
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