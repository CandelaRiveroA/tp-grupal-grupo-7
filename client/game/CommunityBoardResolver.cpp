#include "game/CommunityBoardResolver.h"

#include <algorithm>
#include <string>
#include <utility>

namespace game {

CommunityBoardResolver::CommunityBoardResolver(
	const world::CommunityCatalog& catalog)
	: catalog_(catalog) {}

std::vector<world::CommunitySection> CommunityBoardResolver::sectionsFrom(
	const LocalWorldView& view) const {
	std::vector<world::CommunitySection> sections;
	sections.reserve(catalog_.sets().size());
	for (const world::CommunitySetDef& def : catalog_.sets()) {
		const auto reported = std::find_if(
			view.communitySets.begin(), view.communitySets.end(),
			[&](const ViewCommunitySet& set) {
				return set.setId == def.id;
			});
		const bool known = reported != view.communitySets.end();

		world::CommunitySection section;
		section.title = def.title;
		section.helpText = def.helpText;
		section.done = known && reported->completed;
		section.donatable = def.donatable;
		section.rows.reserve(def.goals.size());
		for (const world::CommunityGoalDef& goalDef : def.goals) {
			ViewCommunityGoal progress;
			if (known) {
				const auto goal = std::find_if(
					reported->goals.begin(),
					reported->goals.end(),
					[&](const ViewCommunityGoal& candidate) {
						return candidate.goalId ==
						       goalDef.id;
					});
				if (goal != reported->goals.end()) {
					progress = *goal;
				}
			}
			world::CommunityRow row;
			row.label = goalDef.label;
			row.amount = std::to_string(progress.current) + " / " +
				     std::to_string(progress.total);
			row.done = progress.completed;
			row.visual = goalDef.visual;
			row.current = progress.current;
			row.total = progress.total;
			section.rows.push_back(std::move(row));
		}
		sections.push_back(std::move(section));
	}
	return sections;
}

}  // namespace game
