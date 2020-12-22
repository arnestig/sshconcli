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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "window.h"
#include "resources.h"
#include <string.h>

Window::Window()
	:	selectedPosition( 0 ),
		searchText( "" )
{
	initscr();
	noecho();
	start_color();
}

Window::~Window()
{
	delwin( helpWindow );
	delwin( searchWindow );
	delwin( connectionWindow );
	delwin( groupWindow );
	refresh();
	endwin();
}

void Window::init()
{
	loadConnections();
	int y,x;
	getmaxyx( stdscr, y, x );

	// help window
	helpWindow = newwin( 3, x/2 - 2, 1, x/2+1 );

	// search window
	searchWindow = newwin( 3, x/2 - 1, 1, 1 );
	keypad( searchWindow, true );

	// connection window
	connectionWindow = newwin( y-7, x - 2, 7, 1 );

	// group window
	groupWindow = newwin( 3, x - 2, 4, 1 );

}

void Window::runConnection()
{
	if ( curConnection != NULL ) {
		std::string connectionName = curConnection->getName();
		for( size_t i = 0; i < connectionName.length(); i++ ) {
			ioctl(0,TIOCSTI, (char*)connectionName.c_str()+i);
		}
		kill(getpid(), SIGINT);
	}
}

std::string Window::getSearchText()
{
	return searchText;
}

void Window::loadConnections( bool byGroup )
{
	connections.clear();
    if ( byGroup == true ) {
        searchText.clear();
        connections = Resources::Instance()->getSSHDatabase()->getConnectionsByGroup( groups.at( selectedGroup ) );
    } else {
        connections = Resources::Instance()->getSSHDatabase()->getConnections( searchText );
    }

	groups = Resources::Instance()->getSSHDatabase()->getGroups();
	if ( connections.empty() == false ) {
        Connection *oldConnection = curConnection;
        // check if our old connection is in this list
        selectedPosition = 0;
		curConnection = connections[ 0 ];
        for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
            if ( oldConnection == *it ) {
                selectedPosition = it - connections.begin();
                curConnection = *it;
            }
        }
	} else {
		curConnection = NULL;
	}
}

void Window::appendSearchText( char *add )
{
	searchText.append( add );
}

void Window::popSearchText()
{
	if ( searchText.length() > 0 ) {
		searchText.erase( searchText.end() - 1 );
	}
}

void Window::handleInput( int c )
{
	switch ( c ) {
		case KEY_DOWN:
			if ( selectedPosition < connections.size() - 1 ) {
				selectedPosition++;
				if ( connections.size() > selectedPosition ) {
					curConnection = connections.at( selectedPosition );
				} else {
					curConnection = NULL;
				}
			}
		break;
		case K_CTRL_T:
			Resources::Instance()->getSSHDatabase()->addConnectionInteractive();
            loadConnections(selectedGroup > 0);
		break;
		case K_CTRL_D:
			curConnection = Resources::Instance()->getSSHDatabase()->removeConnection( curConnection );
            groups = Resources::Instance()->getSSHDatabase()->getGroups();
            if ( selectedGroup > groups.size()-1 ) {
                selectedGroup = groups.size()-1;
            }
            loadConnections(selectedGroup > 0);
            if ( selectedPosition == connections.size() && connections.size() > 0 ) {
                selectedPosition = connections.size()-1;
            }
		break;
		case KEY_ENTER:
		case K_ENTER:
			runConnection();
		break;
        case KEY_LEFT:
            if ( selectedGroup > 0 ) {
                selectedGroup--;
            }
            loadConnections(true);
        break;
        case KEY_RIGHT:
            if ( selectedGroup+1 < groups.size() ) {
                selectedGroup++;
            }
            loadConnections(true);
        break;
		case KEY_UP:
			if ( selectedPosition > 0 ) {
				selectedPosition--;
				if ( connections.size() > selectedPosition ) {
					curConnection = connections.at( selectedPosition );
				} else {
					curConnection = NULL;
				}
			}
		break;
		case KEY_BACKSPACE:
		case K_BACKSPACE:
			popSearchText();
			loadConnections();
		break;
		default:
			if ( c > 31 && c < 127 ) {
				appendSearchText( (char*)(&c) );
				loadConnections();
			}
		break;
	}
}

void Window::draw()
{
	wclear( searchWindow );
	wclear( helpWindow );
	wclear( connectionWindow );
	wclear( groupWindow );

    // make colors
	init_pair(1,COLOR_YELLOW, COLOR_BLACK);
	init_pair(2,COLOR_BLUE, COLOR_BLACK);

	// draw help
	mvwprintw( helpWindow, 1, 1, "Ctrl+T - %s", "Add new connection" );

    // draw groups
    size_t g = 0;
    int gpos = 1;
    for( std::vector< std::string >::iterator it = groups.begin(); it != groups.end(); ++it ) {
        if ( g++ == selectedGroup ) {
            wattron( groupWindow, COLOR_PAIR(2) );
        }
        mvwprintw( groupWindow, 1, gpos, "%s", (*it).c_str() );
        wattroff( groupWindow, COLOR_PAIR(2) );
        gpos += (*it).length()+1;
    }

	// draw connections
	unsigned int connectionIndex = 0;
	for( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
		// draw background if this is our selected connection
		if ( connectionIndex == selectedPosition ) {
            wattron( connectionWindow, COLOR_PAIR(1) );
        }

        mvwprintw( connectionWindow, 1 + connectionIndex++, 1, "%s",(*it)->getName().c_str() );
        wattroff( connectionWindow, COLOR_PAIR(1) );
	}

	// draw search box
	mvwprintw( searchWindow, 1, 1, "Search: %s", getSearchText().c_str() );

	box( searchWindow, 0, 0 );
	box( connectionWindow, 0, 0 );
	box( helpWindow, 0, 0 );
	box( groupWindow, 0, 0 );
	wnoutrefresh( connectionWindow );
	wnoutrefresh( helpWindow );
	wnoutrefresh( groupWindow );
	wnoutrefresh( searchWindow );
	doupdate();
	int c = wgetch(searchWindow);
	handleInput( c );
}

