#pragma once
#include<string>
#include<Windows.h>

class Win_Launcher
{
public:

	Win_Launcher(){}
	~Win_Launcher(void){}
	//run the game from the modified batch file
	void run_game(){ system("start /B Data\\rungame.bat");}
	//run the server from the modified batch file
	void run_server(){ system("start /B Data\\runserver.bat");}
	//open up the configuration file for editing
	void open_config(){ShellExecute(NULL,L"open",L"Data\\game\\config\\configure.txt",NULL,NULL,SW_SHOWDEFAULT);}

};
