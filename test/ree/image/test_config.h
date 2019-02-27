#pragma once

#include <string>

#include <ree/image/types.hpp>

static const std::string &&kTestAssetsDir = "../test_assets/";

inline std::string reeToString(ree::image::ColorSpace t) {
	return t.ToString();
}