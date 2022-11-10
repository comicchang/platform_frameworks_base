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

namespace android {
namespace uirenderer {
class HWSpringModel {
public:
    explicit HWSpringModel(float response, float dampingRatio, float initialOffset,
                           float initialVelocity, float minimumAmplitude);

    virtual ~HWSpringModel() = default;

    float CalculateDisplacement(double time) const;
    float GetEstimatedDuration();

protected:
    HWSpringModel() = default;
    void EstimateDuration();
    void CalculateSpringParameters();

    // physical parameters of spring-damper model
    float response_{0.0f};
    float dampingRatio_{0.0f};
    float initialOffset_;
    float initialVelocity_;

    // estimated duration until the spring is at rest
    float minimumAmplitudeRatio_{0.001f};
    float estimatedDuration_{-1.0f};

private:
    // calculated intermediate coefficient
    float coeffDecay_{0.0f};
    float coeffScale_{0.0f};
    float dampedAngularVelocity_{0.0f};

    // only used in over-damped system
    float coeffScaleAlt_{0.0f};
    float coeffDecayAlt_{0.0f};

    constexpr static float SPRING_MIN_DAMPING_RATIO = 1e-4f;
    constexpr static float SPRING_MAX_DAMPING_RATIO = 1e4f;
    constexpr static float SPRING_MIN_DURATION = 0.001f;
    constexpr static float SPRING_MAX_DURATION = 300.0f;
    constexpr static float SPRING_MIN_RESPONSE = 1e-8;
    constexpr static float SPRING_MIN_AMPLITUDE_RATIO = 0.001f;
};
}  // namespace uirenderer
}  // namespace android