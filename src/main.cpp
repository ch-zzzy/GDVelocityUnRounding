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
static void applyNopPatch(uintptr_t rva, size_t size) {
	const ByteVector nops(size, 0x90);
	
	auto res = Mod::get()->patch(
        reinterpret_cast<void*>(base::get() + rva),
        nops
    );

    if (res.isErr()) {
        log::error(
            "Failed to apply patch at address 0x{:X}: {}",
            rva,
            res.unwrapErr()
        );
    }
}

$on_mod(Loaded) {
	applyNopPatch(0x38C329, 0x24); // updateJump yvel rounding
	applyNopPatch(0x213EA2, 0x32); // checkCollisions yvel rounding
	applyNopPatch(0x39323B, 0x40); // collidedWithObjectInternal yvel rounding
	applyNopPatch(0x38DAC7, 0x38); // postCollision yvel rounding
	applyNopPatch(0x39FF18, 0x32); // boostPlayer yvel rounding
}
#endif