
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
	enum c_val{ ONLINE = 0,REMOTE_PORT, PORT,IP ,PLAYER_NAME};
	
	Config_Reader(irr::IrrlichtDevice* device, char* filename);
	~Config_Reader(void);
	bool readLine(irr::io::IReadFile* f,std::string& str );

	//get all the attributes from our config file
	bool         get_online      () {return ini_vals[ONLINE][0] == '1';}
	int          get_port        () {return atoi(ini_vals[PORT].c_str());}
	int          get_rem_port    () {return atoi(ini_vals[REMOTE_PORT].c_str());}
	char*  get_ip                () {return const_cast<char*>(ini_vals[IP].c_str());}
	std::string  get_player_name () {return ini_vals[PLAYER_NAME];}

private:
	irr::IrrlichtDevice* device_;
	irr::io::IFileSystem* fs_;
	irr::io::IReadFile*   file_reader_;
	char* filename_;                    //our config file
	std::vector<std::string > ini_vals; //config file data
};
