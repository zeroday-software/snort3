/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2002-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
// cv_var.cc author Josh Rosenbaum <jorosenba@cisco.com>

#include "data/cv_var.h"


#if 0
static inline bool var_exists(std::vector<std::string> vec, std::string name)
{
    for( auto str : vec)
        if(name.compare(str))
            return false;

    return true;
}
#endif

Variable::Variable(std::string name, int depth)
{
    this->name = name;
    this->count = name.size();
    this->depth = depth;
}

Variable::Variable(std::string name)
{
    this->name = name;
    this->count = name.size();
    this->depth = 0;
}

Variable::~Variable(){};

// does this need a new variable?
bool Variable::add_value(std::string elem)
{
    std::string s(elem);

    if(s.front() == '$')
    {
        s.erase(s.begin());
        vars.push_back(s);
    }
    else
    {
        strs.push_back(s);
    }

    count += s.size();
    return true;
}

std::ostream& operator<<( std::ostream& out, const Variable &var)
{
    int length = 0;
    std::string whitespace;

    for(int i = 0; i < var.depth; i++)
        whitespace += "    ";

    out << whitespace << var.name << " = ";

    for(auto v : var.vars)
    {
        if ( 0 < length && length + v.size() > var.max_line_length )
            out << std::endl << whitespace << "    ";

        length += v.size();
        out << " " << v << " ..";
    }

    if (var.strs.size() == 0)
        out << " ''";

    else if(var.count < var.max_line_length || var.strs.size() == 1)
    {
        out << "'";

        for (auto s : var.strs)
        {
            if ( 0 < length && length + s.size() > var.max_line_length )
                out << std::endl << whitespace << "    ";

            length += s.size();
            out << " " << s;
        }
        out << "'";
    }
    else
    {
        out << std::endl <<  whitespace << "[[" << std::endl;
        out << whitespace << "    ";
        length = 4 + whitespace.size();

        for (auto s : var.strs)
        {
            if ( 0 < length && length + s.size() > var.max_line_length )
            {
                out << std::endl << whitespace << "    ";
                length = 4 + whitespace.size();
            }

            length += s.size();
            out << " " << s;
        }

        out << std::endl << whitespace << "]]";
    }

    return out;
}
