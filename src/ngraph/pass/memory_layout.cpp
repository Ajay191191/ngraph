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

#include <exception>
#include <sstream>

#include "ngraph/log.hpp"
#include "ngraph/ngraph.hpp"
#include "ngraph/pass/liveness.hpp"
#include "ngraph/pass/memory_layout.hpp"
#include "ngraph/log.hpp"

using namespace std;
using namespace ngraph;

bool pass::MemoryLayout::run_on_call_list(std::list<Node*>& node_list)
{
    for (Node* node : node_list)
    {
    }
    return false;
}

void pass::MemoryLayout::check_dependencies(
    const std::vector<std::shared_ptr<CallBase>>& registered_passes) const
{
    bool found_propagate_types = false;
    for (auto pass : registered_passes)
    {
        if (dynamic_pointer_cast<Liveness>(pass))
        {
            found_propagate_types = true;
        }
    }

    if (!found_propagate_types)
    {
        throw runtime_error("Dependency 'PropagateTypes' not found for pass 'AssignTensors'");
    }
}

pass::MemoryManager::node::node(size_t size, block_state state)
    : m_size{size}
    , m_state{state}
{

}

pass::MemoryManager::MemoryManager(size_t alignment)
    : m_alignment{alignment}
    , m_scheme{allocation_scheme::BEST_FIT}
    , m_max_allocated{0}
{
    // assert(m_base_offset % m_alignment == 0);
    m_node_list.emplace_back(numeric_limits<size_t>::max(), block_state::FREE);
}

size_t pass::MemoryManager::allocate(size_t size)
{
    size_t rc;
    switch(m_scheme)
    {
    case allocation_scheme::FIRST_FIT:
        rc = first_fit(size);
        break;
    case allocation_scheme::BEST_FIT:
        rc = best_fit(size);
        break;
    }
    return rc;
}

size_t pass::MemoryManager::best_fit(size_t size)
{
    size = align(size, m_alignment);
    size_t offset = 0;
    size_t min_delta = numeric_limits<size_t>::max();
    auto best_fit = m_node_list.end();
    size_t best_offset = offset;
    for (auto it=m_node_list.begin(); it != m_node_list.end(); ++it)
    {
        if (it->m_state == block_state::FREE && it->m_size >= size)
        {
            size_t delta = it->m_size - size;
            if (delta < min_delta)
            {
                min_delta = delta;
                best_fit = it;
                best_offset = offset;
            }
        }
        offset += it->m_size;
    }

    if (best_fit == m_node_list.end())
    {
        throw bad_alloc();
    }

    if (min_delta == 0)
    {
        // exact fit
        best_fit->m_state = block_state::ALLOCATED;
    }
    else
    {
        m_node_list.insert(best_fit, node{size, block_state::ALLOCATED});
        best_fit->m_size -= size;
    }
    m_max_allocated = std::max(m_max_allocated, best_offset + size);

    return best_offset;
}

size_t pass::MemoryManager::first_fit(size_t size)
{
    size = align(size, m_alignment);
    size_t offset = 0;
    bool found = false;
    for (auto it=m_node_list.begin(); it != m_node_list.end(); ++it)
    {
        if (it->m_state == block_state::FREE && it->m_size >= size)
        {
            if (it->m_size > size)
            {
                m_node_list.insert(it, node{size, block_state::ALLOCATED});
                it->m_size -= size;
            }
            else
            {
                // exact fit
                it->m_state = block_state::ALLOCATED;
            }

            found = true;
            break;
        }
        offset += it->m_size;
    }
    if (!found)
    {
        throw bad_alloc();
    }
    m_max_allocated = std::max(m_max_allocated, offset + size);

    return offset;
}

void pass::MemoryManager::free(size_t offset)
{
    size_t search_offset = 0;
    bool found = false;
    for (auto it=m_node_list.begin(); it != m_node_list.end(); ++it)
    {
        if (offset == search_offset)
        {
            list<node>::iterator it_next = std::next(it);
            if (it == m_node_list.begin())
            {
                // free the first node in the list
                it->m_state = block_state::FREE;
            }
            else
            {
                // node has predecessor
                list<node>::iterator it_prev = std::prev(it);
                if (it_prev->m_state == block_state::FREE)
                {
                    it->m_size += it_prev->m_size;
                    m_node_list.erase(it_prev);
                }
            }
            if (it_next != m_node_list.end() && it_next->m_state == block_state::FREE)
            {
                // join this node with next
                it->m_size += it_next->m_size;
                m_node_list.erase(it_next);
            }
            it->m_state = block_state::FREE;
            found = true;
            break;
        }
        search_offset += it->m_size;
    }
    if (!found)
    {
        throw runtime_error("bad free");
    }
}

void pass::MemoryManager::dump(std::ostream& out)
{
    for (const node& n : m_node_list)
    {
        out << "size=" << n.m_size << ", ";
        out << (n.m_state == block_state::FREE ? "FREE" : "ALLOCATED");
        out << "\n";
    }
}

size_t pass::MemoryManager::align(size_t size, size_t alignment)
{
    if (size == 0)
    {
        size = alignment;
    }
    else
    {
        auto remainder = size % alignment;
        if (remainder > 0)
        {
            size += (alignment - remainder);
        }
    }
    return size;
}