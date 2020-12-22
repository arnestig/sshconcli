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

#include "sshdatabase.h"
#include "resources.h"
#include <stdlib.h>
#include <algorithm>
#include <fstream>


/** Connection sorter **/

bool sortConnections(Connection *l, Connection *r) {
    return (l->getName()<r->getName());
}

/** END command sorter **/

/** BEGIN COMMAND **/
Connection::Connection( std::string name )
    :   name( name )
{
}

Connection::~Connection()
{
}

std::string Connection::getName() const
{
    return name;
}

std::string Connection::getGroup() const
{
    return group;
}

void Connection::setName( std::string name )
{
    this->name = name;
}

void Connection::setGroup( std::string group )
{
    this->group = group;
}

/** END COMMAND **/


/** BEGIN COMMANDDATABASE **/

SSHDatabase::SSHDatabase()
{
}

SSHDatabase::~SSHDatabase()
{
    for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
        delete (*it);
    }
    connections.clear();
}

bool SSHDatabase::handleConnectionInput( int c )
{
	switch ( c ) {
		case KEY_ENTER:
		case K_ENTER:
			addConnection( newConnectionText );
			newConnectionText.clear();
			return false;
		break;
		case KEY_BACKSPACE:
		case K_BACKSPACE:
			popNewConnectionText();
			return true;
		break;
		default:
			if ( c > 31 && c < 127 ) {
				appendNewConnectionText( (char*)(&c) );
				return true;
			}
		break;
	}
    return false;
}

void SSHDatabase::appendNewConnectionText( char *add )
{
	newConnectionText.append( add );
}

void SSHDatabase::popNewConnectionText()
{
	if ( newConnectionText.length() > 0 ) {
		newConnectionText.erase( newConnectionText.end() - 1 );
	}
}

void SSHDatabase::loadDatabase()
{
    // delete our previous command database
    for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
        delete (*it);
    }
    connections.clear();

	char *home_path = getenv( "HOME" );
	char database_path[256];
	sprintf(database_path,"%s/.scc/connections",home_path);
	std::ifstream ifs;
	ifs.open( database_path, std::ifstream::in );
	if ( ifs.is_open() == true ) {
		std::string line;

		while (std::getline(ifs, line)) {
			addConnection( line );
		}
	}
	ifs.close();
}

void SSHDatabase::writeDatabase()
{
	char *home_path = getenv( "HOME" );
	char database_path[256];
	sprintf(database_path,"%s/.scc/connections",home_path);
	std::ofstream ofs;
	ofs.open( database_path, std::ifstream::out );
	if ( ofs.is_open() == true ) {
		for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
			ofs << (*it)->getName() << std::endl;
		}
	}
	ofs.close();
}

bool SSHDatabase::addConnection( std::string name )
{
    // first check if this command already exist in our database
    if ( getConnectionByName( name ) == NULL ) {
        connections.push_back( new Connection( name ) );
        writeDatabase();
        return true;
    }
    return false;
}

Connection* SSHDatabase::removeConnection( Connection *connection )
{
    Connection *newcom = NULL;
    if ( connection != NULL ) {
        for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ) {
            if ( (*it) == connection ) {
                it = connections.erase(it);
                if ( it != connections.end() ) {
                    newcom = (*it);
                }
                writeDatabase();
            } else {
                ++it;
            }
        }
    }
    return newcom;
}

bool SSHDatabase::addConnectionInteractive()
{
	int height = 3;
	int width = 100;
	int y,x;
	getmaxyx( stdscr, y, x );
	WINDOW *newConnection = newwin( height,width,(y-height)/2,(x-width)/2 );
	box( newConnection, 0, 0 );
	mvwprintw( newConnection, 1, 1, "Add new connection: %s",newConnectionText.c_str() );
	int c = wgetch( newConnection );
	while ( handleConnectionInput( c ) == true ) {
		wclear( newConnection );
		box( newConnection, 0, 0 );
		mvwprintw( newConnection, 1, 1, "Add new connection: %s",newConnectionText.c_str() );
		c = wgetch( newConnection );
	}

	if ( newConnectionText.empty() == false ) {
		return true;
	} else {
		return false;
	}
}

std::vector< std::string > SSHDatabase::getGroups()
{
    std::vector< std::string > groups;
    groups.push_back( "*" );
    for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it) {
        if ( std::find(groups.begin(), groups.end(), (*it)->getGroup() ) == groups.end() ) {
            groups.push_back( (*it)->getGroup() );
        }
    }
    std::sort( groups.begin(), groups.end() );
    return groups;
}

std::vector< Connection* > SSHDatabase::getConnectionsByGroup( std::string group )
{
	std::vector< Connection* > retval;
	if ( group != "*" ) {
		for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
			if ( (*it)->getGroup() == group ) {
				retval.push_back( (*it) );
			}
		}
	} else {
        retval = connections;
	}
    std::sort( retval.begin(), retval.end(), &sortConnections );
	return retval;

}

Connection* SSHDatabase::getConnectionByName( std::string searchText )
{
    Connection *ret = NULL;
    for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
        if ( (*it)->getName() == searchText ) {
            ret = (*it);
        }
    }
    return ret;
}

std::vector< Connection* > SSHDatabase::getConnections( std::string searchText )
{
	std::vector< Connection* > retval;
	if ( searchText.empty() == false ) {
		for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
			if ( (*it)->getName().find( searchText ) != std::string::npos ) {
				retval.push_back( (*it) );
			}
		}
	} else {
		retval = connections;
	}
    std::sort( retval.begin(), retval.end(), &sortConnections );
	return retval;
}

/** END COMMANDDATABASE **/
