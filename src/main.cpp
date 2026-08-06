#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <tuple>
#include <vector>

using namespace geode::prelude;

struct PatchGroup {
	std::vector<Patch*> appliedPatches;

	void init(std::initializer_list<std::tuple<uintptr_t, size_t, uint8_t>> entries) {
		for (auto& [rva, size, newValue] : entries) {
			ByteVector potentialPatch(size, newValue);

			auto result =
				Mod::get()->patch(reinterpret_cast<void*>(base::get() + rva), potentialPatch);

			if (result.isOk()) {
				auto* patch = result.unwrap();
				patch->setAutoEnable(false);
				(void) patch->disable();
				appliedPatches.push_back(patch);
			} else {
				log::error("Failed to create patch at offset 0x{:X}: {}", rva, result.unwrapErr());
			}
		}
	}

	void toggle(bool enable) {
		for (auto* patch : appliedPatches) {
			(void) patch->toggle(enable);
		}
	}
};

static PatchGroup s_velocityUnroundingNops;
// clang-format off
static void toggleVelocityUnroundingPatches(bool enable) {
	if (s_velocityUnroundingNops.appliedPatches.empty()) {
		s_velocityUnroundingNops.init({

			#ifdef GEODE_IS_WINDOWS
			{0x38C329, 0x24, 0x90}, // updateJump yvel rounding
			{0x213EA2, 0x32, 0x90}, // checkCollisions yvel rounding
			{0x38DAC7, 0x38, 0x90}, // postCollision yvel rounding
			{0x39323B, 0x40, 0x90}, // collidedWithObjectInternal yvel rounding
			{0x39FF18, 0x32, 0x90}, // boostPlayer yvel rounding
			#endif

		});
	}

	s_velocityUnroundingNops.toggle(enable);
}
// clang-format on

static bool s_modEnabled = true;

$on_mod(Loaded) {
	s_modEnabled = Mod::get()->getSettingValue<bool>("mod-enabled");
	toggleVelocityUnroundingPatches(s_modEnabled);

	listenForSettingChanges<bool>("mod-enabled", [](bool val) {
		s_modEnabled = val;
		toggleVelocityUnroundingPatches(val);
	});
}

class $modify(PlayerObject) {
	void setYVelocity(double velocity, int type) {
		if (s_modEnabled) {
			this->m_yVelocity = velocity;
			return;
		}
		PlayerObject::setYVelocity(velocity, type);
	}
};
