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
        Connection( std::string name );
        ~Connection();

		std::string getName() const;
        std::string getGroup() const;
		void setName( std::string name );
		void setGroup( std::string group );

    private:
        std::string name;
        std::string group;
};

class SSHDatabase
{
    public:
        SSHDatabase();
        ~SSHDatabase();

		bool addConnectionInteractive();
		bool addConnection( std::string name );
        Connection* removeConnection( Connection *connection );
        void loadDatabase();
		std::vector< Connection* > getConnections( std::string searchText = "" );
        Connection* getConnectionByName( std::string searchText );
		std::vector< Connection* > getConnectionsByGroup( std::string group );
        std::vector< std::string > getGroups();

    private:
		void appendNewConnectionText( char *add );
		void popNewConnectionText();
		bool handleConnectionInput( int c );
		void writeDatabase();

		std::string newConnectionText;
        std::vector< Connection* > connections;
};

#endif

