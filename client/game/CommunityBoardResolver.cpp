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
	for (const world::CommunitySetDef& def : catalog_.sets()) {
		const auto reported = std::find_if(
			view.communitySets.begin(), view.communitySets.end(),
			[&](const ViewCommunitySet& set) {
				return set.setId == def.id;
			});
		const bool known = reported != view.communitySets.end();

		world::CommunitySection section;
		section.title = def.title;
		section.status = catalog_.status();
		section.done = known && reported->completed;
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
			row.amount = def.amountPrefix +
				     std::to_string(progress.current) + " / " +
				     def.amountPrefix +
				     std::to_string(progress.total);
			row.label = goalDef.label;
			row.done = progress.completed;
			if (row.label.empty()) {
				row.label = row.amount;
				row.amount.clear();
			}
			section.rows.push_back(std::move(row));
		}
		sections.push_back(std::move(section));
	}
	return sections;
}

}
