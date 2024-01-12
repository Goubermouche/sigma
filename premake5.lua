workspace "sigma"
    configurations { "Release", "Debug", "Test" }
    startproject "compiler"

    architecture "x64"
    language "C++"
    cppdialect "C++latest"
    warnings "Extra"

    flags {
        "MultiProcessorCompile"
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "Off"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        runtime "Debug"
        defines { "DEBUG", "_DEBUG" }
        sanitize { "Address" }
        debugformat "C7" 

    -- used when running CI tests
    filter "configurations:Test"
        symbols "On"
        optimize "Off"
        runtime "Debug"
        defines { "DEBUG", "_DEBUG" }

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

-- compiler
project "compiler"
    kind "ConsoleApp"
    location "source/compiler"

    debugargs { "compile", "./test/main.s" }

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
    targetdir "output/compiler/bin/%{cfg.buildcfg}"
    objdir "output/compiler/obj/%{cfg.buildcfg}"

    filter { "system:linux", "action:gmake" }
        links {
            "dl",     
            "pthread",
            "m",      
            "rt"
        }

        linkoptions {
            "-lstdc++"
        }

workspace "tests"
    configurations { "Release", "Debug", "Profile" }
    startproject "tests"

    architecture "x64"
    language "C++"
    cppdialect "C++latest"

    flags {
        "MultiProcessorCompile"
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"
        symbols "Off"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        runtime "Debug"
        defines { "DEBUG", "_DEBUG" }
        warnings "Extra"

    filter "configurations:Profile"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"
        symbols "On"


project "tests"
    kind "ConsoleApp"
    location "source/tests"

    files {
        "source/tests/**.h",
        "source/tests/**.cpp"
    }

    libdirs { "../sigma/output/compiler/bin/%{cfg.buildcfg}" }

    debugargs { "run", "../../tests" }

    includedirs {
        "source"
    }

    links {
        "utility"
    }

    -- output directories 
    targetdir "output/tests/bin/%{cfg.buildcfg}"
    objdir "output/tests/obj/%{cfg.buildcfg}"

    filter { "system:linux", "action:gmake" }
        links {
            "dl",     
            "pthread",
            "m",      
            "rt"
        }

        linkoptions {
            "-lstdc++"
        }