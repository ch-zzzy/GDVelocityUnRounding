#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// Removes rounding from PlayerObject::setYVelocity
class $modify(PlayerObject) {
	void setYVelocity(double velocity, int type) {
		this->m_yVelocity = velocity;
	}
};

#ifdef GEODE_IS_WINDOWS
static void ApplyNopPatch(ptrdiff_t offset, size_t size) {
	std::vector<uint8_t> nops(size, 0x90);
	Mod::get()->patch(
		reinterpret_cast<void*>(base::get() + offset),
		nops
	);
}

$on_mod(Loaded) {
	// PlayerObject patches
	ApplyNopPatch(0x38C329, 0x24); // updateJump yvel rounding 
	ApplyNopPatch(0x213EA2, 0x32); // checkCollisions yvel rounding
	ApplyNopPatch(0x39323B, 0x40); // collidedWithObjectInternal yvel rounding
	ApplyNopPatch(0x38DAC7, 0x38); // postCollision yvel rounding
	ApplyNopPatch(0x39FF18, 0x32); // boostPlayer yvel rounding
}
#endif