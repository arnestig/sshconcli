/**
    Copyright (C) 2020-2021 sshconcli

    Written by Tobias Eliasson <arnestig@gmail.com>.

    This file is part of sshconcli <https://github.com/arnestig/sshconcli>.

    sshconcli is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sshconcli is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with sshconcli.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef __WINDOW__H_
#define __WINDOW__H_

#define Y_OFFSET_SEARCH 0
#define Y_OFFSET_HELP 1
#define Y_OFFSET_CONNECTIONS 2

#define K_CTRL_D 4
#define K_CTRL_T 20
#define K_ENTER 10
#define K_BACKSPACE 127

#include <string>
#include <vector>
#include "sshdatabase.h"
#include <ncursesw/curses.h>

class Window
{
    public:
        Window();
        ~Window();

		void init();
		void draw();

    private:
		unsigned int selectedPosition;
		unsigned int selectedGroup;
		std::string searchText;

		void loadConnections( bool byGroup = false );
		void runConnection();
		void handleInput( int c );
		std::string getSearchText();
		void appendSearchText( char *add );
		void popSearchText();
		std::vector< Connection* > connections;
		std::vector< std::string > groups;
		Connection *curConnection;
		WINDOW *helpWindow;
		WINDOW *searchWindow;
		WINDOW *connectionWindow;
		WINDOW *groupWindow;
};

#endif

