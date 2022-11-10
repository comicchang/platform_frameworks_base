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

#include "HWSpringModel.h"

#include <cutils/log.h>
#include <gui/TraceUtils.h>

#include <algorithm>
#include <cmath>

#include "utils/MathUtils.h"

namespace android {
namespace uirenderer {

HWSpringModel::HWSpringModel(float response, float dampingRatio, float initialOffset,
                             float initialVelocity, float minimumAmplitude)
        : response_(response)
        , dampingRatio_(dampingRatio)
        , initialOffset_(initialOffset)
        , initialVelocity_(initialVelocity)
        , minimumAmplitudeRatio_(minimumAmplitude) {
    CalculateSpringParameters();
}

void HWSpringModel::CalculateSpringParameters() {
    // sanity check
    dampingRatio_ = std::clamp(dampingRatio_, SPRING_MIN_DAMPING_RATIO, SPRING_MAX_DAMPING_RATIO);
    if (response_ <= 0) {
        response_ = SPRING_MIN_RESPONSE;
    }
    if (minimumAmplitudeRatio_ <= 0) {
        minimumAmplitudeRatio_ = SPRING_MIN_AMPLITUDE_RATIO;
    }

    // calculate internal parameters
    double naturalAngularVelocity = 2 * M_PI / response_;
    if (dampingRatio_ < 1) {  // Under-damped Systems
        dampedAngularVelocity_ =
                naturalAngularVelocity * sqrt(1.0f - dampingRatio_ * dampingRatio_);
        coeffDecay_ = -dampingRatio_ * naturalAngularVelocity;
        coeffScale_ = (initialVelocity_ + initialOffset_ * dampingRatio_ * naturalAngularVelocity) *
                      (1 / dampedAngularVelocity_);
    } else if (dampingRatio_ == 1) {  // Critically-Damped Systems
        coeffDecay_ = -naturalAngularVelocity;
        coeffScale_ = initialVelocity_ + initialOffset_ * naturalAngularVelocity;
    } else {  // Over-damped Systems
        double coeffTmp = sqrt(dampingRatio_ * dampingRatio_ - 1);
        coeffDecay_ = (-dampingRatio_ + coeffTmp) * naturalAngularVelocity;
        coeffScale_ = (initialOffset_ * ((dampingRatio_ + coeffTmp) * naturalAngularVelocity) +
                       initialVelocity_) *
                      (0.5f / (naturalAngularVelocity * coeffTmp));
        coeffScaleAlt_ = (initialOffset_ * ((coeffTmp - dampingRatio_) * naturalAngularVelocity) -
                          initialVelocity_) *
                         (0.5f / (naturalAngularVelocity * coeffTmp));
        coeffDecayAlt_ = (-dampingRatio_ - coeffTmp) * naturalAngularVelocity;
    }
}

void HWSpringModel::EstimateDuration() {
    if (dampingRatio_ <= 0.0f) {
        ALOGE("HWSpringModel::%s, uninitialized spring model", __func__);
        return;
    }

    // convert templated type to float, simplify estimation of spring duration
    float coeffScale = coeffScale_;
    float initialOffset = initialOffset_;
    float estimatedDuration = 0.0f;
    float minimumAmplitude = initialOffset * minimumAmplitudeRatio_;

    if (dampingRatio_ < 1) {  // Under-damped
        estimatedDuration = log(fmax(coeffScale, initialOffset) / minimumAmplitude) / -coeffDecay_;
    } else if (dampingRatio_ == 1) {  // Critically-damped
        // critical damping spring will rest at 2 * natural period
        estimatedDuration_ = response_ * 2;
    } else {  // Over-damped
        float coeffScaleAlt = coeffScaleAlt_;
        double durationMain = (coeffScale <= minimumAmplitude)
                                      ? 0
                                      : (log(coeffScale / minimumAmplitude) / -coeffDecay_);
        double durationAlt = (coeffScaleAlt <= minimumAmplitude)
                                     ? 0
                                     : (log(coeffScaleAlt / minimumAmplitude) / -coeffDecayAlt_);
        estimatedDuration = fmax(durationMain, durationAlt);
    }
    estimatedDuration_ = std::clamp(estimatedDuration, SPRING_MIN_DURATION, SPRING_MAX_DURATION);
    ALOGD("HWSpringModel::%s estimated duration = %.5f, clamped duration = %.5f", __func__,
          estimatedDuration, estimatedDuration_);
}

float HWSpringModel::CalculateDisplacement(double time) const {
    if (dampingRatio_ <= 0.0f) {
        ALOGE("HWSpringModel::%s, uninitialized spring model", __func__);
        return {};
    }
    double coeffDecay = exp(coeffDecay_ * time);
    if (dampingRatio_ < 1) {
        // under-damped
        double rad = dampedAngularVelocity_ * time;
        float coeffPeriod = initialOffset_ * cos(rad) + coeffScale_ * sin(rad);
        return coeffPeriod * coeffDecay;
    } else if (dampingRatio_ == 1) {
        // critical-damped
        return (initialOffset_ + coeffScale_ * time) * coeffDecay;
    } else {
        // over-damped
        double coeffDecayAlt = exp(coeffDecayAlt_ * time);
        return coeffScale_ * coeffDecay + coeffScaleAlt_ * coeffDecayAlt;
    }
}

float HWSpringModel::GetEstimatedDuration() {
    if (estimatedDuration_ < SPRING_MIN_DURATION) {
        // during not estimated yet
        EstimateDuration();
    }
    return estimatedDuration_;
}
}  // namespace uirenderer
}  // namespace android
