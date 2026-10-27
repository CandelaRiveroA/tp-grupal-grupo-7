#include "gtest/gtest.h"

#include <vector>

#include "game/LocalWorldView.h"
#include "game/ResourceAudioCue.h"
#include "world/ResourceCatalog.h"

namespace {

using game::ResourceAudioCue;
using game::ResourceAudioEvent;
using game::ViewResource;

ViewResource mineral(int x, int y, world::ResourceState state) {
	ViewResource r;
	r.kind = world::ResourceKind::Silver;
	r.cellX = x;
	r.cellY = y;
	r.state = state;
	return r;
}

std::vector<ViewResource> with(ViewResource r) {
	return std::vector<ViewResource>{r};
}

TEST(ResourceAudioCueTest, FirstObservationDoesNotFire) {
	ResourceAudioCue cue;
	const ResourceAudioEvent event =
		cue.update(with(mineral(1, 1, world::ResourceState::Intact)));
	EXPECT_EQ(event, ResourceAudioEvent::None);
}

TEST(ResourceAudioCueTest, NoChangeDoesNotFire) {
	ResourceAudioCue cue;
	cue.update(with(mineral(1, 1, world::ResourceState::Intact)));
	const ResourceAudioEvent event =
		cue.update(with(mineral(1, 1, world::ResourceState::Intact)));
	EXPECT_EQ(event, ResourceAudioEvent::None);
}

TEST(ResourceAudioCueTest, StateChangeFiresHit) {
	ResourceAudioCue cue;
	cue.update(with(mineral(1, 1, world::ResourceState::Intact)));
	const ResourceAudioEvent event =
		cue.update(with(mineral(1, 1, world::ResourceState::Cracked)));
	EXPECT_EQ(event, ResourceAudioEvent::Hit);
}

TEST(ResourceAudioCueTest, DisappearingFiresBroke) {
	ResourceAudioCue cue;
	cue.update(with(mineral(1, 1, world::ResourceState::Broken)));
	// El recurso ya no está en la vista (se agotó).
	const ResourceAudioEvent event =
		cue.update(std::vector<ViewResource>{});
	EXPECT_EQ(event, ResourceAudioEvent::Broke);
}

TEST(ResourceAudioCueTest, BrokeTakesPriorityOverHit) {
	ResourceAudioCue cue;
	// Dos minerales conocidos.
	cue.update(std::vector<ViewResource>{
		mineral(1, 1, world::ResourceState::Broken),
		mineral(2, 2, world::ResourceState::Intact)});
	// Uno desaparece y el otro cambia de estado en el mismo frame:
	// gana Obtained.
	const ResourceAudioEvent event = cue.update(std::vector<ViewResource>{
		mineral(2, 2, world::ResourceState::Cracked)});
	EXPECT_EQ(event, ResourceAudioEvent::Broke);
}

TEST(ResourceAudioCueTest, EachResourceTrackedIndependently) {
	ResourceAudioCue cue;
	cue.update(std::vector<ViewResource>{
		mineral(1, 1, world::ResourceState::Intact),
		mineral(2, 2, world::ResourceState::Intact)});
	// Solo uno cambia: dispara Hit (el otro sigue igual, sin falso evento).
	const ResourceAudioEvent event = cue.update(std::vector<ViewResource>{
		mineral(1, 1, world::ResourceState::Intact),
		mineral(2, 2, world::ResourceState::Cracked)});
	EXPECT_EQ(event, ResourceAudioEvent::Hit);
}

}
