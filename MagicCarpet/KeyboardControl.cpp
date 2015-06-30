#include "KeyboardControl.h"

KeyboardControl::KeyboardControl(Player* player, MastEventReceiver* eventReceiver) : Control(player) {
	this->eventReceiver = eventReceiver;
}

void KeyboardControl::poll() {
	accelerating = eventReceiver->keyDown(KEY_KEY_W);
	decelerating = eventReceiver->keyDown(KEY_KEY_S);
	turningLeft = eventReceiver->keyDown(KEY_KEY_A);
	turningRight = eventReceiver->keyDown(KEY_KEY_D);
	punchingLeft = eventReceiver->keyPressed(KEY_KEY_Q);
	punchingRight = eventReceiver->keyPressed(KEY_KEY_E);
}
