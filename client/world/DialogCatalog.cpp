#include "world/DialogCatalog.h"

namespace world {

int DialogAnimation::frameIndexAt(const int animationTick) const {
	const int count = static_cast<int>(frames.size());
	if (count <= 1) {
		return 0;
	}
	const int step = frameTicks > 0 ? animationTick / frameTicks : 0;
	if (loops) {
		return step % count;
	}
	return step < count ? step : count - 1;
}

const DialogData* DialogCatalog::find(const Indication indication) const {
	const auto it = dialogs.find(indication);
	return it == dialogs.end() ? nullptr : &it->second;
}

}  // namespace world
