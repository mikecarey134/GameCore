/************************************************************************/
/* ChatQueue.h
	purpose: to render player chat and events
	By: Michael Carey
*/
/************************************************************************/

#ifndef __ChatQueue_H__
#define __ChatQueue_H__

#define ChatQueue_STRINGLENGTH 240

#include <irrlicht.h>
#include <list>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

using namespace std;

class ChatQueue {
public:
	//Constructor
	ChatQueue(IrrlichtDevice * device,const core::rect<s32> drawArea,
		float textLife = 5000, bool setFade = false, float fadeTime = 1000.00, bool setShadows = true
		, bool debug = true, bool isDrawn = true  );
	~ChatQueue();
	//Main Functions
	void addMessage(const wchar_t* text,SColor chatColor = SColor(255,255,255,255)
		,SColor chatColorShadow = SColor(255,0,0,0));

	void addMessage(wchar_t* text,SColor chatColor = SColor(255,255,255,255)
		,SColor chatColorShadow = SColor(255,0,0,0));

	void addMessage(core::stringw text,SColor chatColor = SColor(255,255,255,255)
		,SColor chatColorShadow = SColor(255,0,0,0));

	void draw();

	void setFade(bool setFade = true);
	void setRect(const core::rect<s32> drawArea);
	void setFadeTime(float fadeTime = 1000.00);
	void setFont(irr::gui::IGUIFont* font);
	void setMaxLines(unsigned short maxLines);
	void setShadows(bool setShadows = true);
	void setVisible(bool setVisible = true);
	void setLife(float setLife = 1000);
	void setDebug(bool setDebug = true);
	

protected: 
	//Helper functions
	void ChatQueue_calculateRect(const core::rect<s32> drawArea);
	void ChatQueue_calculateFontHeight(irr::gui::IGUIFont* font);
	void ChatQueue_calculateMaxLines();

	struct ChatQueueMessage{
		wchar_t message[ChatQueue_STRINGLENGTH];
		SColor chatColor;
		SColor chatColorShadow;
		SColor chatColorFade;
		SColor chatColorShadowFade;
		float created;
		float fade;
		bool ChatQueue_memoryManaged;
	};  

	//Rect replacment
	unsigned int m_xWidth;
	unsigned int m_yHeight;
	unsigned int m_y;
	unsigned int m_x; 
	//Font height
	unsigned short m_fontHeight;

	IrrlichtDevice* ChatQueue_device;
	bool ChatQueue_fade;
	bool ChatQueue_shadows;
	bool ChatQueue_visible;
	bool ChatQueue_debug;
	float ChatQueue_fadeTime;
	float ChatQueue_life;
	unsigned short ChatQueue_maxLines;
	std::list<ChatQueueMessage> ChatQueue_list;
	IGUIFont* ChatQueue_font;
};

#endif //ChatQueue.h

