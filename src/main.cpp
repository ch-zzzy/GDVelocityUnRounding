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
			{0x4EA5BC, 0x24, 0x90}, // updateJump
			{0x31B1CE, 0x32, 0x90}, // checkCollisions
			{0x4EBDF8, 0x38, 0x90}, // postCollision
			{0x4F146A, 0x3F, 0x90}, // collidedWithObjectInternal
			{0x4FE334, 0x26, 0x90}, // boostPlayer
			{0x501726, 0x32, 0x90}, // loadFromCheckpoint
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
