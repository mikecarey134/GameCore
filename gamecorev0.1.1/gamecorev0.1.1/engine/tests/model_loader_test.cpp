#include "doctest.h"
#include "AssimpMeshLoader.h"
#include <irrlicht.h>
#include <filesystem>
#include <fstream>
#include <set>
#include <vector>
#include <string>

using namespace irr;
using namespace core;
using namespace scene;

namespace
{
	struct Models
	{
		Models() : device(createDevice(video::EDT_NULL)), smgr(device->getSceneManager()), loader(addAssimpMeshLoader(smgr)) {}
		~Models() { device->drop(); }

		ISkinnedMesh* load(const char* name)
		{
			IAnimatedMesh* mesh = smgr->getMesh((std::string(MODEL_DATA_DIR) + "/" + name).c_str());
			return mesh && mesh->getMeshType() == EAMT_SKINNED ? static_cast<ISkinnedMesh*>(mesh) : nullptr;
		}

		IrrlichtDevice* device;
		ISceneManager* smgr;
		AssimpMeshLoader* loader;
	};

	s32 vertexAt(IMeshBuffer* buffer, const vector3df& position)
	{
		for (u32 i = 0; i < buffer->getVertexCount(); ++i)
			if (buffer->getPosition(i).equals(position, 0.001f))
				return (s32)i;
		return -1;
	}

	vector3df positionAt(ISkinnedMesh* mesh, s32 frame, s32 vertex)
	{
		return mesh->getMesh(frame)->getMeshBuffer(0)->getPosition((u32)vertex);
	}

	ISkinnedMesh::SJoint* joint(ISkinnedMesh* mesh, const char* name)
	{
		s32 index = mesh->getJointNumber(name);
		return index < 0 ? nullptr : mesh->getAllJoints()[index];
	}

	const vector3df topCorner(0.25f, 2.0f, -0.25f);
	const vector3df topAtRest(0.25f, 1.5f, -0.25f);
	const vector3df lowCorner(0.25f, 1.0f, -0.25f);
}

TEST_CASE("models: the loader takes fbx files and leaves the built-in formats to Irrlicht")
{
	Models models;
	CHECK(models.loader->isALoadableFileExtension("hero.fbx"));
	CHECK(models.loader->isALoadableFileExtension("models/HERO.FBX"));
	const char* builtIn[] = { "stick_dan.ms3d", "crate.obj", "sun.md2", "level.irrmesh", "map.b3d", "thing.x", "box.3ds" };
	for (const char* name : builtIn)
	{
		INFO(name);
		CHECK_FALSE(models.loader->isALoadableFileExtension(name));
	}
}

TEST_CASE("models: a rigged fbx loads every mesh, with each node's transform applied")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	REQUIRE(rig->getMeshBufferCount() == 2);
	CHECK(rig->getMeshBuffer(0)->getVertexCount() == 12);
	CHECK(rig->getMeshBuffer(0)->getIndexCount() == 60);
	REQUIRE(rig->getMeshBuffer(1)->getVertexCount() == 4);
	for (u32 i = 0; i < 4; ++i)
		CHECK(rig->getMeshBuffer(1)->getPosition(i).Y == doctest::Approx(-0.5f));
	CHECK(vertexAt(rig->getMeshBuffer(0), topCorner) >= 0);
	CHECK(vertexAt(rig->getMeshBuffer(0), vector3df(0.25f, 0.0f, 0.25f)) >= 0);
}

TEST_CASE("models: every node becomes a joint, keeping the bone chain")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	CHECK(rig->getJointCount() == 6);
	std::set<std::string> names;
	for (u32 i = 0; i < rig->getJointCount(); ++i)
		names.insert(rig->getJointName(i));
	CHECK(names == std::set<std::string>{ "RootNode", "Bone1", "Bone2", "Bone3", "Body", "Base" });
	REQUIRE(joint(rig, "Bone2") != nullptr);
	REQUIRE(joint(rig, "Bone2")->Children.size() == 1);
	CHECK(std::string(joint(rig, "Bone2")->Children[0]->Name.c_str()) == "Bone3");
}

TEST_CASE("models: clips from the file and from name@clip.fbx files sit end to end on one timeline")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	const std::vector<AnimationClip>& clips = models.loader->clips(rig);
	REQUIRE(clips.size() == 3);
	CHECK(clips[0].name == "bend");
	CHECK(clips[0].start == 0);
	CHECK(clips[0].end == 30);
	CHECK(clips[1].name == "wave");
	CHECK(clips[1].start == 31);
	CHECK(clips[1].end == 61);
	CHECK(clips[2].name == "nod");
	CHECK(clips[2].start == 62);
	CHECK(clips[2].end == 77);
	CHECK(rig->getAnimationSpeed() == doctest::Approx(30.0f));
}

TEST_CASE("models: wave turns the top of the column ninety degrees about Bone2 and leaves the rest still")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	s32 top = vertexAt(rig->getMesh(31)->getMeshBuffer(0), topAtRest);
	s32 low = vertexAt(rig->getMesh(31)->getMeshBuffer(0), lowCorner);
	REQUIRE(top >= 0);
	REQUIRE(low >= 0);

	vector3df waved = positionAt(rig, 61, top);
	CHECK(waved.X == doctest::Approx(-0.5f).epsilon(0.001));
	CHECK(waved.Y == doctest::Approx(1.25f).epsilon(0.001));
	CHECK(waved.Z == doctest::Approx(-0.25f).epsilon(0.001));
	CHECK(positionAt(rig, 61, low).equals(lowCorner, 0.001f));

	vector3df halfway = positionAt(rig, 46, top);
	CHECK(halfway.X < 0.25f);
	CHECK(halfway.X > -0.5f);
}

TEST_CASE("models: bend lifts Bone1 and tips the whole column forty-five degrees about it")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	s32 top = vertexAt(rig->getMesh(0)->getMeshBuffer(0), topAtRest);
	REQUIRE(top >= 0);
	vector3df bent = positionAt(rig, 30, top);
	CHECK(bent.X == doctest::Approx(0.25f).epsilon(0.001));
	CHECK(bent.Y == doctest::Approx(1.38388f).epsilon(0.001));
	CHECK(bent.Z == doctest::Approx(-1.23744f).epsilon(0.001));
}

TEST_CASE("models: a node that never animates keeps its rest transform")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	rig->getMesh(10);
	REQUIRE(joint(rig, "Base") != nullptr);
	CHECK(joint(rig, "Base")->GlobalAnimatedMatrix.getTranslation().equals(vector3df(0.0f, -0.5f, 0.0f), 0.001f));
}

TEST_CASE("models: nod from the sibling file turns Bone3 and leaves it in place")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	ISkinnedMesh::SJoint* head = joint(rig, "Bone3");
	REQUIRE(head != nullptr);
	rig->getMesh(62);
	matrix4 start = head->GlobalAnimatedMatrix;
	rig->getMesh(77);
	matrix4 end = head->GlobalAnimatedMatrix;
	CHECK_FALSE(start.equals(end, 0.01f));
	CHECK(end.getTranslation().equals(vector3df(0.0f, 2.0f, 0.0f), 0.001f));
	CHECK(end.getRotationDegrees().X == doctest::Approx(330.0f).epsilon(0.01));
	CHECK(end.getScale().Y == doctest::Approx(2.0f).epsilon(0.001));
	rig->getMesh(61);
	CHECK(head->GlobalAnimatedMatrix.getScale().Y == doctest::Approx(1.0f).epsilon(0.001));
}

TEST_CASE("models: play sets a node's frame loop by clip name")
{
	Models models;
	ISkinnedMesh* rig = models.load("rig.fbx");
	REQUIRE(rig != nullptr);
	IAnimatedMeshSceneNode* node = models.smgr->addAnimatedMeshSceneNode(rig);
	CHECK(models.loader->play(node, "wave", false));
	CHECK(node->getStartFrame() == 31);
	CHECK(node->getEndFrame() == 61);
	CHECK_FALSE(node->getLoopMode());
	CHECK(models.loader->play(node, "nod", true));
	CHECK(node->getStartFrame() == 62);
	CHECK(node->getLoopMode());
	CHECK_FALSE(models.loader->play(node, "dance", true));
	CHECK(node->getStartFrame() == 62);
}

TEST_CASE("models: at rest a skinned mesh sits where the file places it, whatever its node and parent do")
{
	Models models;
	ISkinnedMesh* turned = models.load("turned.fbx");
	REQUIRE(turned != nullptr);
	IMeshBuffer* body = turned->getMeshBuffer(0);
	std::vector<vector3df> placed;
	for (u32 i = 0; i < body->getVertexCount(); ++i)
		placed.push_back(body->getPosition(i));
	REQUIRE(vertexAt(body, vector3df(2.75f, 2.0f, 0.25f)) >= 0);

	turned->getMesh(0);
	for (u32 i = 0; i < body->getVertexCount(); ++i)
	{
		INFO("vertex ", i);
		CHECK(body->getPosition(i).equals(placed[i], 0.001f));
	}
}

TEST_CASE("models: an fbx without bones loads as a still mesh with no joints or clips")
{
	Models models;
	ISkinnedMesh* still = models.load("static.fbx");
	REQUIRE(still != nullptr);
	CHECK(still->getMeshBufferCount() == 1);
	CHECK(still->getJointCount() == 0);
	CHECK(models.loader->clips(still).empty());
}

TEST_CASE("models: a broken fbx gives no mesh instead of crashing")
{
	Models models;
	std::filesystem::path broken = std::filesystem::temp_directory_path() / "gamecore_broken_model.fbx";
	std::ofstream(broken) << "; FBX 7.4.0 project file\nthis is not an fbx\n";
	CHECK(models.smgr->getMesh(broken.string().c_str()) == nullptr);
	std::filesystem::remove(broken);
}
