/*
 * Copyright (c) 2006-2016, Guillaume Gimenez <guillaume@blackmilk.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of G.Gimenez nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL G.Gimenez BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     * Guillaume Gimenez <guillaume@blackmilk.fr>
 *
 */
#include "treephotoitem.h"
#include "photo.h"
#include "console.h"

TreePhotoItem::TreePhotoItem(const Photo &photo,
                             PhotoType type,
                             QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent, Type),
    m_photo(photo),
    m_type(type)
{
    setText(0, photo.getTag(TAG_NAME));
    setToolTip(0, photo.getIdentity());
    if ( !photo.isComplete() ) {
        dflCritical(TreePhotoItem::tr("TreePhotoItem: photo is not complete"));
    }
    if ( !m_photo.isComplete() ) {
        dflCritical(TreePhotoItem::tr("TreePhotoItem: m_photo is not complete"));
    }
    setType(type);
}

const Photo &TreePhotoItem::photo() const
{
    return m_photo;
}

Photo &TreePhotoItem::photo()
{
    return m_photo;
}

bool TreePhotoItem::isInput() const
{
    return m_type != Output;
}

void TreePhotoItem::setType(TreePhotoItem::PhotoType type)
{
    if ( type == InputError ) {
        setForeground(0, Qt::red);
    }
    else if ( type == InputDisabled ) {
        setForeground(0, Qt::gray);
    }
    else if ( type == InputReference ) {
        setForeground(0, Qt::green);
    }
    else {
        setForeground(0, Qt::black);
    }

}
