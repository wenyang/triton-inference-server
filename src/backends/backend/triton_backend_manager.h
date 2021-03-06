// Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "src/backends/backend/tritonbackend.h"
#include "src/core/status.h"

namespace nvidia { namespace inferenceserver {

//
// Proxy to a backend shared library.
//
class TritonBackend {
 public:
  static Status Create(
      const std::string& name, const std::string& path,
      std::shared_ptr<TritonBackend>* backend);
  ~TritonBackend();

  const std::string& Name() const { return name_; }
  void* State() { return state_; }
  void SetState(void* state) { state_ = state; }

 private:
  typedef TRITONSERVER_Error* (*TritonBackendInitFn_t)(
      TRITONBACKEND_Backend* backend);

  explicit TritonBackend(const std::string& name, const std::string& path)
      : name_(name), path_(path), state_(nullptr)
  {
  }

  Status LoadBackendLibrary();
  Status UnloadBackendLibrary();

  // The name of the backend.
  const std::string name_;

  // Full path to the backend shared library.
  const std::string path_;

  // dlopen / dlsym handles
  void* dlhandle_;
  TritonBackendInitFn_t backend_init_fn_;
  TritonBackendInitFn_t backend_fini_fn_;

  // Opaque state associated with the backend.
  void* state_;
};

//
// Manage communication with Triton backends and their lifecycle.
//
class TritonBackendManager {
 public:
  static Status CreateBackend(
      const std::string& name, const std::string& path,
      std::shared_ptr<TritonBackend>* backend);

 private:
  std::mutex mu_;
  std::unordered_map<std::string, std::weak_ptr<TritonBackend>> backend_map_;
};

}}  // namespace nvidia::inferenceserver
