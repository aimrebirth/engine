#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

#include <primitives/sw/main.h>
#include <primitives/sw/cl.h>

template <class T>
struct UniqueVector : std::vector<T>
{
    using base = std::vector<T>;

    void push_back(const T &v)
    {
        if (unique.find(v) != unique.end())
            return;
        base::push_back(v);
        unique.insert(v);
    }

private:
    std::set<T> unique;
};

int main(int argc, char **argv)
{
    cl::opt<path> output_dir(cl::Positional, cl::Required);
    cl::opt<path> input_json(cl::Positional, cl::Required);
    cl::opt<String> main_target(cl::Positional, cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    UniqueVector<String> defs;
    UniqueVector<String> idirs;
    UniqueVector<String> libs;

    auto j = nlohmann::json::parse(read_file(input_json));

    std::function<void(nlohmann::json &)> process_deps;
    process_deps = [&process_deps, &libs, &defs, &idirs, &j](nlohmann::json &j2)
    {
        if (j2.contains("import_library"))
            libs.push_back(j2["import_library"]);
        for (auto &[k, v] : j2["definitions"].items())
        {
            if (v.get<String>().empty())
                defs.push_back(k);
            else
                defs.push_back(k + "=" + v.get<String>());
        }
        for (auto &v : j2["include_directories"])
            idirs.push_back(v.get<String>());
        for (auto &[k,v] : j2["dependencies"]["link"].items())
        {
            for (auto &v2 : j["build"][k])
            {
                if (v2["key"] == v)
                {
                    process_deps(v2["value"]);
                    break;
                }
            }
        }
    };
    process_deps(j["build"][boost::to_lower_copy(String(main_target))][0]["value"]);

    auto write_var = [&output_dir](const String &name, const auto &a)
    {
        String s;
        for (auto &v : a)
            s += v + "\n";
        write_file(output_dir / name += ".txt", s);
    };
    write_var("defs", defs);
    write_var("idirs", idirs);
    write_var("libs", libs);

    return 0;
}