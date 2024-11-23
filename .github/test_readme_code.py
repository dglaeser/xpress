
from __future__ import annotations

import os
import shutil
import argparse
import subprocess


def default_cmake_lists(app_name: str = "my_app") -> str:
    return f"""
cmake_minimum_required(VERSION 3.18)
project(xpress_example)

set(XPRESS_BUILD_TESTS OFF)
include(FetchContent)
FetchContent_Declare(
    xpress
    GIT_REPOSITORY https://github.com/dglaeser/xpress.git
    GIT_TAG main
)
FetchContent_MakeAvailable(xpress)

add_executable({app_name} {app_name}.cpp)
target_link_libraries({app_name} PRIVATE xpress::xpress)
"""


def default_compile_commands(app_name: str = "my_app") -> str:
    return f"""
cmake -B build
cmake --build build
cd build && ./{app_name}
"""


def main_file_for(snippet: str) -> str:
    return f"""
#include <print>
#include <iostream>
#include <vector>
#include <array>
#include <xpress/xp.hpp>
#include <xpress/solvers/newton.hpp>

int main() {{
    using namespace xp;
    {snippet}
    std::cout << "Snippet finished" << std::endl;
    return 0;
}}
"""


class CodeBlock:
    def __init__(self, header: str, code: str) -> None:
        self.name = extract_example_name_from(header)
        self.type = extract_code_type_from(header)
        self.code = code
        assert(self.type != "" and self.type is not None)


class Application:
    def __init__(
        self,
        name: str,
        cmake_lists: str | None,
        main_file: str | None,
        build_commands: str | None
    ) -> None:
        self._app_name = name
        self._cmake_lists = cmake_lists
        self._main_file = main_file
        self._build_commands = build_commands

    @staticmethod
    def from_readme(content: str, name: str) -> Application:
        def parse_code_block_at(remainder: str) -> CodeBlock:
            assert(remainder.startswith("```"))
            header, remainder = remainder.split("\n", maxsplit=1)
            code = remainder.split("```", maxsplit=1)[0]
            code_block = CodeBlock(header=header, code=code)
            assert(code_block.name == name)
            return code_block

        code_blocks = []
        content_lines = content.split("\n")
        for line_idx in range(len(content_lines)):
            if not is_code_block_begin(content_lines[line_idx]):
                continue
            line = content_lines[line_idx]
            this_name = extract_example_name_from(line)
            if this_name != name:
                continue
            code_blocks.append(parse_code_block_at("\n".join(content_lines[line_idx:])))

        assert(all(sum(1 if _cb.type == cb.type else 0 for _cb in code_blocks) == 1 for cb in code_blocks))
        cmake = None
        main = None
        cmds = None
        snippet = None
        for cb in code_blocks:
            if cb.type == "main": main = cb.code
            elif cb.type == "snippet": snippet = cb.code
            elif cb.type == "bash": cmds = cb.code
            elif cb.type == "cmake": cmake = cb.code
            else:raise NotImplementedError(f"Unsupported code type '{cb.type}'")

        assert(not (main and snippet))
        app_name = "my_app"
        if cmake is not None:
            app_name = cmake.split("add_executable(")[1].split()[0]

        return Application(
            name=app_name,
            cmake_lists=cmake,
            main_file=main or (main_file_for(snippet) if snippet is not None else None),
            build_commands=cmds or default_compile_commands(app_name)
        )

    def run_with(self, c_compiler: str, cxx_compiler: str) -> None:
        os.environ["CC"] = c_compiler
        os.environ["CXX"] = cxx_compiler
        os.makedirs("_tmp")
        cwd = os.getcwd()
        os.chdir("_tmp")

        print("Using the following build commands:\n", self._build_commands)
        if self._main_file is None and self._cmake_lists is None:
            subprocess.run(str(self._build_commands), shell=True, check=True)
            subprocess.run(str(self._build_commands), shell=True, check=True)
        else:
            print("Using the following cmake file:\n", self._cmake_lists)
            print("Using the following main file:\n", self._main_file)
            with open("CMakeLists.txt", "w") as cmake_file: cmake_file.write(self._cmake_lists or default_cmake_lists(self._app_name))
            with open(f"{self._app_name}.cpp", "w") as main: main.write(str(self._main_file))
            subprocess.run(str(self._build_commands), shell=True, check=True)
        os.chdir(cwd)
        shutil.rmtree("_tmp")


def is_code_block_begin(line: str) -> bool:
    return line.startswith("```") and line.split("```")[1] != ""


def extract_code_type_from(line: str) -> str | None:
    if not line.startswith("```"):
        return None
    if "cpp" in line and "main" in line:
        return "main"
    if "cpp" in line and "snippet" in line:
        return "snippet"
    return line.split("```")[1].split(" ")[0]


def extract_example_name_from(line: str) -> str | None:
    if "xpress-" not in line:
        return None
    return line.split("xpress-")[1].split("-", maxsplit=1)[0]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--c-compiler", required=True)
    parser.add_argument("-x", "--cxx-compiler", required=True)
    parser.add_argument("-r", "--readme", required=True, help="Path to the README.md file")
    parser.add_argument("-n", "--name", required=False, help="Specify a concrete example to run individually")
    args = vars(parser.parse_args())

    content = open(args["readme"]).read()
    parsed_examples: list[str] = []
    for line in content.split("\n"):
        if not is_code_block_begin(line):
            continue

        name = extract_example_name_from(line)
        if args["name"] is not None and name != args["name"]:
            continue

        if name in parsed_examples:
            continue

        print("\n"*3)
        print(f"Running readme code '{name}'")
        Application.from_readme(content, name=name).run_with(
            c_compiler=args["c_compiler"],
            cxx_compiler=args["cxx_compiler"]
        )
        parsed_examples.append(name)
