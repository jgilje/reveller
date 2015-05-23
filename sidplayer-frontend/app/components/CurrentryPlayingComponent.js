const React = require('react');

class CurrentryPlayingComponent extends React.Component {
	constructor() {
		super();
	}
	render() {
		return (
			<div className="currentlyPlaying">
				{currentlyPlaying}
			</div>
		)
	}
}

CurrentryPlayingComponent.proptypes = {
	isPlaying: React.PropTypes.bool.isRequired,
	sidfileName: React.PropTypes.string
};

CurrentryPlayingComponent.defaultProps = {
	isPlaying: false,
	sidfileName: ''
};

export default CurrentryPlayingComponent;