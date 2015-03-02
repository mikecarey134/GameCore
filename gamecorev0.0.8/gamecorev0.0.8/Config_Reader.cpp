/************************************************************************/
/*Config_Reader.cpp
  purpose: load in game configurations in order to change hardcoded variables
  by:Michael Carey

*/
/************************************************************************/



#include "Config_Reader.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


Config_Reader::Config_Reader(irr::IrrlichtDevice* device, char* filename):device_(device)
{
	ini_vals.reserve(DEFAULT_SIZE);//reserve the default amount so we do not have to call doubleCap()
	
	//get our filename
	filename_ = new char[20];
	filename_ = strcpy(filename_, filename);
	fs_ = device_->getFileSystem();

	if(!fs_->existFile(filename_))//if the file exsists
	{
		exit(-1);
	}

	file_reader_ = fs_->createAndOpenFile(filename_);
	if(!file_reader_)
	{
		exit(-1);
	}

	std::string read_string;
	
	while(readLine(file_reader_, read_string))
	{
		ini_vals.push_back(read_string);
	}
	ini_vals.push_back(read_string);//push last item on the vector

	printf("Configuration-File Read Success!\n");

	


}
bool Config_Reader::readLine(IReadFile* f, std::string& str)
{
	char ch;
	str ="";
	
	while(0 != f->read(&ch, 1))
	{
		if(ch == '\n' )//if its the return char
			return true;
		
		else
			str += ch;
	}

	return false;

}
Config_Reader::~Config_Reader(void)
{	//deallocate dyanmic memory
	//delete filename_;
}
