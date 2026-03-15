#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// Removes rounding from PlayerObject::setYVelocity
class $modify(PlayerObject) {
	void setYVelocity(double velocity, int type) {
		this->m_yVelocity = velocity;
	}
};

// Patches the rounding from PlayerObject::updateJump
#ifdef GEODE_IS_WINDOWS
$on_mod(Loaded) {
	std::vector<uint8_t> nops(0x26, 0x90);
	Mod::get()->patch(
		reinterpret_cast<void*>(base::get() + 0x38C327),
		nops
	);
}
#endif