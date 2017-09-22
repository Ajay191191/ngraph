// ----------------------------------------------------------------------------
// Copyright 2017 Nervana Systems Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// ----------------------------------------------------------------------------

#pragma once

#include <memory>

#include <Eigen/Dense>

namespace ngraph
{
    namespace runtime
    {
        namespace eigen
        {
            template <typename T, typename U>
            void set_map(std::shared_ptr<T>& t, const U& u)
            {
                auto& v = t->get_vector();
                Eigen::Map<Eigen::Array<typename T::value_type, Eigen::Dynamic, 1>>(
                    &v[0], v.size(), 1) = u;
            }

            template <typename T, typename U>
            void set_map(T* t, const U& u)
            {
                auto& v = t->get_vector();
                Eigen::Map<Eigen::Array<typename T::value_type, Eigen::Dynamic, 1>>(
                    &v[0], v.size(), 1) = u;
            }

            template <typename T>
            Eigen::Map<Eigen::Array<typename T::value_type, Eigen::Dynamic, 1>>
                get_map(std::shared_ptr<T>& arg)
            {
                auto& v = arg->get_vector();
                return Eigen::Map<Eigen::Array<typename T::value_type, Eigen::Dynamic, 1>>(
                    &v[0], v.size(), 1);
            }

            template <typename T>
            Eigen::Map<Eigen::Array<typename T::value_type, Eigen::Dynamic, 1>> get_map(T* arg)
            {
                auto& v = arg->get_vector();
                return Eigen::Map<Eigen::Array<typename T::value_type, Eigen::Dynamic, 1>>(
                    &v[0], v.size(), 1);
            }
        }
    }
}