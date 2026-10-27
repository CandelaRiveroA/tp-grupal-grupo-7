#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "sdl/log/Logger.h"

namespace {

class ClogCapture {
public:
	ClogCapture() : previous_(std::clog.rdbuf(buffer_.rdbuf())) {}
	~ClogCapture() { std::clog.rdbuf(previous_); }

	std::string text() const { return buffer_.str(); }

private:
	std::ostringstream buffer_;
	std::streambuf* previous_;
};

TEST(LoggerTest, ErrorHasErrorPrefixAndMessage) {
	ClogCapture capture;
	const sdl::Logger logger;
	logger.error("fallo X");
	const std::string output = capture.text();
	EXPECT_NE(output.find("[SDL][ERROR]"), std::string::npos);
	EXPECT_NE(output.find("fallo X"), std::string::npos);
}

TEST(LoggerTest, WarningHasWarningPrefix) {
	ClogCapture capture;
	const sdl::Logger logger;
	logger.warning("ojo");
	EXPECT_NE(capture.text().find("[SDL][WARNING]"), std::string::npos);
}

TEST(LoggerTest, InfoHasInfoPrefix) {
	ClogCapture capture;
	const sdl::Logger logger;
	logger.info("arranca");
	EXPECT_NE(capture.text().find("[SDL][INFO]"), std::string::npos);
}

TEST(LoggerTest, EmitsOneLinePerCall) {
	ClogCapture capture;
	const sdl::Logger logger;
	logger.info("uno");
	logger.error("dos");
	const std::string output = capture.text();
	EXPECT_EQ(std::count(output.begin(), output.end(), '\n'), 2);
}

}
