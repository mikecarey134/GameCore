#ifndef COMMMON_H
#define COMMMON_H

#include<string>
#include<vector>


	enum selectionType
	{
		// I use this ISceneNode ID to indicate a scene node that is
		// not pickable by getSceneNodeAndCollisionPointFromRay()
		ID_IsNotPickable = 0,

		// I use this flag in ISceneNode IDs to indicate that the
		// scene node can be picked by ray selection.
		IDFlag_IsPickable = 1 << 0,

		// I use this flag in ISceneNode IDs to indicate that the
		// scene node can be highlighted.  In this example, the
		// hominids can be highlighted, but the level mesh can't.
		IDFlag_IsHighlightable = 1 << 1
	};

	enum loadState { NOT_LOADED = 0, LOADING, DONE_LOADING };
	enum State{IDLE,JUMP,ATTACK,WALK,DAMAGED,DEAD,SPECTATOR};

	//enum char_models{DAN=0,MIKE,ZOMBIE};
	//enum char_skins{DAN_S=0,MIKE_S,ZOMBIE_S};

    #define PATH_DAN "characters/stick_dan.ms3d"
    #define PATH_MIKE "characters/stick_mike.ms3d"
	//#define PATH_MIKE "characters/proto/proto.ms3d"
    #define PATH_ZOMB "characters/stick_mike.ms3d"//"characters/zombie.ms3d"
	#define PATH_DAN_SKIN "characters/playerskin_d1.bmp"
	#define PATH_MIKE_SKIN "characters/playerskin_m1.jpg"
	//#define PATH_MIKE_SKIN "characters/proto/pskin.png"
	#define PATH_ZOMB_SKIN "characters/zombieskin.jpg"
	#define PATH_SLENDER_SKIN "characters/sm.jpg"
	struct common_paths{

		common_paths(){ model_paths.push_back(PATH_DAN);
			model_paths.push_back(PATH_MIKE);
			model_paths.push_back(PATH_ZOMB);
			model_paths.push_back(PATH_ZOMB);
			skin_paths.push_back(PATH_DAN_SKIN);
			skin_paths.push_back(PATH_MIKE_SKIN);
			skin_paths.push_back(PATH_ZOMB_SKIN);
			skin_paths.push_back(PATH_SLENDER_SKIN);
		}
	
		const static int SIZE=3;
		std::vector<std::string> model_paths;
		std::vector<std::string> skin_paths;
	};

	struct house_spawn
	{
		house_spawn()
		{
			loc.push_back(irr::core::vector3df(81,47,60));
			loc.push_back(irr::core::vector3df(119,47,-34));
			loc.push_back(irr::core::vector3df(113.0f, 47.0f, 25.0f));
		}
		irr::core::vector3df get_spawn(){return loc[rand()% 3 ];}

		std::vector<irr::core::vector3df> loc;
	};

	#define TYPE_CLUE "clue"
	#define TYPE_PLAYER "player"
    #define TYPE_WEAPON "weapon"

#endif 
//}