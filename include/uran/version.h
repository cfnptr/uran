// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <stdint.h>

/*
 * Version variable.
 */
typedef uint32_t Version;

/*
 * Creates version from the major, minor and patch parts.
 */
#define createVersion(major, minor, patch) \
	(((major) << 24) | ((minor) << 16) | ((patch) << 8))

/*
 * Returns version major part value.
 */
#define getVersionMajor(version) (((version) >> 24) & 0xFF)
/*
 * Returns version minor part value.
 */
#define getVersionMinor(version) (((version) >> 16) & 0xFF)
/*
 * Returns version patch part value.
 */
#define getVersionPatch(version) (((version) >> 8) & 0xFF)

/*
 * Returns version with changed major part.
 */
#define setVersionMajor(version, major) \
	(((version) & 0x00FFFFFF) | ((major) << 24))
/*
 * Returns version with changed minor part.
 */
#define setVersionMinor(version, minor) \
	(((version) & 0xFF00FFFF) | ((minor) << 16))
/*
 * Returns version with changed patch part.
 */
#define setVersionPatch(version, patch) \
	(((version) & 0xFFFF00FF) | ((patch) << 8))
