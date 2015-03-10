/************************************************************************/
/*Config_Reader.cpp
  purpose: load in game configurations in order to change hardcoded variables
  by:Michael Carey
*/
/************************************************************************/


#include <iostream>
#include "Config_Reader.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


Config_Reader::Config_Reader(char* filename)
{
	//make an instance of irrlicht device to read the file
	device_ = createDevice(EDT_NULL,dimension2d<u32>(0,0),16,false);
	if(!device_)//if we cannot create a device
	{
		exit(-1);
	}
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
		if(read_string[0] == '#')//ignore comment lines
			read_string = "";
		else
			ini_vals.push_back(read_string);
	}
	ini_vals.push_back(read_string);//push last item on the vector

	file_reader_->drop();
	device_->drop();

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

//get the graphics driver from the configuration file
irr::video::E_DRIVER_TYPE Config_Reader::get_driver()
{
	irr::video::E_DRIVER_TYPE driver;

	switch(atoi(ini_vals[DRIVER_TYPE].c_str()))
	{
			case 1:
				driver =EDT_BURNINGSVIDEO;
				break;

			case 2:	 
				driver =EDT_DIRECT3D9;
				break;

			case 3:
				driver =EDT_OPENGL;
				break;

				break;
			default:
				std::cerr<<"No Driver in Config File!\n";
				exit(-1);
				break;
	}

	return driver;
}