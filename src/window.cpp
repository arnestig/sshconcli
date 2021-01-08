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
#include <sstream>

Window::Window()
    :	selectedPosition( 0 ),
      searchText( "" )
{
    initscr();
    noecho();
    start_color();
    newConText.resize(5);
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
    Resources::Instance()->getSSHDatabase()->setRunOnExit( curConnection );
    kill(getpid(), SIGINT);
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
    case K_CTRL_E:
        addConnectionInteractive( true );
        loadConnections(selectedGroup > 0);
        break;
    case K_CTRL_K:
        Resources::Instance()->getSSHDatabase()->addConnection( curConnection );
        loadConnections(selectedGroup > 0);
        break;
    case K_CTRL_N:
        addConnectionInteractive( false );
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

bool Window::handleNewConnectionInput( int c, bool mode )
{
    switch ( c ) {
    case KEY_UP:
        newConLine--;
        if ( newConLine == -1 ) {
            newConLine = 4;
        }
        return true;
        break;
    case 9:
    case KEY_DOWN:
        newConLine++;
        if ( newConLine == 5 ) {
            newConLine = 0;
        }
        return true;
        break;
    case KEY_ENTER:
    case K_ENTER:
        if ( mode == false ) { // add connection
            Resources::Instance()->getSSHDatabase()->addConnection(newConText[0], newConText[1], newConText[2], newConText[3], newConText[4] );
            for ( size_t i = 0; i < newConText.size(); i++ ) {
                newConText[i].clear();
            }
        } else { // edit connection
            curConnection->setName( newConText[ 0 ] );
            curConnection->setHostname( newConText[ 1 ] );
            curConnection->setGroup( newConText[ 2 ] );
            curConnection->setUser( newConText[ 3 ] );
            curConnection->setPassword( newConText[ 4 ] );

        }
        return false;
        break;
    case KEY_BACKSPACE:
    case K_BACKSPACE:
        if ( newConText[newConLine].size() > 0 ) {
            newConText[newConLine].erase(newConText[newConLine].size()-1);
        }
        return true;
        break;
    default:
        if ( c > 31 && c < 127 ) {
            newConText[newConLine].append( (char*)(&c) );
            return true;
        }
        break;
    }
    return false;
}

void Window::addConnectionInteractive( bool editMode )
{
    int height = 9;
    int width = 40;
    int y,x;
    std::string newConnectionText;
    getmaxyx( stdscr, y, x );
    WINDOW *newConnection = newwin( height,width,y/2-(height/2),x/2-(width/2) );
    keypad( newConnection, true );
    wclear(newConnection);
    box( newConnection, 0, 0 );
    wattron( newConnection, COLOR_PAIR(1) );
    if ( editMode == false ) {
        mvwprintw( newConnection, 0, 2, " Add new connection ");
        mvwprintw( newConnection, 2, 1, "Name: ");
        mvwprintw( newConnection, 3, 1, "Hostname: ");
        mvwprintw( newConnection, 4, 1, "Group: ");
        mvwprintw( newConnection, 5, 1, "Username: ");
        mvwprintw( newConnection, 6, 1, "Password: ");
        wmove( newConnection, 2, 12 );
    } else {
        newConText[0] = curConnection->getName();
        newConText[1] = curConnection->getHostname();
        newConText[2] = curConnection->getGroup();
        newConText[3] = curConnection->getUser();
        newConText[4] = curConnection->getPassword();
        mvwprintw( newConnection, 0, 2, " Edit connection ");
        mvwprintw( newConnection, 2, 1, "Name: " );
        mvwprintw( newConnection, 3, 1, "Hostname: " );
        mvwprintw( newConnection, 4, 1, "Group: " );
        mvwprintw( newConnection, 5, 1, "Username: " );
        mvwprintw( newConnection, 6, 1, "Password: " );
        wattroff( newConnection, COLOR_PAIR(1) );
        mvwprintw( newConnection, 2, 12, "%s", newConText[0].c_str());
        mvwprintw( newConnection, 3, 12, "%s", newConText[1].c_str());
        mvwprintw( newConnection, 4, 12, "%s", newConText[2].c_str());
        mvwprintw( newConnection, 5, 12, "%s", newConText[3].c_str());
        std::string pass;
        for ( size_t i = 0; i < newConText[4].length(); i++ ) {
            pass.append("*");
        }
        mvwprintw( newConnection, 6, 12, "%s", pass.c_str());
        wmove( newConnection, 2, 12 + newConText[0].length() );
    }
    wattroff( newConnection, COLOR_PAIR(1) );
    wnoutrefresh( newConnection );
    doupdate();
    newConLine = 0;
    int c = wgetch( newConnection );
    while ( handleNewConnectionInput( c, editMode ) == true ) {
        wclear( newConnection );
        box( newConnection, 0, 0 );
        wattron( newConnection, COLOR_PAIR(1) );
        if ( editMode == false ) {
            mvwprintw( newConnection, 0, 2, " Add new connection ");
        } else {
            mvwprintw( newConnection, 0, 2, " Edit connection ");
        }
        mvwprintw( newConnection, 2, 1, "Name: " );
        mvwprintw( newConnection, 3, 1, "Hostname: " );
        mvwprintw( newConnection, 4, 1, "Group: " );
        mvwprintw( newConnection, 5, 1, "Username: " );
        mvwprintw( newConnection, 6, 1, "Password: " );
        wattroff( newConnection, COLOR_PAIR(1) );
        mvwprintw( newConnection, 2, 12, "%s", newConText[0].c_str());
        mvwprintw( newConnection, 3, 12, "%s", newConText[1].c_str());
        mvwprintw( newConnection, 4, 12, "%s", newConText[2].c_str());
        mvwprintw( newConnection, 5, 12, "%s", newConText[3].c_str());
        std::string pass;
        for ( size_t i = 0; i < newConText[4].length(); i++ ) {
            pass.append("*");
        }
        mvwprintw( newConnection, 6, 12, "%s", pass.c_str());
        wmove( newConnection, 2+newConLine, 12 + newConText[newConLine].length());
        wnoutrefresh( newConnection );
        doupdate();
        c = wgetch( newConnection );
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
    // ^D - delete
    // ^N - new
    // ^K - duplicate
    // ^E - edit
    wattron( helpWindow, COLOR_PAIR(1) );
    mvwprintw( helpWindow, 1, 1, "^D delete | ^N new | ^K duplicate | ^E edit");
    wattroff( helpWindow, COLOR_PAIR(1) );

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

        mvwprintw( connectionWindow, 1 + connectionIndex, 1, "%s",(*it)->getName().c_str() );
        mvwprintw( connectionWindow, 1 + connectionIndex, 21, "%s",(*it)->getHostname().c_str() );
        mvwprintw( connectionWindow, 1 + connectionIndex, 41, "%s",(*it)->getGroup().c_str() );
        mvwprintw( connectionWindow, 1 + connectionIndex, 61, "%s",(*it)->getUser().c_str() );
        wattroff( connectionWindow, COLOR_PAIR(1) );
        connectionIndex++;
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

