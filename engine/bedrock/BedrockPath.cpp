#include "BedrockPath.hpp"

#include "BedrockAssert.hpp"
#include "BedrockPath.hpp"
#include <filesystem>

#define VALUE(string) #string
#define TO_LITERAL(string) VALUE(string)

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
#if defined(ASSET_DIR)
	mAssetPath = std::filesystem::absolute(std::string(TO_LITERAL(ASSET_DIR))).string();
#endif
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
