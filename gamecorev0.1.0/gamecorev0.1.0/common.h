#ifndef COMMMON_H
#define COMMMON_H

#include<string>


//namespace common
//{

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

	enum char_models{DAN=0,MIKE,ZOMBIE};

    #define PATH_DAN "characters/stick_dan.ms3d"
    #define PATH_MIKE "characters/stick_mike.ms3d"
    #define PATH_ZOMB "characters/zombie.ms3d"


	#define TYPE_CLUE "clue"

#endif 
//}