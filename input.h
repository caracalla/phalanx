#pragma once

namespace Input {
	struct KeyStates {
		bool forward = false;
		bool reverse = false;
		bool left = false;
		bool right = false;
		bool rise = false;
		bool fall = false;
	};

	struct MouseState {
		double xOffset = 0;
		double yOffset = 0;

		void reset() {
			// prevent drift from old inputs sticking around
			xOffset = 0;
			yOffset = 0;
		}
	};
}
