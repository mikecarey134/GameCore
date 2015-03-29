
/************************************************************************/
/*Config_Reader.h
  purpose: load in game configurations in order to change hardcoded variables
  by:Michael Carey
*/
/************************************************************************/


#pragma once
#include<irrlicht.h>
#include<cstring>
#include<vector>
#include<string>

//10 is the default data size for our config file for now
#define DEFAULT_SIZE 10

class Config_Reader
{
public:
	//locations for data in our vector
	enum config_vals{ DRIVER_TYPE=0,DEBUG,STENCIL_BUFF, 
		VSINC, FULL_SCREEN, ONLINE,REMOTE_PORT, PORT,IP ,PLAYER_MODEL,PLAYER_NAME};
	
	Config_Reader(char* filename);
	~Config_Reader(void);
	bool readLine(irr::io::IReadFile* f,std::string& str );

	//get all the attributes from our config file
	irr::video::E_DRIVER_TYPE get_driver       ();
	bool                      get_debug       () {return ini_vals[DEBUG][0] == '1';}
	bool                      get_s_buffer    () {return ini_vals[STENCIL_BUFF][0] == '1';}
	bool                      get_vsinc       () {return ini_vals[VSINC][0] == '1';}
	bool                      get_full_screen () {return ini_vals[FULL_SCREEN][0] == '1';}
	bool                      get_online      () {return ini_vals[ONLINE][0] == '1';}
	int                       get_port        () {return atoi(ini_vals[PORT].c_str());}
	int                       get_rem_port    () {return atoi(ini_vals[REMOTE_PORT].c_str());}
	char*                     get_ip          () {return const_cast<char*>(ini_vals[IP].c_str());}
	std::string               get_player_name () {return ini_vals[PLAYER_NAME];}
	int                       get_model       () {return atoi(ini_vals[PLAYER_MODEL].c_str());} 

private:
	irr::IrrlichtDevice* device_;  
	irr::io::IFileSystem* fs_;
	irr::io::IReadFile*   file_reader_;
	char* filename_;                    //our config file
	std::vector<std::string > ini_vals; //config file data
};
