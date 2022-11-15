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

#pragma once

#include "Animator.h"
#include "HWSpringModel.h"

namespace android {
namespace uirenderer {
class HWSpringAnimator : public RenderPropertyAnimator, public HWSpringModel {
public:
    HWSpringAnimator(RenderProperty property, float finalValue);
    HWSpringAnimator(RenderProperty property, float finalValue, float response,
                      float dampingRatio);
    HWSpringAnimator(RenderProperty property, float finalValue, float response, float dampingRatio,
                      float blendDuration);
    ~HWSpringAnimator() override = default;


private:
    bool updatePlayTime(nsecs_t playTime) override;
    void resolveStagingRequest(Request request) override;
    void initAnimation(AnimationContext& context) override;

    float estimateInstantaneousVelocity() const;

    float mStagingFinalValue;
    float mStagingResponse;
    float mStagingDampingRatio;
    float mBlendDuration;

    float mDisplacement;

    bool mNeedUpdateModel = true;
};
}  // namespace uirenderer
}  // namespace android