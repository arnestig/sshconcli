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

#define K_CTRL_A 1
#define K_CTRL_B 2
#define K_CTRL_C 3
#define K_CTRL_D 4
#define K_CTRL_E 5
#define K_CTRL_F 6
#define K_CTRL_G 7
#define K_CTRL_H 8
#define K_CTRL_I 9
#define K_CTRL_J 10
#define K_CTRL_K 11
#define K_CTRL_L 12
#define K_CTRL_M 13
#define K_CTRL_N 14
#define K_CTRL_O 15
#define K_CTRL_P 16
#define K_CTRL_Q 17
#define K_CTRL_R 18
#define K_CTRL_S 19
#define K_CTRL_T 20
#define K_CTRL_U 21
#define K_CTRL_V 22
#define K_CTRL_W 23
#define K_CTRL_X 24
#define K_CTRL_Y 25
#define K_CTRL_Z 26
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
    void loadConnections( bool byGroup = false );
    void runConnection();
    void handleInput( int c );
    bool handleNewConnectionInput( int c, bool mode );
    std::string getSearchText();
    void appendSearchText( char *add );
    void popSearchText();
    void addConnectionInteractive( bool editMode );

    unsigned int selectedPosition;
    unsigned int selectedGroup;
    std::string searchText;
    std::vector< Connection* > connections;
    std::vector< std::string > groups;
    std::vector< std::string > newConText;
    Connection *curConnection;
    WINDOW *helpWindow;
    WINDOW *searchWindow;
    WINDOW *connectionWindow;
    WINDOW *groupWindow;
    int newConLine;
};

#endif

