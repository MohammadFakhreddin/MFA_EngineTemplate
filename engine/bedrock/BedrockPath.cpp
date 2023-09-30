#include "BedrockPath.hpp"

#include "BedrockAssert.hpp"
#include "BedrockPath.hpp"
#include <filesystem>

//-------------------------------------------------------------------------------------------------

std::shared_ptr<MFA::Path> MFA::Path::Instantiate()
{
	return std::make_shared<Path>();
}

//-------------------------------------------------------------------------------------------------

MFA::Path::Path()
{
	MFA_ASSERT(Instance == nullptr);
	Instance = this;

	char addressBuffer[256]{};
	int stringSize = 0;
#if defined(__PLATFORM_WIN__) || defined(__PLATFORM_LINUX__)
	stringSize = sprintf(addressBuffer, "../assets/");
#else
#error "Platform not supported"
#endif
	mAssetPath = std::filesystem::absolute(std::string(addressBuffer, stringSize)).string();
}

//-------------------------------------------------------------------------------------------------

MFA::Path::~Path()
{
	MFA_ASSERT(Instance != nullptr);
	Instance = nullptr;
}

//-------------------------------------------------------------------------------------------------

std::string MFA::Path::Get(std::string const& address) const
{
	return std::filesystem::path(mAssetPath).append(address).string();
}

//-------------------------------------------------------------------------------------------------
