/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QT3D_RENDER_RENDERVIEWJOB_H
#define QT3D_RENDER_RENDERVIEWJOB_H

#include <Qt3DCore/qjob.h>

QT_BEGIN_NAMESPACE

namespace Qt3D {

class Renderer;

namespace Render {

class FrameGraphNode;

class RenderViewJob : public QJob
{
public:
    RenderViewJob()
        : m_fgLeaf(0)
        , m_index(0)
    {}

    inline void setRenderer(Renderer *renderer) { m_renderer = renderer; }

    inline void setFrameGraphLeafNode(FrameGraphNode *fgLeaf)
    {
        m_fgLeaf = fgLeaf;
    }

    // Sets the position in the queue of RenderViews that the
    // RenderView generated by this job should be inserted. This is
    // used to ensure that for example a RenderView for creating
    // a shadow map texture is submitted before the RenderView that
    // contains commands making use of the shadow map
    inline void setSubmitOrderIndex(int index) { m_index = index; }

protected:
    void run() Q_DECL_OVERRIDE;

private:
    Renderer *m_renderer;
    FrameGraphNode *m_fgLeaf;
    int m_index;
};

typedef QSharedPointer<RenderViewJob> RenderViewJobPtr;

} // namespace Render
} // namespace Qt3D

QT_END_NAMESPACE

#endif // QT3D_RENDER_RENDERVIEWJOB_H
