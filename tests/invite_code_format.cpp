#include "common/model/draft/ProvisionalInviteCodeFormat.h"
#include "gtest/gtest.h"

TEST(ProvisionalInviteCodeFormatTest, AcceptsSixAllowedChars) {
    const ProvisionalInviteCodeFormat format;

    EXPECT_TRUE(format.isValid("ABC234"));
    EXPECT_TRUE(format.isValid("ZZZZZZ"));
}

TEST(ProvisionalInviteCodeFormatTest, RejectsWrongLength) {
    const ProvisionalInviteCodeFormat format;

    EXPECT_FALSE(format.isValid(""));
    EXPECT_FALSE(format.isValid("ABC23"));
    EXPECT_FALSE(format.isValid("ABC2345"));
}

TEST(ProvisionalInviteCodeFormatTest, RejectsAmbiguousAndInvalidChars) {
    const ProvisionalInviteCodeFormat format;

    EXPECT_FALSE(format.isValid("ABC0EF"));
    EXPECT_FALSE(format.isValid("ABCOEF"));
    EXPECT_FALSE(format.isValid("ABC1EF"));
    EXPECT_FALSE(format.isValid("ABCIEF"));
    EXPECT_FALSE(format.isValid("abc234"));
    EXPECT_FALSE(format.isValid("ABC-23"));
}
