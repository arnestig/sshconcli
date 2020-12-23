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

#ifndef __SSH_DATABASE__H_
#define __SSH_DATABASE__H_

#include <string>
#include <map>
#include <vector>

class Connection
{
public:
    Connection( std::string name, std::string hostname, std::string group, std::string user, std::string password );
    ~Connection();

    std::string getName() const;
    std::string getHostname() const;
    std::string getGroup() const;
    std::string getUser() const;
    std::string getPassword() const;

    void setName( std::string name );
    void setHostname( std::string hostname );
    void setGroup( std::string group );
    void setUser( std::string user );
    void setPassword( std::string password );

    std::string getCommand();

private:
    std::string name;
    std::string hostname;
    std::string group;
    std::string user;
    std::string password;
};

class SSHDatabase
{
public:
    SSHDatabase();
    ~SSHDatabase();

    bool addConnectionInteractive();
    bool addConnection( std::string name, std::string hostname, std::string group, std::string user, std::string password );
    Connection* removeConnection( Connection *connection );
    void loadDatabase();
    std::vector< Connection* > getConnections( std::string searchText = "" );
    Connection* getConnectionByName( std::string searchText );
    std::vector< Connection* > getConnectionsByGroup( std::string group );
    std::vector< std::string > getGroups();
    Connection* getRunOnExit();
    void setRunOnExit(Connection *conn);

private:
    void appendNewConnectionText( char *add );
    void popNewConnectionText();
    bool handleConnectionInput( int c );
    void writeDatabase();
    Connection *runOnExit;

    std::string newConnectionText;
    std::vector< Connection* > connections;
};

#endif

