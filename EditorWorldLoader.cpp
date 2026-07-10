#include "EditorWorldLoader.h"
#include "Loader.h"
#include "libsiedler2/Archiv.h"
#include "libsiedler2/ArchivItem_Bitmap.h"
#include "libsiedler2/libsiedler2.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "ogl/glAllocator.h"
#include "resources/ResourceId.h"
#include <boost/filesystem.hpp>

void loadTexturesForEditorWorld(const std::string& gameDataPath)
{
    static bool initDone = false;
    if(initDone) return;
    initDone = true;

    libsiedler2::setAllocator(new GlAllocator());

    const std::string texFiles[] = {"GFX/TEXTURES/TEX5.LBM", "GFX/TEXTURES/TEX6.LBM", "GFX/TEXTURES/TEX7.LBM"};
    for(const auto& f : texFiles) {
        auto path = boost::filesystem::path(gameDataPath) / f;
        if(boost::filesystem::exists(path))
            LOADER.Load(path, nullptr);
    }
}
