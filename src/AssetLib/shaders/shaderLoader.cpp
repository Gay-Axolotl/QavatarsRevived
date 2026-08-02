#include "AssetLib/shaders/shaderLoader.hpp"
#include "QavatarsRevived/Main.hpp"
#include <string_view>

#include "AssetLib/modelImporter.hpp"
#include "AssetLib/shaders/ShaderSO.hpp"
#include "UnityEngine/AssetBundleCreateRequest.hpp"
#include "UnityEngine/AssetBundleRequest.hpp"

#include "assets.hpp"

// NOTE: the original mod also sets VRMQavatars::MirrorManager::mirrorShader
// here. MirrorManager (in-game mirror rendering) is a separate, unrelated
// feature that hasn't been ported -- the mToon shader assignment below is
// the only piece material generation actually needs.

namespace AssetLib
{
    #define coro(...) custom_types::Helpers::CoroutineHelper::New(__VA_ARGS__)

    SafePtrUnity<UnityEngine::ScriptableObject> ShaderLoader::shaders;

    custom_types::Helpers::Coroutine ShaderLoader::LoadBund()
    {
        VRMLogger.info("Loading shaders...");
        UnityEngine::AssetBundle* ass;
        co_yield coro(ShaderLoader::LoadBundleFromMemoryAsync(Assets::shaders_sbund, ass));
        if (!ass)
        {
            VRMLogger.error("Couldn't load bundle from file, dieing...");
            co_return;
        }
        VRMLogger.info("Loaded Bundle");

        UnityEngine::ScriptableObject* data = nullptr;
        co_yield coro(ShaderLoader::LoadAssetFromBundleAsync(ass, "Assets/shaders.asset", csTypeOf(UnityEngine::ScriptableObject*), reinterpret_cast<UnityEngine::Object*&>(data)));
        if(data == nullptr)
        {
            VRMLogger.error("Couldn't load asset...");
            co_return;
        }
        ass->Unload(false);
        VRMLogger.info("Loaded asset");

        AssetLib::ModelImporter::mtoon = VRMData::ShaderSOFields::GetMToonShader(data);
        shaders = data;
        VRMLogger.info("Finished Loading assets");
        co_return;
    }

    custom_types::Helpers::Coroutine ShaderLoader::LoadBundleFromMemoryAsync(ArrayW<uint8_t> bytes, UnityEngine::AssetBundle*& out)
    {
        using AssetBundle_LoadFromMemoryAsync = function_ptr_t<UnityEngine::AssetBundleCreateRequest*, ArrayW<uint8_t>, int>;
        static auto assetBundle_LoadFromMemoryAsync = reinterpret_cast<AssetBundle_LoadFromMemoryAsync>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemoryAsync_Internal"));

        auto req = assetBundle_LoadFromMemoryAsync(bytes, 0);
        req->set_allowSceneActivation(true);
        while (!req->get_isDone())
            co_yield nullptr;

        out = req->get_assetBundle();
        UnityEngine::Object::DontDestroyOnLoad(out);
        co_return;
    }

    custom_types::Helpers::Coroutine ShaderLoader::LoadAssetFromBundleAsync(UnityEngine::AssetBundle* bundle, std::string_view name, System::Type* type, UnityEngine::Object*& out)
    {
        auto req = bundle->LoadAssetAsync(name, type);
        req->set_allowSceneActivation(true);
        while (!req->get_isDone())
            co_yield nullptr;

        out = req->get_asset();
        UnityEngine::Object::DontDestroyOnLoad(out);
        co_return;
    }
}
