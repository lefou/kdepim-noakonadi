/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */


#include "abstractdocument.h"
#include "documentvisitor.h"

namespace LibSyndication {

DocumentVisitor::~DocumentVisitor()
{
}
        
bool DocumentVisitor::visit(AbstractDocument* document)
{
    return document->accept(this);
}

bool DocumentVisitor::visitRSS2Document(LibSyndication::RSS2::Document*)
{
    return false;
}

bool DocumentVisitor::visitRDFDocument(LibSyndication::RDF::Document*)
{
    return false;
}
        
bool DocumentVisitor::visitAtomFeedDocument(LibSyndication::Atom::FeedDocument*)
{
    return false;
}
        
bool DocumentVisitor::visitAtomEntryDocument(LibSyndication::Atom::EntryDocument*)
{ 
    return false;
}

} // namespace LibSyndication
