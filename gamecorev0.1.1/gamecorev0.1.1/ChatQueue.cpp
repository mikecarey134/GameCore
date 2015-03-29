/************************************************************************/
/* ChatQueue.cpp
	purpose: to render player chat and events
	By: Michael Carey
*/
/************************************************************************/


#include "ChatQueue.h"

ChatQueue::ChatQueue(IrrlichtDevice * device,
					 const core::rect<s32> drawArea,float textLife,
					 bool setFade, float fadeTime, bool setShadows, bool debug, bool isDrawn):
					ChatQueue_device(device),ChatQueue_life(textLife)
					,ChatQueue_fade(setFade),ChatQueue_fadeTime(fadeTime),ChatQueue_debug(debug),ChatQueue_visible(isDrawn)
{
	//init the chat queue
	m_fontHeight = 0;
	ChatQueue_calculateRect(drawArea);
	ChatQueue_font = device->getGUIEnvironment()->getBuiltInFont();
	setFont(ChatQueue_font);
}

ChatQueue::~ChatQueue()
{
	std::list<ChatQueueMessage>::iterator iter;
	for (iter=ChatQueue_list.begin(); iter != ChatQueue_list.end(); iter++)
	{     
		if(iter->ChatQueue_memoryManaged == true)
			delete iter->message;
	}
	ChatQueue_list.erase(ChatQueue_list.begin(),ChatQueue_list.end());
}

void ChatQueue::addMessage(wchar_t* text, SColor chatColor, SColor chatColorShadow)
{
	addMessage((const wchar_t*) text,chatColor,chatColorShadow);
}

void ChatQueue::addMessage(core::stringw text, SColor chatColor, SColor chatColorShadow)
{
	addMessage((const wchar_t*) text.c_str(),chatColor,chatColorShadow);
}

void ChatQueue::addMessage(const wchar_t* text, SColor chatColor, SColor chatColorShadow)
{
	ChatQueueMessage lmsg;
	wcscpy(lmsg.message,text);
	lmsg.chatColor = chatColor;
	lmsg.chatColorShadow = chatColorShadow;
	lmsg.fade = 0;
	lmsg.ChatQueue_memoryManaged = false;
	lmsg.created = ChatQueue_device->getTimer()->getTime();
	ChatQueue_list.push_front(lmsg);
}

void ChatQueue::draw()
{
	//Debug
	if(ChatQueue_debug && ChatQueue_visible)
		ChatQueue_device->getVideoDriver()->draw2DRectangle(video::SColor(100,0,0,0),
		core::rect<s32>(m_x, m_y - m_yHeight,m_x + m_xWidth,m_y));    
	//Main Update/Draw
	std::list<ChatQueueMessage>::iterator iter;
	unsigned short count = 0;
	unsigned int m_y_tmp = m_y;
	u32 l_time = ChatQueue_device->getTimer()->getTime();
	
	
	
		for (iter=ChatQueue_list.begin(); iter != ChatQueue_list.end(); iter++)
		{ 
			//Max lines means no fade, special case delete
			if(count > ChatQueue_maxLines){
				if(iter->ChatQueue_memoryManaged == true)
					delete iter->message;
				ChatQueue_list.erase(iter,ChatQueue_list.end());
				return;
			}
			//Update
			if((iter->created + ChatQueue_life) < l_time){
				if(ChatQueue_fade)
				{
					if(iter->fade == 0)
					{
						iter->fade = l_time;
						iter->chatColorFade = iter->chatColor;
						iter->chatColorShadowFade = iter->chatColorShadow;
					}
					//Fade code
					f32 l_alpha = (   (f32)(l_time - iter->fade)/(f32)ChatQueue_fadeTime );
					iter->chatColor = iter->chatColorFade.getInterpolated(SColor(0,0,0,0),1-l_alpha);
					iter->chatColorShadow = iter->chatColorShadowFade.getInterpolated(SColor(0,0,0,0),1-l_alpha);
				}
				if( ((l_time - iter->fade) >= ChatQueue_fadeTime) || !ChatQueue_fade )
				{
					if(iter->ChatQueue_memoryManaged == true)
						delete iter->message;
					ChatQueue_list.erase(iter,ChatQueue_list.end());
					return;
				}
			} 
			//Draw
			if (ChatQueue_visible)
			{
				if (ChatQueue_font)
				{
					if (ChatQueue_shadows)
						ChatQueue_font->draw(iter->message,
						core::rect<s32>(m_x + 1,m_y_tmp - m_fontHeight+ 1,m_x+m_xWidth + 1,m_y_tmp + 1),
						iter->chatColorShadow);        

					ChatQueue_font->draw(iter->message,
						core::rect<s32>(m_x,m_y_tmp - m_fontHeight,m_x+m_xWidth,m_y_tmp),
						iter->chatColor);    
					m_y_tmp -= m_fontHeight;
				}
			}
			count++;
		}

	
}

//!-- Start Helpers

void ChatQueue::ChatQueue_calculateRect(const core::rect<s32> drawArea)
{
	m_xWidth  = drawArea.getWidth();
	m_yHeight = drawArea.getHeight();
	position2d<s32> l_positionTmp = drawArea.LowerRightCorner;
	m_y = l_positionTmp.Y;
	l_positionTmp = drawArea.UpperLeftCorner;
	m_x = l_positionTmp.X;  
}

void ChatQueue::ChatQueue_calculateFontHeight(irr::gui::IGUIFont* font)
{
	dimension2d<u32> l_fontHeight = ChatQueue_font->getDimension(L"|");
	m_fontHeight = l_fontHeight.Height;   
}

void ChatQueue::ChatQueue_calculateMaxLines()
{
	ChatQueue_maxLines = ((m_yHeight - m_fontHeight) / m_fontHeight);     
}

//!-- End Helpers

void ChatQueue::setFade(bool setFade)
{
	ChatQueue_fade = setFade;
}

void ChatQueue::setRect(const core::rect<s32> drawArea)
{
	ChatQueue_calculateRect(drawArea);
}

void ChatQueue::setFadeTime(float fadeTime)
{
	ChatQueue_fadeTime = fadeTime;
}

void ChatQueue::setFont(irr::gui::IGUIFont* font)
{    
	ChatQueue_font = font;
	ChatQueue_calculateFontHeight(font);
	ChatQueue_calculateMaxLines();
}

void ChatQueue::setLife(float setLife)
{
	ChatQueue_life = setLife;
}

void ChatQueue::setShadows(bool setShadows)
{
	ChatQueue_shadows = setShadows;
}

void ChatQueue::setVisible(bool setVisible)
{
	ChatQueue_visible = setVisible;
}

void ChatQueue::setDebug(bool setDebug)
{
	ChatQueue_debug = setDebug;
}