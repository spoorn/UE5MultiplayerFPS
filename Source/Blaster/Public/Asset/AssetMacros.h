/**
* Macros for dealing with assets
 */

#pragma once

/// Load an asset from asset file path to a variable via assignment
#define LOAD_ASSET_TO_VARIABLE(AssetType, AssetPath, VariableToSet) if (const ConstructorHelpers::FObjectFinder<AssetType> AssetFile( \
TEXT(AssetPath)); AssetFile.Succeeded()) \
{ \
VariableToSet = AssetFile.Object; \
}
/// Load an asset from asset file path and pass to callback as an argument
#define LOAD_ASSET_TO_CALLBACK(AssetType, AssetPath, Callback) if (const ConstructorHelpers::FObjectFinder<AssetType> AssetFile( \
TEXT(AssetPath)); AssetFile.Succeeded()) \
{ \
Callback(AssetFile.Object); \
}