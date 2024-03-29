#
# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifeq ($(BOARD_USES_QCOM_HARDWARE),true)

common_msm_dirs := liblights dspcrashd libaudio-msm7x30
msm7k_dirs := $(common_msm_dirs) boot

ifeq ($(TARGET_BOARD_PLATFORM),msm7x30)
  include $(call all-named-subdir-makefiles,$(msm7k_dirs))
else
  include $(call all-named-subdir-makefiles,$(common_msm_dirs))
endif

endif
