workspace "sigma"
    configurations { "Release", "Debug" }
    startproject "compiler"

    architecture "x64"
    language "C++"
    cppdialect "C++latest"

    externalanglebrackets "On"
    externalwarnings "Off"

    buildoptions { "-w" } -- temp

    flags {
        "MultiProcessorCompile"
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        runtime "Debug"
        defines { "DEBUG", "_DEBUG" }
        warnings "Extra"

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- utility 
project "utility"
    kind "StaticLib"
    location "source/utility"

    files {
        "source/utility/**.h",
        "source/utility/**.cpp"
    }

    includedirs {
        "source"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- -- tokenizer
project "tokenizer"
    kind "StaticLib"
    location "source/tokenizer"

    files {
        "source/tokenizer/**.h",
        "source/tokenizer/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "utility"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- abstract_syntax_tree
project "abstract_syntax_tree"
    kind "StaticLib"
    location "source/abstract_syntax_tree"

    files {
        "source/abstract_syntax_tree/**.h",
        "source/abstract_syntax_tree/**.cpp"
    } 

    includedirs {
        "source"
    }

    links {
        "utility"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- parser
project "parser"
    kind "StaticLib"
    location "source/parser"

    files {
        "source/parser/**.h",
        "source/parser/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "abstract_syntax_tree",
        "utility"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- type_checker
project "type_checker"
    kind "StaticLib"
    location "source/type_checker"

    files {
        "source/type_checker/**.h",
        "source/type_checker/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "abstract_syntax_tree",
        "utility"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- intermediate_representation
project "intermediate_representation"
    kind "StaticLib"
    location "source/intermediate_representation"

    files {
        "source/intermediate_representation/**.h",
        "source/intermediate_representation/**.cpp"
    } 

    includedirs {
        "source"
    }

    links {
        "utility"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- ir_translator
project "ir_translator"
    kind "StaticLib"
    location "source/ir_translator"

    files {
        "source/ir_translator/**.h",
        "source/ir_translator/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "abstract_syntax_tree",
        "intermediate_representation",
        "utility"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- compiler
project "compiler"
    kind "ConsoleApp"
    location "source/compiler"

    debugargs { "compile", "-s", "test/main.s", "-o", "test/app.exe" }

    files {
        "source/compiler/**.h",
        "source/compiler/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "ir_translator",
        "intermediate_representation",
        "type_checker",
        "parser",
        "abstract_syntax_tree",
        "tokenizer",
        "utility"
    }

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

    filter { "system:linux", "action:gmake" }
        links {
            "dl",     
            "pthread",
            "m",      
            "rt"
        }

        buildoptions { 
            "-std=c++23"
        }

        linkoptions {
            "-lstdc++"
        }

      