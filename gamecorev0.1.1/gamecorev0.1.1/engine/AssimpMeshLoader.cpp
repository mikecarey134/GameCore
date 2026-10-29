#include "AssimpMeshLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <IAnimatedMeshSceneNode.h>
#include <IFileSystem.h>
#include <IReadFile.h>
#include <ISceneManager.h>
#include <ISkinnedMesh.h>
#include <IVideoDriver.h>
#include <SSkinMeshBuffer.h>
#include <coreutil.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <set>

using namespace irr;
namespace fs = std::filesystem;

namespace
{
	core::matrix4 toIrr(const aiMatrix4x4& m)
	{
		core::matrix4 result;
		for (int row = 0; row < 4; ++row)
			for (int column = 0; column < 4; ++column)
				result[column * 4 + row] = m[row][column];
		return result;
	}

	core::vector3df toIrr(const aiVector3D& v)
	{
		return core::vector3df(v.x, v.y, v.z);
	}

	core::quaternion toIrr(const aiQuaternion& q)
	{
		return core::quaternion(-q.x, -q.y, -q.z, q.w);
	}

	std::string lower(std::string text)
	{
		for (char& c : text)
			c = (char)std::tolower((unsigned char)c);
		return text;
	}

	void warn(const std::string& file, const std::string& message)
	{
		std::cerr << "Assimp: " << file << ": " << message << std::endl;
	}

	const aiScene* read(Assimp::Importer& importer, const std::string& path, unsigned int steps)
	{
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 65535);
		const aiScene* scene = importer.ReadFile(path, steps | aiProcess_ConvertToLeftHanded);
		if (!scene || !scene->mRootNode)
		{
			warn(path, importer.GetErrorString());
			return nullptr;
		}
		return scene;
	}

	struct Instance
	{
		const aiMesh* mesh;
		u32 buffer;
		aiMatrix4x4 parent;
	};

	struct Clip
	{
		const aiAnimation* animation;
		std::string name;
	};

	class Builder
	{
	public:
		Builder(scene::ISceneManager* smgr, scene::ISkinnedMesh* mesh, const aiScene* scene, const std::string& path)
			: smgr_(smgr), mesh_(mesh), scene_(scene), path_(path) {}

		void addMeshes(const aiNode* node, const aiMatrix4x4& parent)
		{
			aiMatrix4x4 global = parent * node->mTransformation;
			for (unsigned int i = 0; i < node->mNumMeshes; ++i)
			{
				const aiMesh* source = scene_->mMeshes[node->mMeshes[i]];
				if (source->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)
					addMesh(source, global, node->mParent ? node->mParent->mTransformation : aiMatrix4x4());
			}
			for (unsigned int i = 0; i < node->mNumChildren; ++i)
				addMeshes(node->mChildren[i], global);
		}

		bool isSkinned() const
		{
			for (const Instance& instance : instances_)
				if (instance.mesh->mNumBones > 0)
					return true;
			return false;
		}

		void addJoints(const aiNode* node, scene::ISkinnedMesh::SJoint* parent)
		{
			scene::ISkinnedMesh::SJoint* joint = mesh_->addJoint(parent);
			joint->Name = node->mName.C_Str();
			joint->LocalMatrix = toIrr(node->mTransformation);
			joints_[node->mName.C_Str()] = { joint, node->mTransformation };
			for (unsigned int i = 0; i < node->mNumChildren; ++i)
				addJoints(node->mChildren[i], joint);
		}

		void addWeights()
		{
			for (const Instance& instance : instances_)
				for (unsigned int b = 0; b < instance.mesh->mNumBones; ++b)
				{
					const aiBone* bone = instance.mesh->mBones[b];
					std::map<std::string, Joint>::iterator found = joints_.find(bone->mName.C_Str());
					if (found == joints_.end())
					{
						warn(path_, std::string("bone ") + bone->mName.C_Str() + " has no node, its weights were skipped");
						continue;
					}
					found->second.joint->GlobalInversedMatrix = toIrr(bone->mOffsetMatrix * aiMatrix4x4(instance.parent).Inverse());
					for (unsigned int w = 0; w < bone->mNumWeights; ++w)
					{
						scene::ISkinnedMesh::SWeight* weight = mesh_->addWeight(found->second.joint);
						weight->buffer_id = (u16)instance.buffer;
						weight->vertex_id = bone->mWeights[w].mVertexId;
						weight->strength = bone->mWeights[w].mWeight;
					}
				}
		}

		std::vector<AnimationClip> addClips(const std::vector<Clip>& clips)
		{
			std::set<std::string> animated;
			for (const Clip& clip : clips)
				for (unsigned int c = 0; c < clip.animation->mNumChannels; ++c)
					if (joints_.count(clip.animation->mChannels[c]->mNodeName.C_Str()))
						animated.insert(clip.animation->mChannels[c]->mNodeName.C_Str());

			std::vector<AnimationClip> result;
			s32 start = 0;
			for (const Clip& clip : clips)
			{
				double ticks = clip.animation->mTicksPerSecond != 0.0 ? clip.animation->mTicksPerSecond : 25.0;
				double toFrames = AssimpMeshLoader::FRAMES_PER_SECOND / ticks;
				s32 end = start + (s32)std::ceil(clip.animation->mDuration * toFrames);
				for (const std::string& name : animated)
					addKeys(joints_[name], channel(clip.animation, name), (f32)start, (f32)end, toFrames);
				result.push_back(AnimationClip{ clip.name, start, end });
				start = end + 1;
			}
			return result;
		}

	private:
		struct Joint
		{
			scene::ISkinnedMesh::SJoint* joint;
			aiMatrix4x4 bind;
		};

		static const aiNodeAnim* channel(const aiAnimation* animation, const std::string& name)
		{
			for (unsigned int c = 0; c < animation->mNumChannels; ++c)
				if (name == animation->mChannels[c]->mNodeName.C_Str())
					return animation->mChannels[c];
			return nullptr;
		}

		void addKeys(const Joint& joint, const aiNodeAnim* keys, f32 start, f32 end, double toFrames)
		{
			aiVector3D scale;
			aiQuaternion rotation;
			aiVector3D position;
			aiMatrix4x4(joint.bind).Decompose(scale, rotation, position);

			if (keys && keys->mNumPositionKeys)
				for (unsigned int k = 0; k < keys->mNumPositionKeys; ++k)
					positionKey(joint, start + (f32)(keys->mPositionKeys[k].mTime * toFrames), keys->mPositionKeys[k].mValue);
			else
				for (f32 frame : { start, end })
					positionKey(joint, frame, position);

			if (keys && keys->mNumRotationKeys)
				for (unsigned int k = 0; k < keys->mNumRotationKeys; ++k)
					rotationKey(joint, start + (f32)(keys->mRotationKeys[k].mTime * toFrames), keys->mRotationKeys[k].mValue);
			else
				for (f32 frame : { start, end })
					rotationKey(joint, frame, rotation);

			if (keys && keys->mNumScalingKeys)
				for (unsigned int k = 0; k < keys->mNumScalingKeys; ++k)
					scaleKey(joint, start + (f32)(keys->mScalingKeys[k].mTime * toFrames), keys->mScalingKeys[k].mValue);
			else
				for (f32 frame : { start, end })
					scaleKey(joint, frame, scale);
		}

		void positionKey(const Joint& joint, f32 frame, const aiVector3D& value)
		{
			scene::ISkinnedMesh::SPositionKey* key = mesh_->addPositionKey(joint.joint);
			key->frame = frame;
			key->position = toIrr(value);
		}

		void rotationKey(const Joint& joint, f32 frame, const aiQuaternion& value)
		{
			scene::ISkinnedMesh::SRotationKey* key = mesh_->addRotationKey(joint.joint);
			key->frame = frame;
			key->rotation = toIrr(value);
		}

		void scaleKey(const Joint& joint, f32 frame, const aiVector3D& value)
		{
			scene::ISkinnedMesh::SScaleKey* key = mesh_->addScaleKey(joint.joint);
			key->frame = frame;
			key->scale = toIrr(value);
		}

		void addMesh(const aiMesh* source, const aiMatrix4x4& global, const aiMatrix4x4& parent)
		{
			scene::SSkinMeshBuffer* buffer = mesh_->addMeshBuffer();
			instances_.push_back(Instance{ source, mesh_->getMeshBufferCount() - 1, parent });

			const aiMaterial* material = scene_->mMaterials[source->mMaterialIndex];
			buffer->Material.setTexture(0, texture(material));
			video::SColor colour(255, 255, 255, 255);
			aiColor4D diffuse;
			if (!buffer->Material.getTexture(0) && material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
				colour = video::SColorf(diffuse.r, diffuse.g, diffuse.b, 1.0f).toSColor();

			aiMatrix3x3 normals = aiMatrix3x3(global).Inverse().Transpose();
			for (unsigned int v = 0; v < source->mNumVertices; ++v)
			{
				aiVector3D normal = source->HasNormals() ? (normals * source->mNormals[v]).Normalize() : aiVector3D(0.0f, 1.0f, 0.0f);
				aiVector3D uv = source->HasTextureCoords(0) ? source->mTextureCoords[0][v] : aiVector3D();
				buffer->Vertices_Standard.push_back(video::S3DVertex(toIrr(global * source->mVertices[v]), toIrr(normal), colour,
					core::vector2df(uv.x, uv.y)));
			}
			for (unsigned int f = 0; f < source->mNumFaces; ++f)
				if (source->mFaces[f].mNumIndices == 3)
					for (unsigned int i = 0; i < 3; ++i)
						buffer->Indices.push_back((u16)source->mFaces[f].mIndices[i]);
		}

		video::ITexture* texture(const aiMaterial* material)
		{
			aiString name;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &name) != AI_SUCCESS)
				return nullptr;
			video::IVideoDriver* driver = smgr_->getVideoDriver();
			io::IFileSystem* files = smgr_->getFileSystem();
			if (const aiTexture* embedded = scene_->GetEmbeddedTexture(name.C_Str()))
			{
				if (embedded->mHeight != 0)
				{
					warn(path_, std::string("uncompressed embedded texture ") + name.C_Str() + " was skipped");
					return nullptr;
				}
				io::IReadFile* file = files->createMemoryReadFile(embedded->pcData, (s32)embedded->mWidth,
					(path_ + "#" + name.C_Str()).c_str(), false);
				video::ITexture* result = driver->getTexture(file);
				file->drop();
				return result;
			}
			std::string relative = name.C_Str();
			std::replace(relative.begin(), relative.end(), '\\', '/');
			fs::path folder = fs::path(path_).parent_path();
			for (const fs::path& candidate : { folder / relative, folder / fs::path(relative).filename() })
				if (files->existFile(candidate.string().c_str()))
					return driver->getTexture(candidate.string().c_str());
			warn(path_, "texture " + relative + " is missing");
			return nullptr;
		}

		scene::ISceneManager* smgr_;
		scene::ISkinnedMesh* mesh_;
		const aiScene* scene_;
		std::string path_;
		std::vector<Instance> instances_;
		std::map<std::string, Joint> joints_;
	};

	std::vector<fs::path> clipFiles(const std::string& path)
	{
		std::vector<fs::path> files;
		fs::path model(path);
		std::string prefix = lower(model.stem().string()) + "@";
		std::error_code error;
		for (fs::directory_iterator entry(model.parent_path().empty() ? fs::path(".") : model.parent_path(), error), end;
			!error && entry != end; entry.increment(error))
		{
			std::string name = lower(entry->path().filename().string());
			if (name.compare(0, prefix.size(), prefix) == 0 && lower(entry->path().extension().string()) == ".fbx")
				files.push_back(entry->path());
		}
		std::sort(files.begin(), files.end());
		return files;
	}
}

bool AssimpMeshLoader::isALoadableFileExtension(const io::path& filename) const
{
	return core::hasFileExtension(filename, "fbx");
}

scene::IAnimatedMesh* AssimpMeshLoader::createMesh(io::IReadFile* file)
{
	std::string path = file->getFileName().c_str();
	Assimp::Importer importer;
	const aiScene* scene = read(importer, path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
		aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights | aiProcess_SplitLargeMeshes | aiProcess_SortByPType);
	if (!scene)
		return nullptr;

	scene::ISkinnedMesh* mesh = smgr_->createSkinnedMesh();
	Builder builder(smgr_, mesh, scene, path);
	builder.addMeshes(scene->mRootNode, aiMatrix4x4());
	if (mesh->getMeshBufferCount() == 0)
	{
		warn(path, "has no triangle meshes");
		mesh->drop();
		return nullptr;
	}

	std::vector<AnimationClip> clips;
	if (builder.isSkinned())
	{
		builder.addJoints(scene->mRootNode, nullptr);
		builder.addWeights();

		std::vector<Clip> sources;
		for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
		{
			std::string name = scene->mAnimations[a]->mName.C_Str();
			sources.push_back(Clip{ scene->mAnimations[a], name.empty() ? "clip" + std::to_string(a) : name });
		}
		std::vector<std::unique_ptr<Assimp::Importer> > siblings;
		for (const fs::path& clipFile : clipFiles(path))
		{
			siblings.emplace_back(new Assimp::Importer());
			const aiScene* clipScene = read(*siblings.back(), clipFile.string(), 0);
			if (!clipScene)
				continue;
			std::string stem = clipFile.stem().string();
			std::string clipName = stem.substr(stem.find('@') + 1);
			for (unsigned int a = 0; a < clipScene->mNumAnimations; ++a)
				sources.push_back(Clip{ clipScene->mAnimations[a],
					clipScene->mNumAnimations == 1 ? clipName : clipName + "/" + clipScene->mAnimations[a]->mName.C_Str() });
		}
		clips = builder.addClips(sources);
	}
	else if (scene->mNumAnimations > 0)
		warn(path, "animation without bones is not supported, the mesh was loaded without it");

	mesh->setAnimationSpeed((f32)FRAMES_PER_SECOND);
	mesh->finalize();
	if (!clips.empty())
		clips_[mesh] = clips;
	return mesh;
}

const std::vector<AnimationClip>& AssimpMeshLoader::clips(const scene::IAnimatedMesh* mesh) const
{
	static const std::vector<AnimationClip> none;
	std::map<const scene::IAnimatedMesh*, std::vector<AnimationClip> >::const_iterator found = clips_.find(mesh);
	return found == clips_.end() ? none : found->second;
}

bool AssimpMeshLoader::play(scene::IAnimatedMeshSceneNode* node, const std::string& clip, bool loop) const
{
	for (const AnimationClip& candidate : clips(node->getMesh()))
		if (candidate.name == clip)
		{
			node->setFrameLoop(candidate.start, candidate.end);
			node->setLoopMode(loop);
			return true;
		}
	return false;
}

AssimpMeshLoader* addAssimpMeshLoader(scene::ISceneManager* smgr)
{
	AssimpMeshLoader* loader = new AssimpMeshLoader(smgr);
	smgr->addExternalMeshLoader(loader);
	loader->drop();
	return loader;
}
