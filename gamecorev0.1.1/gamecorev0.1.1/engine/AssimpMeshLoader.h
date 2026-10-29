#ifndef ASSIMPMESHLOADER_H
#define ASSIMPMESHLOADER_H

#include <map>
#include <string>
#include <vector>
#include <IMeshLoader.h>

namespace irr { namespace scene { class ISceneManager; class IAnimatedMeshSceneNode; } }

struct AnimationClip
{
	std::string name;
	irr::s32 start;
	irr::s32 end;
};

class AssimpMeshLoader : public irr::scene::IMeshLoader
{
public:
	static const irr::s32 FRAMES_PER_SECOND = 30;

	explicit AssimpMeshLoader(irr::scene::ISceneManager* smgr) : smgr_(smgr) {}

	bool isALoadableFileExtension(const irr::io::path& filename) const override;
	irr::scene::IAnimatedMesh* createMesh(irr::io::IReadFile* file) override;

	const std::vector<AnimationClip>& clips(const irr::scene::IAnimatedMesh* mesh) const;
	bool play(irr::scene::IAnimatedMeshSceneNode* node, const std::string& clip, bool loop) const;

private:
	irr::scene::ISceneManager* smgr_;
	std::map<const irr::scene::IAnimatedMesh*, std::vector<AnimationClip> > clips_;
};

AssimpMeshLoader* addAssimpMeshLoader(irr::scene::ISceneManager* smgr);

#endif
