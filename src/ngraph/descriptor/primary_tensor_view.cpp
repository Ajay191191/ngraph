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

#include "ngraph/descriptor/primary_tensor_view.hpp"

using namespace ngraph;
using namespace descriptor;

PrimaryTensorView::PrimaryTensorView(const std::shared_ptr<const TensorViewType>& tensor_view_type,
                                     const std::string& name,
                                     bool is_output,
                                     bool is_input)
    : TensorView(tensor_view_type)
    , m_tensor(tensor_view_type->get_element_type(), this, name, is_output, is_input)
{
    // Set the name in the parent TensorView.
    // This can't be done until after the m_tensor is constructed.
    m_name = m_tensor.get_next_view_name();
}

const Tensor& PrimaryTensorView::get_tensor() const
{
    return m_tensor;
}

Tensor& PrimaryTensorView::get_tensor()
{
    return m_tensor;
}