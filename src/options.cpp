#include "options.h"

std::vector<std::string> ParamArr(const TOptions& opts) {
    std::vector<std::string> params;
    params.reserve(opts.Size());
    for (std::size_t param = 0; param < opts.Size(); ++param) {
        params.emplace_back(opts.Get<std::string_view>(param));
    }
    return params;
}
