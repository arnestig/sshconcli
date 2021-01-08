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
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <fstream>


/** Connection sorter **/

bool sortConnections(Connection *l, Connection *r)
{
    return (l->getName()<r->getName());
}

std::string toUpperString( std::string s )
{
    std::stringstream ss;
    for( std::string::iterator it = s.begin(); it != s.end(); ++it ) {
        ss << char(toupper((*it)));
    }
    return ss.str();
}

/** END connection sorter **/

/** BEGIN CONNECTION **/
Connection::Connection( std::string name, std::string hostname, std::string group, std::string user, std::string password )
    :   name( name ),
        hostname( hostname),
        group( group ),
        user( user ),
        password( password )
{
}

Connection::Connection( Connection *copy )
    :   name( copy->name ),
        hostname( copy->hostname),
        group( copy->group ),
        user( copy->user ),
        password( copy->password )
{
}


Connection::~Connection()
{
}

std::string Connection::getName() const
{
    return name;
}

std::string Connection::getHostname() const
{
    return hostname;
}

std::string Connection::getGroup() const
{
    return group;
}

std::string Connection::getUser() const
{
    return user;
}

std::string Connection::getPassword() const
{
    return password;
}

void Connection::setName( std::string name )
{
    this->name = name;
}

void Connection::setHostname( std::string hostname )
{
    this->hostname = hostname;
}

void Connection::setGroup( std::string group )
{
    this->group = group;
}

void Connection::setUser( std::string user )
{
    this->user = user;
}

void Connection::setPassword( std::string password )
{
    this->password = password;
}

std::string Connection::getCommand()
{
    std::stringstream ss;
    if ( getPassword().empty() == false ) {
        ss << "sshpass -p " << getPassword() << " ";
    }
    ss << "ssh " << getUser() << "@" << getHostname();
    return ss.str();
}

/** END CONNECTION **/


/** BEGIN SSHDATABASE **/

SSHDatabase::SSHDatabase()
    : runOnExit( NULL )
{
}

SSHDatabase::~SSHDatabase()
{
    for ( std::vector< Connection* >::iterator it = connections.begin(); it != connections.end(); ++it ) {
        delete (*it);
    }
    connections.clear();
}

Connection* SSHDatabase::getRunOnExit()
{
    return runOnExit;
}

void SSHDatabase::setRunOnExit(Connection *conn)
{
    runOnExit = conn;
}

void SSHDatabase::loadDatabase()
{
    // delete our previous connection database
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
            char data[5][256];
            char *token;
            const char del[2] = { 0x1f, 0 };
            int i = 0;
            token = strtok((char *)line.c_str(), del);
            strcpy(data[i++],token);
            while ( token != NULL ) {
                token = strtok(NULL, del);
                if ( token != NULL ) {
                    strcpy(data[i++],token);
                }
            }
            if ( i == 5 ) {
                addConnection( data[0], data[1], data[2], data[3], data[4] );
            }
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
            ofs << (*it)->getName() << char(0x1f) << (*it)->getHostname() << char(0x1f) << (*it)->getGroup() << char(0x1f) << (*it)->getUser() << char(0x1f) << (*it)->getPassword() << std::endl;
        }
    }
    ofs.close();
}

bool SSHDatabase::addConnection( std::string name, std::string hostname, std::string group, std::string user, std::string password )
{
    connections.push_back( new Connection( name, hostname, group, user, password ) );
    writeDatabase();
    return true;
}
bool SSHDatabase::addConnection( Connection *copy )
{
    if ( copy != NULL ) {
        Connection *newCon = new Connection(copy);
        connections.push_back( newCon );
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
            if ( toUpperString((*it)->getName()).find(toUpperString(searchText)) != std::string::npos ) {
                retval.push_back( (*it) );
            } else if ( toUpperString((*it)->getHostname()).find(toUpperString(searchText)) != std::string::npos ) {
                retval.push_back( (*it) );
            } else if ( toUpperString((*it)->getGroup()).find(toUpperString(searchText)) != std::string::npos ) {
                retval.push_back( (*it) );
            } else if ( toUpperString((*it)->getUser()).find(toUpperString(searchText)) != std::string::npos ) {
                retval.push_back( (*it) );
            }
        }
    } else {
        retval = connections;
    }
    std::sort( retval.begin(), retval.end(), &sortConnections );
    return retval;
}

/** END SSHDATABASE **/
