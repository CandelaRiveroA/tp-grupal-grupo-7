#include "common/model/InviteCode.h"

#include <set>
#include <string>

#include "gtest/gtest.h"

TEST(InviteCodeTest, AcceptsSixAllowedChars) {
	const InviteCode inviteCode;

	EXPECT_TRUE(inviteCode.isValid("ABC234"));
	EXPECT_TRUE(inviteCode.isValid("ZZZZZZ"));
}

TEST(InviteCodeTest, RejectsWrongLength) {
	const InviteCode inviteCode;

	EXPECT_FALSE(inviteCode.isValid(""));
	EXPECT_FALSE(inviteCode.isValid("ABC23"));
	EXPECT_FALSE(inviteCode.isValid("ABC2345"));
}

TEST(InviteCodeTest, RejectsAmbiguousAndInvalidChars) {
	const InviteCode inviteCode;

	EXPECT_FALSE(inviteCode.isValid("ABC0EF"));
	EXPECT_FALSE(inviteCode.isValid("ABCOEF"));
	EXPECT_FALSE(inviteCode.isValid("ABC1EF"));
	EXPECT_FALSE(inviteCode.isValid("ABCIEF"));
	EXPECT_FALSE(inviteCode.isValid("abc234"));
	EXPECT_FALSE(inviteCode.isValid("ABC-23"));
}

TEST(InviteCodeTest, GeneratesCodesWithValidFormat) {
	InviteCode inviteCode;

	for (int i = 0; i < 500; ++i) {
		EXPECT_TRUE(inviteCode.isValid(inviteCode.generate()));
	}
}

TEST(InviteCodeTest, GeneratesVarietyOfCodes) {
	InviteCode inviteCode;
	std::set<std::string> codes;

	for (int i = 0; i < 100; ++i) {
		codes.insert(inviteCode.generate());
	}
	EXPECT_EQ(codes.size(), 100);
}

TEST(InviteCodeTest, IndependentInstancesDrawIndependently) {
	InviteCode first;
	InviteCode second;

	EXPECT_NE(first.generate(), second.generate());
}
