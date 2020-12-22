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

#include "resources.h"

Resources* Resources::instance = NULL;

Resources::Resources()
    :   sshDatabase( NULL ),
    	window( NULL )
{
}

Resources::~Resources()
{
    delete sshDatabase;
	delete window;
}

void Resources::DestroyInstance()
{
    delete instance;
    instance = NULL;
}

Resources* Resources::Instance()
{
    if ( instance == NULL ) {
        instance = new Resources();
    }
    return instance;
}

SSHDatabase* Resources::getSSHDatabase()
{
    if ( sshDatabase == NULL ) {
        sshDatabase = new SSHDatabase();
        sshDatabase->loadDatabase();
    }

    return sshDatabase;
}

Window* Resources::getWindow()
{
    if ( window == NULL ) {
        window = new Window();
    }
    return window;
}
