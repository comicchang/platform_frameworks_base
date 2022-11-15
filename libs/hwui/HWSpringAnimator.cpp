/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "HWSpringAnimator.h"

#include <cutils/log.h>
#include <gui/TraceUtils.h>

#include <algorithm>
#include <cmath>

#include "utils/MathUtils.h"

namespace android {
namespace uirenderer {

namespace{
    constexpr float NANOSECOND_TO_SECOND = 1e-9f;
    constexpr float SECOND_TO_NANOSECONDS = 1e9f;
    constexpr float DELTA_TIME_TO_ESTIMATE_VELOCITY = 1e-6f;
}

HWSpringAnimator::HWSpringAnimator(RenderProperty property, float finalValue)
        : HWSpringAnimator(property, finalValue, 0.55f, 0.825f, 0.f) {}

HWSpringAnimator::HWSpringAnimator(RenderProperty property, float finalValue, float response,
                                     float dampingRatio)
        : HWSpringAnimator(property, finalValue, response, dampingRatio, 0.f) {}

HWSpringAnimator::HWSpringAnimator(RenderProperty property, float finalValue, float response,
                                     float dampingRatio, float blendDuration)
        : RenderPropertyAnimator(property, finalValue)
        , HWSpringModel()
        , mStagingFinalValue(finalValue)
        , mStagingResponse(response)
        , mStagingDampingRatio(dampingRatio)
        , mBlendDuration(blendDuration) {}

void HWSpringAnimator::initAnimation(AnimationContext& context) {
    if (mBlendDuration > 0.f) {
        mDisplacement = mFinalValue - mStartValue;
        mStartValue = mFinalValue - mDisplacement * context.getBlend();
    }

    }
    nsecs_t currentPlayTime = context.frameTimeMs() - mStartTime;
    // set start value
    if (mHasStartValue == false) {
        // update Animator parameters
        mFromValue = getValue(mTarget);
        mDeltaValue = (mFinalValue - mFromValue);
        // update spring parameters
        initialOffset_ = -mDeltaValue;

        mHasStartValue = true;
        mNeedUpdateModel = true;
    }

    if (mNeedUpdateModel) {
        // (re-)calculate spring parameters
        CalculateSpringParameters();
        mDuration = GetEstimatedDuration() * SECOND_TO_NANOSECONDS;
        mNeedUpdateModel = false;
    }
}

bool HWSpringAnimator::updatePlayTime(nsecs_t playTime) {
    // Note: spring animation should not support reverse/startDelay etc
    if (playTime >= mDuration) {
        setValue(mTarget, mFinalValue);
        return true;
    }

    mPlayTime = playTime;
    mDisplacement = CalculateDisplacement(mPlayTime * NANOSECOND_TO_SECOND);
    setValue(mTarget, mFinalValue + mDisplacement);

    return false;
}

void HWSpringAnimator::resolveStagingRequest(Request request) {
    switch (request) {
        case Request::Start:
            RenderPropertyAnimator::resolveStagingRequest(request);
            [[fallthrough]];
        case Request::Update:
            // TODO: inherit initial Velocity, add blend duration
            if (!mNeedUpdateModel) {
                // mPlayTime is the last time when the animation is running
                // initialVelocity_ = CalculateVelocity();
            }
            response_ = mStagingResponse;
            dampingRatio_ = mStagingDampingRatio;
            mFinalValue = mStagingFinalValue;

            // update start value on next run
            mHasStartValue = false;
            mNeedUpdateModel = true;
            break;
        default:
            RenderPropertyAnimator::resolveStagingRequest(request);
            break;
    }
}

float HWSpringAnimator::estimateInstantaneousVelocity() const {
    if (mPlayTime <= 0) {
        return initialVelocity_;
    } else if (mPlayTime >= mDuration) {
        return 0.f;
    } else {
        // estimate instantaneous velocity at previous run
        float displacement_with_delta = CalculateDisplacement(mPlayTime * NANOSECOND_TO_SECOND +
                                                              DELTA_TIME_TO_ESTIMATE_VELOCITY);
        return (displacement_with_delta - mDisplacement) / DELTA_TIME_TO_ESTIMATE_VELOCITY;
    }
}
}  // namespace uirenderer
}  // namespace android
