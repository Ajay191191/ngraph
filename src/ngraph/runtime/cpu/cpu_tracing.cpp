// ----------------------------------------------------------------------------
// Copyright 2018 Nervana Systems Inc.
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

#include <fstream>
#include <map>

#include "cpu_tracing.hpp"

void ngraph::runtime::cpu::to_json(nlohmann::json& json, const TraceEvent& event)
{
    std::map<std::string, std::string> args;
    for (size_t i = 0; i < event.Inputs.size(); i++)
    {
        args["Input" + std::to_string(i + 1)] = event.Inputs[i];
    }
    for (size_t i = 0; i < event.Outputs.size(); i++)
    {
        args["Output" + std::to_string(i + 1)] = event.Outputs[i];
    }

    json = nlohmann::json{{"ph", event.Phase},
                          {"cat", event.Category},
                          {"name", event.Name},
                          {"pid", event.PID},
                          {"tid", event.TID},
                          {"ts", event.Timestamp},
                          {"dur", event.Duration},
                          {"args", args}};
}

void ngraph::runtime::cpu::GenerateTimeline(const std::vector<OpAttributes>& op_attrs,
                                            int64_t* op_durations)
{
    nlohmann::json timeline;
    std::list<TraceEvent> trace;
    std::ofstream out("timeline.json");

    int64_t ts = 0;
    for (size_t i = 0; i < op_attrs.size(); i++)
    {
        trace.emplace_back("X",
                           "Op",
                           op_attrs[i].Description,
                           0,
                           0,
                           ts,
                           op_durations[i],
                           op_attrs[i].Outputs,
                           op_attrs[i].Inputs);
        ts += op_durations[i];
    }

    timeline["traceEvents"] = trace;
    out << timeline;
    out.close();

    return;
}

bool ngraph::runtime::cpu::IsTracingEnabled()
{
    return (std::getenv("NGRAPH_CPU_TRACING") != nullptr);
}