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

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "sshdatabase.h"
#include "window.h"

class Resources
{
public:
    static Resources* Instance();
    static void DestroyInstance();

    SSHDatabase* getSSHDatabase();
    Window* getWindow();

private:
    static Resources* instance;
    Resources();
    ~Resources();
    Resources( Resources const& ) {};

    SSHDatabase *sshDatabase;
    Window *window;
};

#endif
