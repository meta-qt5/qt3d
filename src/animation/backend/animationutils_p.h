/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QT3DANIMATION_ANIMATION_ANIMATIONUTILS_P_H
#define QT3DANIMATION_ANIMATION_ANIMATIONUTILS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <Qt3DAnimation/private/qt3danimation_global_p.h>
#include <Qt3DAnimation/private/clock_p.h>
#include <Qt3DAnimation/qanimationcallback.h>
#include <Qt3DCore/qnodeid.h>
#include <Qt3DCore/qscenechange.h>

#include <QtCore/qbitarray.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
class QAnimationCallback;
namespace Animation {

struct Channel;
class BlendedClipAnimator;
class Handler;
class AnimationClip;
class ChannelMapper;
class ChannelMapping;

typedef QVector<int> ComponentIndices;

enum JointTransformComponent {
    NoTransformComponent = 0,
    Scale,
    Rotation,
    Translation
};

struct MappingData
{
    Qt3DCore::QNodeId targetId;
    Skeleton *skeleton = nullptr;
    int jointIndex = -1;
    JointTransformComponent jointTransformComponent = NoTransformComponent;
    const char *propertyName;
    QAnimationCallback *callback = nullptr;
    QAnimationCallback::Flags callbackFlags;
    int type;
    ComponentIndices channelIndices;
};

#ifndef QT_NO_DEBUG_STREAM
inline QDebug operator<<(QDebug dbg, const MappingData &mapping)
{
    QDebugStateSaver saver(dbg);
    dbg << "targetId =" << mapping.targetId << endl
        << "jointIndex =" << mapping.jointIndex << endl
        << "jointTransformComponent: " << mapping.jointTransformComponent << endl
        << "propertyName:" << mapping.propertyName << endl
        << "channelIndices:" << mapping.channelIndices;
    return dbg;
}
#endif

struct AnimatorEvaluationData
{
    double elapsedTime;
    double currentTime;
    int loopCount;
    int currentLoop;
    double playbackRate;
};

struct ClipEvaluationData
{
    int currentLoop;
    double localTime;
    bool isFinalFrame;
};

typedef QVector<float> ClipResults;

struct ChannelNameAndType
{
    QString jointName;
    QString name;
    int type;
    int jointIndex;
    Qt3DCore::QNodeId mappingId;
    JointTransformComponent jointTransformComponent;
    float pad; // Unused

    static const int invalidIndex = -1;

    ChannelNameAndType()
        : jointName()
        , name()
        , type(-1)
        , jointIndex(-1)
        , mappingId()
        , jointTransformComponent(NoTransformComponent)
    {}

    ChannelNameAndType(const QString &_name,
                       int _type,
                       Qt3DCore::QNodeId _mappingId = Qt3DCore::QNodeId(),
                       int _jointIndex = invalidIndex)
        : jointName()
        , name(_name)
        , type(_type)
        , jointIndex(_jointIndex)
        , mappingId(_mappingId)
        , jointTransformComponent(NoTransformComponent)
    {}

    ChannelNameAndType(const QString &_name,
                       int _type,
                       JointTransformComponent _jointTransformComponent)
        : jointName()
        , name(_name)
        , type(_type)
        , jointIndex(invalidIndex)
        , mappingId()
        , jointTransformComponent(_jointTransformComponent)
    {}

    bool operator==(const ChannelNameAndType &rhs) const
    {
        return name == rhs.name
            && type == rhs.type
            && jointIndex == rhs.jointIndex
            && mappingId == rhs.mappingId
            && jointTransformComponent == rhs.jointTransformComponent;
    }
};

#ifndef QT_NO_DEBUG_STREAM
inline QDebug operator<<(QDebug dbg, const ChannelNameAndType &nameAndType)
{
    QDebugStateSaver saver(dbg);
    dbg << "name =" << nameAndType.name
        << "type =" << nameAndType.type
        << "mappingId =" << nameAndType.mappingId
        << "jointIndex =" << nameAndType.jointIndex
        << "jointName =" << nameAndType.jointName
        << "jointTransformComponent =" << nameAndType.jointTransformComponent;
    return dbg;
}
#endif

struct ComponentValue
{
    int componentIndex;
    float value;
};
QT3D_DECLARE_TYPEINFO_2(Qt3DAnimation, Animation, ComponentValue, Q_PRIMITIVE_TYPE)

struct ClipFormat
{
    // TODO: Remove the mask and store both the sourceClipIndices and
    // formattedComponentIndices in flat vectors. This will require a
    // way to look up the offset and number of elements for each channel.
    ComponentIndices sourceClipIndices;
    QVector<QBitArray> sourceClipMask;
    QVector<ComponentIndices> formattedComponentIndices;
    QVector<ChannelNameAndType> namesAndTypes;
    QVector<ComponentValue> defaultComponentValues;
};

#ifndef QT_NO_DEBUG_STREAM
inline QDebug operator<<(QDebug dbg, const ClipFormat &format)
{
    QDebugStateSaver saver(dbg);
    int sourceIndex = 0;
    for (int i = 0; i < format.namesAndTypes.size(); ++i) {
        dbg << i
            << format.namesAndTypes[i].jointIndex
            << format.namesAndTypes[i].jointName
            << format.namesAndTypes[i].name
            << format.namesAndTypes[i].type
            << "formatted results dst indices =" << format.formattedComponentIndices[i];
        const int componentCount = format.formattedComponentIndices[i].size();

        dbg << "clip src indices =";
        for (int j = sourceIndex; j < sourceIndex + componentCount; ++j)
            dbg << format.sourceClipIndices[j] << "";

        dbg << "src clip mask =" << format.sourceClipMask[i];
        dbg << endl;
        sourceIndex += componentCount;
    }
    return dbg;
}
#endif

struct AnimationCallbackAndValue
{
    QAnimationCallback *callback;
    QAnimationCallback::Flags flags;
    QVariant value;
};

template<typename Animator>
AnimatorEvaluationData evaluationDataForAnimator(Animator animator, Clock* clock, qint64 nsSincePreviousFrame)
{
    AnimatorEvaluationData data;
    data.loopCount = animator->loops();
    data.currentLoop = animator->currentLoop();
    data.playbackRate = clock != nullptr ? clock->playbackRate() : 1.0;
    // Convert global time from nsec to sec
    data.elapsedTime = double(nsSincePreviousFrame) / 1.0e9;
    data.currentTime = animator->lastLocalTime();
    return data;
}

inline bool isFinalFrame(double localTime,
                         double duration,
                         int currentLoop,
                         int loopCount)
{
    return (localTime >= duration &&
            loopCount != 0 &&
            currentLoop >= loopCount - 1);
}

Q_AUTOTEST_EXPORT
int componentsForType(int type);

Q_AUTOTEST_EXPORT
ClipEvaluationData evaluationDataForClip(AnimationClip *clip,
                                         const AnimatorEvaluationData &animatorData);

Q_AUTOTEST_EXPORT
ComponentIndices channelComponentsToIndices(const Channel &channel,
                                            int dataType,
                                            int offset = 0);

Q_AUTOTEST_EXPORT
ComponentIndices channelComponentsToIndicesHelper(const Channel &channelGroup,
                                                  int dataType,
                                                  int offset,
                                                  const QVector<char> &suffixes);

Q_AUTOTEST_EXPORT
ClipResults evaluateClipAtLocalTime(AnimationClip *clip,
                                    float localTime);

Q_AUTOTEST_EXPORT
ClipResults evaluateClipAtPhase(AnimationClip *clip,
                                float phase);

Q_AUTOTEST_EXPORT
QVector<Qt3DCore::QSceneChangePtr> preparePropertyChanges(Qt3DCore::QNodeId animatorId,
                                                          const QVector<MappingData> &mappingDataVec,
                                                          const QVector<float> &channelResults,
                                                          bool finalFrame);

Q_AUTOTEST_EXPORT
QVector<AnimationCallbackAndValue> prepareCallbacks(const QVector<MappingData> &mappingDataVec,
                                                    const QVector<float> &channelResults);

Q_AUTOTEST_EXPORT
QVector<MappingData> buildPropertyMappings(const QVector<ChannelMapping *> &channelMappings,
                                           const QVector<ChannelNameAndType> &channelNamesAndTypes,
                                           const QVector<ComponentIndices> &channelComponentIndices,
                                           const QVector<QBitArray> &sourceClipMask);

Q_AUTOTEST_EXPORT
QVector<ChannelNameAndType> buildRequiredChannelsAndTypes(Handler *handler,
                                                          const ChannelMapper *mapper);

Q_AUTOTEST_EXPORT
QVector<ComponentIndices> assignChannelComponentIndices(const QVector<ChannelNameAndType> &namesAndTypes);

Q_AUTOTEST_EXPORT
double localTimeFromElapsedTime(double t_current_local, double t_elapsed_global,
                                double playbackRate, double duration,
                                int loopCount, int &currentLoop);

Q_AUTOTEST_EXPORT
double phaseFromElapsedTime(double t_current_local, double t_elapsed_global,
                           double playbackRate, double duration,
                           int loopCount, int &currentLoop);

Q_AUTOTEST_EXPORT
QVector<Qt3DCore::QNodeId> gatherValueNodesToEvaluate(Handler *handler,
                                                      Qt3DCore::QNodeId blendTreeRootId);

Q_AUTOTEST_EXPORT
ClipFormat generateClipFormatIndices(const QVector<ChannelNameAndType> &targetChannels,
                                     const QVector<ComponentIndices> &targetIndices,
                                     const AnimationClip *clip);

Q_AUTOTEST_EXPORT
ClipResults formatClipResults(const ClipResults &rawClipResults,
                              const ComponentIndices &format);

Q_AUTOTEST_EXPORT
ClipResults evaluateBlendTree(Handler *handler,
                              BlendedClipAnimator *animator,
                              Qt3DCore::QNodeId blendNodeId);

Q_AUTOTEST_EXPORT
QVector<float> defaultValueForChannel(Handler *handler, const ChannelNameAndType &channelDescription);

} // Animation
} // Qt3DAnimation

QT_END_NAMESPACE


#endif // QT3DANIMATION_ANIMATION_ANIMATIONUTILS_P_H
